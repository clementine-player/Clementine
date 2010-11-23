/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "icecastservice.h"

#include <algorithm>
using std::sort;
using std::unique;

#include <QFutureWatcher>
#include <QMultiHash>
#include <QNetworkReply>
#include <QRegExp>
#include <QtConcurrentRun>

#include "icecastbackend.h"
#include "icecastfilterwidget.h"
#include "icecastmodel.h"
#include "radiomodel.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/taskmanager.h"

const char* IcecastService::kServiceName = "Icecast";
const char* IcecastService::kDirectoryUrl = "http://dir.xiph.org/yp.xml";

IcecastService::IcecastService(RadioModel* parent)
    : RadioService(kServiceName, parent),
      network_(new NetworkAccessManager(this)),
      backend_(NULL),
      model_(NULL),
      filter_(new IcecastFilterWidget(0)),
      load_directory_task_id_(0)
{
  backend_ = new IcecastBackend;
  backend_->moveToThread(parent->db_thread());
  backend_->Init(parent->db_thread()->Worker());

  model_ = new IcecastModel(backend_, this);
  filter_->SetIcecastModel(model_);
}

IcecastService::~IcecastService() {
}

RadioItem* IcecastService::CreateRootItem(RadioItem* parent) {
  root_ = new RadioItem(this, RadioItem::Type_Service, kServiceName, parent);
  root_->icon = QIcon(":last.fm/icon_radio.png");
  return root_;
}

void IcecastService::LazyPopulate(RadioItem* item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      model_->Init();
      model()->merged_model()->AddSubModel(
            model()->index(root_->row, 0, model()->ItemToIndex(item->parent)),
            model_);

      if (backend_->IsEmpty()) {
        LoadDirectory();
      }

      break;
    default:
      break;
  }
  item->lazy_loaded = true;
}

void IcecastService::LoadDirectory() {
  QNetworkRequest req = QNetworkRequest(QUrl(kDirectoryUrl));
  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(DownloadDirectoryFinished()));

  if (!load_directory_task_id_) {
    load_directory_task_id_ = model()->task_manager()->StartTask(
        tr("Downloading Icecast directory"));
  }
}

void IcecastService::DownloadDirectoryFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);

  QFuture<IcecastBackend::StationList> future =
      QtConcurrent::run(this, &IcecastService::ParseDirectory, reply);
  QFutureWatcher<IcecastBackend::StationList>* watcher =
      new QFutureWatcher<IcecastBackend::StationList>(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(ParseDirectoryFinished()));
}

namespace {
template <typename T>
struct GenreSorter {
  GenreSorter(const QMultiHash<QString, T>& genres)
      : genres_(genres) {
  }

  bool operator() (const QString& a, const QString& b) const {
    return genres_.count(a) > genres_.count(b);
  }

 private:
  const QMultiHash<QString, T>& genres_;
};

template <typename T>
struct StationSorter {
  bool operator() (const T& a, const T& b) const {
    return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
  }
};

template <typename T>
struct StationSorter<T*> {
  bool operator() (const T* a, const T* b) const {
    return a->name.compare(b->name, Qt::CaseInsensitive) < 0;
  }
};

template <typename T>
struct StationEquality {
  bool operator() (T a, T b) const {
    return a.name == b.name;
  }
};

QStringList FilterGenres(const QStringList& genres) {
  QStringList ret;
  foreach (const QString& genre, genres) {
    if (genre.length() < 2) continue;
    if (genre.contains("ÃÂ")) continue;  // Broken unicode.
    if (genre.contains(QRegExp("^#x[0-9a-f][0-9a-f]"))) continue;  // Broken XML entities.

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

}

void IcecastService::ParseDirectoryFinished() {
  QFutureWatcher<IcecastBackend::StationList >* watcher =
      static_cast<QFutureWatcher<IcecastBackend::StationList>*>(sender());
  IcecastBackend::StationList all_stations = watcher->result();
  sort(all_stations.begin(), all_stations.end(), StationSorter<IcecastBackend::Station>());
  // Remove duplicates by name. These tend to be multiple URLs for the same station.
  IcecastBackend::StationList::iterator it =
      unique(all_stations.begin(), all_stations.end(), StationEquality<IcecastBackend::Station>());
  all_stations.erase(it, all_stations.end());

  // Cluster stations by genre.
  QMultiHash<QString, IcecastBackend::Station*> genres;

  // Add stations.
  for (int i=0 ; i<all_stations.count() ; ++i) {
    IcecastBackend::Station& s = all_stations[i];
    genres.insert(s.genre, &s);
  }

  QSet<QString> genre_set = genres.keys().toSet();

  // Merge genres with only 1 or 2 stations into "Other".
  foreach (const QString& genre, genre_set) {
    if (genres.count(genre) < 3) {
      const QList<IcecastBackend::Station*>& small_genre = genres.values(genre);
      foreach (IcecastBackend::Station* s, small_genre) {
        s->genre = "Other";
      }
    }
  }

  backend_->ClearAndAddStations(all_stations);
  delete watcher;

  model()->task_manager()->SetTaskFinished(load_directory_task_id_);
  load_directory_task_id_ = 0;
}

IcecastBackend::StationList IcecastService::ParseDirectory(QIODevice* device) const {
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

IcecastBackend::Station IcecastService::ReadStation(QXmlStreamReader* reader) const {
  IcecastBackend::Station station;
  while (!reader->atEnd()) {
    reader->readNext();
    if (reader->tokenType() == QXmlStreamReader::EndElement)
      break;

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();
      QString value = reader->readElementText(QXmlStreamReader::SkipChildElements);

      if (name == "server_name") station.name = value;
      if (name == "listen_url")  station.url = QUrl(value);
      if (name == "server_type") station.mime_type = value;
      if (name == "bitrate")     station.bitrate = value.toInt();
      if (name == "channels")    station.channels = value.toInt();
      if (name == "samplerate")  station.samplerate = value.toInt();
      if (name == "genre")       station.genre =
          FilterGenres(value.split(' ', QString::SkipEmptyParts))[0];
    }
  }

  // Title case the genre
  if (!station.genre.isEmpty()) {
    station.genre[0] = station.genre[0].toUpper();
  }

  // HACK: This hints to the player that the artist and title metadata needs swapping.
  station.url.setFragment("icecast");

  return station;
}

QWidget* IcecastService::HeaderWidget() const {
  return filter_;
}
