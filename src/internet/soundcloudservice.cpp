/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "soundcloudservice.h"

#include <QDesktopServices>
#include <QMenu>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include "internetmodel.h"
#include "searchboxwidget.h"

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/soundcloudsearchprovider.h"
#include "ui/iconloader.h"

const char* SoundCloudService::kApiClientId =
    "2add0f709fcfae1fd7a198ec7573d2d4";

const char* SoundCloudService::kServiceName = "SoundCloud";
const char* SoundCloudService::kSettingsGroup = "SoundCloud";
const char* SoundCloudService::kUrl = "https://api.soundcloud.com/";
const char* SoundCloudService::kHomepage = "http://soundcloud.com/";

const int SoundCloudService::kSearchDelayMsec = 400;
const int SoundCloudService::kSongSearchLimit = 100;
const int SoundCloudService::kSongSimpleSearchLimit = 10;

typedef QPair<QString, QString> Param;

SoundCloudService::SoundCloudService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_(nullptr),
      search_(nullptr),
      network_(new NetworkAccessManager(this)),
      context_menu_(nullptr),
      search_box_(new SearchBoxWidget(this)),
      search_delay_(new QTimer(this)),
      next_pending_search_id_(0) {

  search_delay_->setInterval(kSearchDelayMsec);
  search_delay_->setSingleShot(true);
  connect(search_delay_, SIGNAL(timeout()), SLOT(DoSearch()));

  SoundCloudSearchProvider* search_provider =
      new SoundCloudSearchProvider(app_, this);
  search_provider->Init(this);
  app_->global_search()->AddProvider(search_provider);

  connect(search_box_, SIGNAL(TextChanged(QString)), SLOT(Search(QString)));
}

SoundCloudService::~SoundCloudService() {}

QStandardItem* SoundCloudService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/soundcloud.png"), kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  root_->setData(InternetModel::PlayBehaviour_DoubleClickAction,
                 InternetModel::Role_PlayBehaviour);
  return root_;
}

void SoundCloudService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service: {
      EnsureItemsCreated();
      break;
    }
    default:
      break;
  }
}

void SoundCloudService::EnsureItemsCreated() {
  search_ =
      new QStandardItem(IconLoader::Load("edit-find"), tr("Search results"));
  search_->setToolTip(
      tr("Start typing something on the search box above to "
         "fill this search results list"));
  search_->setData(InternetModel::PlayBehaviour_MultipleItems,
                   InternetModel::Role_PlayBehaviour);
  root_->appendRow(search_);
}

QWidget* SoundCloudService::HeaderWidget() const { return search_box_; }

void SoundCloudService::Homepage() {
  QDesktopServices::openUrl(QUrl(kHomepage));
}

void SoundCloudService::Search(const QString& text, bool now) {
  pending_search_ = text;

  // If there is no text (e.g. user cleared search box), we don't need to do a
  // real query that will return nothing: we can clear the playlist now
  if (text.isEmpty()) {
    search_delay_->stop();
    ClearSearchResults();
    return;
  }

  if (now) {
    search_delay_->stop();
    DoSearch();
  } else {
    search_delay_->start();
  }
}

void SoundCloudService::DoSearch() {
  ClearSearchResults();

  QList<Param> parameters;
  parameters << Param("q", pending_search_);
  QNetworkReply* reply = CreateRequest("tracks", parameters);
  const int id = next_pending_search_id_++;
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SearchFinished(QNetworkReply*, int)), reply, id);
}

void SoundCloudService::SearchFinished(QNetworkReply* reply, int task_id) {
  reply->deleteLater();

  SongList songs = ExtractSongs(ExtractResult(reply));
  // Fill results list
  foreach(const Song & song, songs) {
    QStandardItem* child = CreateSongItem(song);
    search_->appendRow(child);
  }

  QModelIndex index = model()->merged_model()->mapFromSource(search_->index());
  ScrollToIndex(index);
}

void SoundCloudService::ClearSearchResults() {
  if (search_) search_->removeRows(0, search_->rowCount());
}

int SoundCloudService::SimpleSearch(const QString& text) {
  QList<Param> parameters;
  parameters << Param("q", text);
  QNetworkReply* reply = CreateRequest("tracks", parameters);
  const int id = next_pending_search_id_++;
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SimpleSearchFinished(QNetworkReply*, int)), reply, id);
  return id;
}

void SoundCloudService::SimpleSearchFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  SongList songs = ExtractSongs(ExtractResult(reply));
  emit SimpleSearchResults(id, songs);
}

void SoundCloudService::EnsureMenuCreated() {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("download"),
                             tr("Open %1 in browser").arg("soundcloud.com"),
                             this, SLOT(Homepage()));
  }
}

void SoundCloudService::ShowContextMenu(const QPoint& global_pos) {
  EnsureMenuCreated();

  context_menu_->popup(global_pos);
}

QNetworkReply* SoundCloudService::CreateRequest(const QString& ressource_name,
                                                const QList<Param>& params) {

  QUrl url(kUrl);

  url.setPath(ressource_name);

  url.addQueryItem("client_id", kApiClientId);
  foreach(const Param & param, params) {
    url.addQueryItem(param.first, param.second);
  }

  qLog(Debug) << "Request Url: " << url.toEncoded();

  QNetworkRequest req(url);
  req.setRawHeader("Accept", "application/json");
  QNetworkReply* reply = network_->get(req);
  return reply;
}

QVariant SoundCloudService::ExtractResult(QNetworkReply* reply) {
  QJson::Parser parser;
  bool ok;
  QVariant result = parser.parse(reply, &ok);
  if (!ok) {
    qLog(Error) << "Error while parsing SoundCloud result";
  }
  return result;
}

SongList SoundCloudService::ExtractSongs(const QVariant& result) {
  SongList songs;

  QVariantList q_variant_list = result.toList();
  foreach(const QVariant & q, q_variant_list) {
    Song song = ExtractSong(q.toMap());
    if (song.is_valid()) {
      songs << song;
    }
  }
  return songs;
}

Song SoundCloudService::ExtractSong(const QVariantMap& result_song) {
  Song song;
  if (!result_song.isEmpty() && result_song["streamable"].toBool()) {
    QUrl stream_url = result_song["stream_url"].toUrl();
    stream_url.addQueryItem("client_id", kApiClientId);
    song.set_url(stream_url);

    QString username = result_song["user"].toMap()["username"].toString();
    // We don't have a real artist name, but username is the most similar thing
    // we have
    song.set_artist(username);

    QString title = result_song["title"].toString();
    song.set_title(title);

    QString genre = result_song["genre"].toString();
    song.set_genre(genre);

    float bpm = result_song["bpm"].toFloat();
    song.set_bpm(bpm);

    QVariant cover = result_song["artwork_url"];
    if (cover.isValid()) {
      // SoundCloud covers URL are https, but our cover loader doesn't seem to
      // deal well with https URL. Anyway, we don't need a secure connection to
      // get a cover image.
      QUrl cover_url = cover.toUrl();
      cover_url.setScheme("http");
      song.set_art_automatic(cover_url.toEncoded());
    }

    int playcount = result_song["playback_count"].toInt();
    song.set_playcount(playcount);

    int year = result_song["release_year"].toInt();
    song.set_year(year);

    QVariant q_duration = result_song["duration"];
    quint64 duration = q_duration.toULongLong() * kNsecPerMsec;
    song.set_length_nanosec(duration);

    song.set_valid(true);
  }
  return song;
}
