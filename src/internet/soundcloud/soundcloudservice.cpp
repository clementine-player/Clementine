/* This file is part of Clementine.
   Copyright 2012, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, maximko <me@maximko.org>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>

#include "internet/core/internetmodel.h"
#include "internet/core/oauthenticator.h"
#include "internet/core/searchboxwidget.h"

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
const char* SoundCloudService::kApiClientSecret =
    "d1cd7829da2e98e1e0621d85d57a2077";

const char* SoundCloudService::kServiceName = "SoundCloud";
const char* SoundCloudService::kSettingsGroup = "SoundCloud";
const char* SoundCloudService::kUrl = "https://api.soundcloud.com/";
const char* SoundCloudService::kOAuthEndpoint = "https://soundcloud.com/connect";
const char* SoundCloudService::kOAuthTokenEndpoint = "https://api.soundcloud.com/oauth2/token";
const char* SoundCloudService::kOAuthScope = "non-expiring";
const char* SoundCloudService::kHomepage = "http://soundcloud.com/";

const int SoundCloudService::kSearchDelayMsec = 400;
const int SoundCloudService::kSongSearchLimit = 100;
const int SoundCloudService::kSongSimpleSearchLimit = 10;

typedef QPair<QString, QString> Param;

SoundCloudService::SoundCloudService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_(nullptr),
      search_(nullptr),
      user_tracks_(nullptr),
      user_playlists_(nullptr),
      user_activities_(nullptr),
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
  root_ = new QStandardItem(IconLoader::Load("soundcloud", 
                            IconLoader::Provider), kServiceName);
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
  if (!search_) {
    search_ =
        new QStandardItem(IconLoader::Load("edit-find", IconLoader::Base), 
                          tr("Search results"));
    search_->setToolTip(
        tr("Start typing something on the search box above to "
           "fill this search results list"));
    search_->setData(InternetModel::PlayBehaviour_MultipleItems,
                     InternetModel::Role_PlayBehaviour);
    root_->appendRow(search_);
  }
  if (!user_tracks_ && !user_activities_ && !user_playlists_ && IsLoggedIn()) {
    user_activities_ = new QStandardItem(tr("Activities stream"));
    user_activities_->setData(InternetModel::PlayBehaviour_MultipleItems,
                              InternetModel::Role_PlayBehaviour);
    root_->appendRow(user_activities_);

    user_playlists_ = new QStandardItem(tr("Playlists"));
    root_->appendRow(user_playlists_);

    user_tracks_ = new QStandardItem(tr("Tracks"));
    user_tracks_->setData(InternetModel::PlayBehaviour_MultipleItems,
                          InternetModel::Role_PlayBehaviour);
    root_->appendRow(user_tracks_);

    RetrieveUserData();  // at least, try to (this will do nothing if user isn't
                         // logged)
  }
}

QWidget* SoundCloudService::HeaderWidget() const { return search_box_; }

void SoundCloudService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_SoundCloud);
}

void SoundCloudService::Homepage() {
  QDesktopServices::openUrl(QUrl(kHomepage));
}

void SoundCloudService::Connect() {
  OAuthenticator* oauth = new OAuthenticator(
      kApiClientId, kApiClientSecret,
      OAuthenticator::RedirectStyle::REMOTE_WITH_STATE, this);

  oauth->StartAuthorisation(kOAuthEndpoint, kOAuthTokenEndpoint, kOAuthScope);

  NewClosure(oauth, SIGNAL(Finished()), this,
             SLOT(ConnectFinished(OAuthenticator*)), oauth);
}

void SoundCloudService::ConnectFinished(OAuthenticator* oauth) {
  oauth->deleteLater();

  access_token_ = oauth->access_token();
  if (!access_token_.isEmpty()) {
    emit Connected();
  }
  expiry_time_ = oauth->expiry_time();
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("access_token", access_token_);

  EnsureItemsCreated();
}

void SoundCloudService::LoadAccessTokenIfEmpty() {
  if (access_token_.isEmpty()) {
    QSettings s;
    s.beginGroup(kSettingsGroup);
    if (!s.contains("access_token")) {
      return;
    }
    access_token_ = s.value("access_token").toString();
  }
}

bool SoundCloudService::IsLoggedIn() {
  LoadAccessTokenIfEmpty();
  return !access_token_.isEmpty();
}

void SoundCloudService::Logout() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  access_token_.clear();
  s.remove("access_token");
  pending_playlists_requests_.clear();
  if (user_activities_) root_->removeRow(user_activities_->row());
  if (user_tracks_) root_->removeRow(user_tracks_->row());
  if (user_playlists_) root_->removeRow(user_playlists_->row());
  user_activities_ = nullptr;
  user_tracks_ = nullptr;
  user_playlists_ = nullptr;
}

void SoundCloudService::RetrieveUserData() {
  LoadAccessTokenIfEmpty();
  RetrieveUserActivities();
  RetrieveUserTracks();
  RetrieveUserPlaylists();
}

void SoundCloudService::RetrieveUserTracks() {
  QList<Param> parameters;
  parameters << Param("oauth_token", access_token_);
  QNetworkReply* reply = CreateRequest("/me/tracks", parameters);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(UserTracksRetrieved(QNetworkReply*)), reply);
}

void SoundCloudService::UserTracksRetrieved(QNetworkReply* reply) {
  reply->deleteLater();

  SongList songs = ExtractSongs(ExtractResult(reply).array());
  // Fill results list
  for (const Song& song : songs) {
    QStandardItem* child = CreateSongItem(song);
    user_tracks_->appendRow(child);
  }
}

void SoundCloudService::RetrieveUserActivities() {
  QList<Param> parameters;
  parameters << Param("oauth_token", access_token_);
  QNetworkReply* reply = CreateRequest("/me/activities", parameters);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(UserActivitiesRetrieved(QNetworkReply*)), reply);
}

void SoundCloudService::UserActivitiesRetrieved(QNetworkReply* reply) {
  reply->deleteLater();

  QList<QStandardItem*> activities = ExtractActivities(ExtractResult(reply).object());
  // Fill results list
  for (QStandardItem* activity : activities) {
    user_activities_->appendRow(activity);
  }
}

void SoundCloudService::RetrieveUserPlaylists() {
  QList<Param> parameters;
  parameters << Param("oauth_token", access_token_);
  QNetworkReply* reply = CreateRequest("/me/playlists", parameters);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(UserPlaylistsRetrieved(QNetworkReply*)), reply);
}

void SoundCloudService::UserPlaylistsRetrieved(QNetworkReply* reply) {
  reply->deleteLater();

  QJsonArray json_playlists = ExtractResult(reply).array();
  for (const QJsonValue& playlist : json_playlists) {
    QJsonObject json_playlist = playlist.toObject();

    QStandardItem* playlist_item = CreatePlaylistItem(json_playlist["title"].toString());
    SongList songs = ExtractSongs(json_playlist["tracks"].toArray());
    for (const Song& song : songs) {
      playlist_item->appendRow(CreateSongItem(song));
    }
    user_playlists_->appendRow(playlist_item);
  }
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
  QNetworkReply* reply = CreateRequest("/tracks", parameters);
  const int id = next_pending_search_id_++;
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SearchFinished(QNetworkReply*, int)), reply, id);
}

void SoundCloudService::SearchFinished(QNetworkReply* reply, int task_id) {
  reply->deleteLater();

  SongList songs = ExtractSongs(ExtractResult(reply).array());
  // Fill results list
  for (const Song& song : songs) {
    QStandardItem* child = CreateSongItem(song);
    search_->appendRow(child);
  }

  QModelIndex index = model()->merged_model()->mapFromSource(search_->index());
  ScrollToIndex(index);
}

void SoundCloudService::ClearSearchResults() {
  if (search_) {
    search_->removeRows(0, search_->rowCount());
  }
}

int SoundCloudService::SimpleSearch(const QString& text) {
  QList<Param> parameters;
  parameters << Param("q", text);
  QNetworkReply* reply = CreateRequest("/tracks", parameters);
  const int id = next_pending_search_id_++;
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SimpleSearchFinished(QNetworkReply*, int)), reply, id);
  return id;
}

void SoundCloudService::SimpleSearchFinished(QNetworkReply* reply, int id) {
  reply->deleteLater();

  SongList songs = ExtractSongs(ExtractResult(reply).array());
  emit SimpleSearchResults(id, songs);
}

void SoundCloudService::EnsureMenuCreated() {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("download", IconLoader::Base),
                             tr("Open %1 in browser").arg("soundcloud.com"),
                             this, SLOT(Homepage()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("configure", IconLoader::Base),
                             tr("Configure SoundCloud..."),
                             this, SLOT(ShowConfig()));
  }
}

void SoundCloudService::ShowContextMenu(const QPoint& global_pos) {
  EnsureMenuCreated();

  context_menu_->popup(global_pos);
}

QStandardItem* SoundCloudService::CreatePlaylistItem(const QString& playlist_name) {
  QStandardItem* item = new QStandardItem(playlist_name);
  item->setData(true, InternetModel::Role_CanLazyLoad);
  item->setData(InternetModel::PlayBehaviour_MultipleItems,
                InternetModel::Role_PlayBehaviour);
  return item;
}

QNetworkReply* SoundCloudService::CreateRequest(const QString& ressource_name,
                                                const QList<Param>& params) {
  QUrl url(kUrl);

  url.setPath(ressource_name);
  QUrlQuery url_query;

  url_query.addQueryItem("client_id", kApiClientId);
  for (const Param& param : params) {
    url_query.addQueryItem(param.first, param.second);
  }

  url.setQuery(url_query);

  qLog(Debug) << "Request Url: " << url.toEncoded();

  QNetworkRequest req(url);
  req.setRawHeader("Accept", "application/json");
  QNetworkReply* reply = network_->get(req);
  return reply;
}

QJsonDocument SoundCloudService::ExtractResult(QNetworkReply* reply) {
  if (reply->error() != QNetworkReply::NoError) {
    qLog(Error) << "Error when retrieving SoundCloud results:"
                << reply->errorString() << QString(" (%1)").arg(reply->error());
    if (reply->error() == QNetworkReply::ContentAccessDenied ||
        reply->error() == QNetworkReply::ContentOperationNotPermittedError ||
        reply->error() == QNetworkReply::ContentNotFoundError ||
        reply->error() == QNetworkReply::AuthenticationRequiredError) {
      // In case of access denied errors (invalid token?) logout
      Logout();
      return QJsonDocument();
    }
  }
  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &error);
  if (error.error != QJsonParseError::NoError) {
    qLog(Error) << "Error while parsing SoundCloud result";
  }

  return document;
}

void SoundCloudService::RetrievePlaylist(int playlist_id,
                                         QStandardItem* playlist_item) {
  const int request_id = next_retrieve_playlist_id_++;
  pending_playlists_requests_.insert(request_id,
                                     PlaylistInfo(playlist_id, playlist_item));
  QList<Param> parameters;
  parameters << Param("oauth_token", access_token_);
  QNetworkReply* reply =
      CreateRequest("/playlists/" + QString::number(playlist_id), parameters);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(PlaylistRetrieved(QNetworkReply*, int)), reply, request_id);
}

void SoundCloudService::PlaylistRetrieved(QNetworkReply* reply,
                                          int request_id) {
  if (!pending_playlists_requests_.contains(request_id)) return;
  PlaylistInfo playlist_info = pending_playlists_requests_.take(request_id);
  QJsonDocument res = ExtractResult(reply);
  SongList songs = ExtractSongs(res.object()["tracks"].toArray());
  for (const Song& song : songs) {
    QStandardItem* child = CreateSongItem(song);
    playlist_info.item_->appendRow(child);
  }
}

QList<QStandardItem*> SoundCloudService::ExtractActivities(const QJsonObject& result) {
  QList<QStandardItem*> activities;
  QJsonArray q_list = result["collection"].toArray();
  for (const QJsonValue& q : q_list) {
    QJsonObject json_activity = q.toObject();
    const QString type = json_activity["type"].toString();
    if (type == "track") {
      Song song = ExtractSong(json_activity["origin"].toObject());
      if (song.is_valid()) {
        activities << CreateSongItem(song);
      }
    } else if (type == "playlist") {
      QJsonObject json_origin = json_activity["origin"].toObject();
      QStandardItem* playlist_item =
          CreatePlaylistItem(json_origin["title"].toString());
      activities << playlist_item;
      RetrievePlaylist(json_origin["id"].toInt(), playlist_item);
    }
  }
  return activities;
}

SongList SoundCloudService::ExtractSongs(const QJsonArray & result) {
  SongList songs;

  for (const QJsonValue& q : result) {
    Song song = ExtractSong(q.toObject());
    if (song.is_valid()) {
      songs << song;
    }
  }
  return songs;
}

Song SoundCloudService::ExtractSong(const QJsonObject& result_song) {
  Song song;
  if (!result_song.isEmpty() && result_song["streamable"].toBool()) {
    QUrl stream_url(result_song["stream_url"].toString());
    QUrlQuery stream_url_query;
    stream_url_query.addQueryItem("client_id", kApiClientId);
    stream_url.setQuery(stream_url_query);
    song.set_url(stream_url);

    QString username = result_song["user"].toObject()["username"].toString();
    // We don't have a real artist name, but username is the most similar thing
    // we have
    song.set_artist(username);

    QString title = result_song["title"].toString();
    song.set_title(title);

    QString genre = result_song["genre"].toString();
    song.set_genre(genre);

    float bpm = result_song["bpm"].toDouble();
    song.set_bpm(bpm);

    QVariant cover = result_song["artwork_url"];
    if (cover.isValid()) {
      // Increase cover size.
      // See https://developers.soundcloud.com/docs/api/reference#artwork_url
      QString big_cover = cover.toString().replace("large", "t500x500");
      QUrl cover_url(big_cover, QUrl::StrictMode);

      // SoundCloud covers URL are https, but our cover loader doesn't seem to
      // deal well with https URL. Anyway, we don't need a secure connection to
      // get a cover image.
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
