/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2010, 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2016, David Ó Laıġeanáın <david.lynam@redbrick.dcu.ie>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "internet/icecast/icecastservice.h"

#include <algorithm>

#include <QDesktopServices>
#include <QMenu>
#include <QMultiHash>
#include <QNetworkReply>
#include <QRegExp>
#include <QtConcurrentRun>

#include "core/application.h"
#include "core/closure.h"
#include "core/database.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/taskmanager.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/icecastsearchprovider.h"
#include "internet/core/internetmodel.h"
#include "internet/icecast/icecastbackend.h"
#include "internet/icecast/icecastfilterwidget.h"
#include "internet/icecast/icecastmodel.h"
#include "playlist/songplaylistitem.h"
#include "ui/iconloader.h"

using std::sort;
using std::unique;

const char* IcecastService::kServiceName = "Icecast";
const char* IcecastService::kDirectoryUrl =
    "http://data.clementine-player.org/icecast-directory";
const char* IcecastService::kHomepage = "http://dir.xiph.org/";

IcecastService::IcecastService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      network_(new NetworkAccessManager(this)),
      context_menu_(nullptr),
      backend_(nullptr),
      model_(nullptr),
      filter_(new IcecastFilterWidget(0)) {
  backend_ = new IcecastBackend;
  backend_->moveToThread(app_->database()->thread());
  backend_->Init(app_->database());

  model_ = new IcecastModel(backend_, this);
  filter_->SetIcecastModel(model_);

  app_->global_search()->AddProvider(
      new IcecastSearchProvider(backend_, app_, this));
}

IcecastService::~IcecastService() {}

QStandardItem* IcecastService::CreateRootItem() {
  root_ = new QStandardItem(IconLoader::Load("icon_radio", IconLoader::Lastfm),
                            kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void IcecastService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      model_->Init();
      model()->merged_model()->AddSubModel(model()->indexFromItem(item),
                                           model_);

      if (backend_->IsEmpty()) {
        LoadDirectory();
      }

      break;
    default:
      break;
  }
}

void IcecastService::LoadDirectory() {
  int task_id =
      app_->task_manager()->StartTask(tr("Downloading Icecast directory"));
  RequestDirectory(QUrl(kDirectoryUrl), task_id);
}

void IcecastService::RequestDirectory(const QUrl& url, int task_id) {
  QNetworkRequest req = QNetworkRequest(url);
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);

  QNetworkReply* reply = network_->get(req);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(DownloadDirectoryFinished(QNetworkReply*, int)), reply,
             task_id);
}

void IcecastService::DownloadDirectoryFinished(QNetworkReply* reply,
                                               int task_id) {
  if (reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isValid()) {
    // Discard the old reply and follow the redirect
    reply->deleteLater();
    RequestDirectory(
        reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl(),
        task_id);
    return;
  }

  QFuture<IcecastBackend::StationList> future =
      QtConcurrent::run(this, &IcecastService::ParseDirectory, reply);
  NewClosure(future, this, SLOT(ParseDirectoryFinished(
                               QFuture<IcecastBackend::StationList>, int)),
             future, task_id);
}

namespace {
template <typename T>
struct GenreSorter {
  explicit GenreSorter(const QMultiHash<QString, T>& genres)
      : genres_(genres) {}

  bool operator()(const QString& a, const QString& b) const {
    return genres_.count(a) > genres_.count(b);
  }

 private:
  const QMultiHash<QString, T>& genres_;
};

template <typename T>
struct StationSorter {
  bool operator()(const T& a, const T& b) const {
    return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
  }
};

template <typename T>
struct StationSorter<T*> {
  bool operator()(const T* a, const T* b) const {
    return a->name.compare(b->name, Qt::CaseInsensitive) < 0;
  }
};

template <typename T>
struct StationEquality {
  bool operator()(T a, T b) const { return a.name == b.name; }
};

QStringList FilterGenres(const QStringList& genres) {
  QStringList ret;
  for (const QString& genre : genres) {
    if (genre.length() < 2) continue;
    if (genre.contains("ÃÂ")) continue;  // Broken unicode.
    if (genre.contains(QRegExp("^#x[0-9a-f][0-9a-f]")))
      continue;  // Broken XML entities.

    // Convert 80 -> 80s.
    if (genre.contains(QRegExp("^[0-9]0$"))) {
      ret << genre + 's';
    } else {
      ret << genre;
    }
  }

  if (ret.empty()) {
    ret << "other";
  }
  return ret;
}
}  // namespace

void IcecastService::ParseDirectoryFinished(
    QFuture<IcecastBackend::StationList> future, int task_id) {
  IcecastBackend::StationList all_stations = future.result();
  sort(all_stations.begin(), all_stations.end(),
       StationSorter<IcecastBackend::Station>());
  // Remove duplicates by name. These tend to be multiple URLs for the same
  // station.
  IcecastBackend::StationList::iterator it =
      unique(all_stations.begin(), all_stations.end(),
             StationEquality<IcecastBackend::Station>());
  all_stations.erase(it, all_stations.end());

  // Cluster stations by genre.
  QMultiHash<QString, IcecastBackend::Station*> genres;

  // Add stations.
  for (int i = 0; i < all_stations.count(); ++i) {
    IcecastBackend::Station& s = all_stations[i];
    genres.insert(s.genre, &s);
  }

  QSet<QString> genre_set = genres.keys().toSet();

  // Merge genres with only 1 or 2 stations into "Other".
  for (const QString& genre : genre_set) {
    if (genres.count(genre) < 3) {
      const QList<IcecastBackend::Station*>& small_genre = genres.values(genre);
      for (IcecastBackend::Station* s : small_genre) {
        s->genre = "Other";
      }
    }
  }

  backend_->ClearAndAddStations(all_stations);

  app_->task_manager()->SetTaskFinished(task_id);
}

IcecastBackend::StationList IcecastService::ParseDirectory(
    QIODevice* device) const {
  QXmlStreamReader reader(device);
  IcecastBackend::StationList stations;
  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "entry") {
      stations << ReadStation(&reader);
    }
  }
  device->deleteLater();
  return stations;
}

IcecastBackend::Station IcecastService::ReadStation(
    QXmlStreamReader* reader) const {
  IcecastBackend::Station station;
  while (!reader->atEnd()) {
    reader->readNext();
    if (reader->tokenType() == QXmlStreamReader::EndElement) break;

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();
      QString value =
          reader->readElementText(QXmlStreamReader::SkipChildElements);

      if (name == "server_name") station.name = value;
      if (name == "listen_url") station.url = QUrl(value);
      if (name == "server_type") station.mime_type = value;
      if (name == "bitrate") station.bitrate = value.toInt();
      if (name == "channels") station.channels = value.toInt();
      if (name == "samplerate") station.samplerate = value.toInt();
      if (name == "genre")
        station.genre =
            FilterGenres(value.split(' ', QString::SkipEmptyParts))[0];
    }
  }

  // Title case the genre
  if (!station.genre.isEmpty()) {
    station.genre[0] = station.genre[0].toUpper();
  }

  return station;
}

QWidget* IcecastService::HeaderWidget() const { return filter_; }

void IcecastService::ShowContextMenu(const QPoint& global_pos) {
  EnsureMenuCreated();

  const bool can_play = model()->current_index().isValid() &&
                        model()->current_index().model() == model_ &&
                        model_->GetSong(model()->current_index()).is_valid();

  if (can_play)
    selected_playable_item_url_ = model_->GetSong(model()->current_index()).url();

  GetAppendToPlaylistAction()->setEnabled(can_play);
  GetReplacePlaylistAction()->setEnabled(can_play);
  GetOpenInNewPlaylistAction()->setEnabled(can_play);
  GetCopySelectedPlayableItemURLAction()->setEnabled(can_play);

  context_menu_->popup(global_pos);
}

void IcecastService::EnsureMenuCreated() {
  if (context_menu_) return;

  context_menu_ = new QMenu;

  context_menu_->addActions(GetPlaylistActions());
  context_menu_->addAction(IconLoader::Load("download", IconLoader::Base),
                           tr("Open %1 in browser").arg("dir.xiph.org"), this,
                           SLOT(Homepage()));
  context_menu_->addAction(IconLoader::Load("view-refresh", IconLoader::Base),
                           tr("Refresh station list"), this,
                           SLOT(LoadDirectory()));
  context_menu_->addAction(GetCopySelectedPlayableItemURLAction());
  context_menu_->addSeparator();
  context_menu_->addMenu(filter_->menu());
}

void IcecastService::Homepage() { QDesktopServices::openUrl(QUrl(kHomepage)); }
