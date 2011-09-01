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


#include <QMenu>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include "qtiocompressor.h"

#include "internetmodel.h"
#include "groovesharksearchplaylisttype.h"

#include "core/database.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/player.h"
#include "core/scopedtransaction.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "playlist/playlist.h"
#include "playlist/playlistcontainer.h"
#include "playlist/playlistmanager.h"
#include "ui/iconloader.h"

#include "groovesharkservice.h"

// The GrooveShark terms of service require that application keys are not
// accessible to third parties. Therefore this application key is ofuscated to
// prevent third parties from viewing it.
const char* GrooveSharkService::kApiKey = "clementineplayer";
const char* GrooveSharkService::kApiSecret = "MWVlNmU1N2IzNGY3MjA1ZTg1OWJkMTllNjk4YzEzZjY";

const char* GrooveSharkService::kServiceName = "GrooveShark";
const char* GrooveSharkService::kUrl = "http://api.grooveshark.com/ws/3.0/";

const int GrooveSharkService::kSongSearchLimit = 50;

typedef QPair<QString, QString> Param;

GrooveSharkService::GrooveSharkService(InternetModel *parent)
  : InternetService(kServiceName, parent, parent),
    pending_search_playlist_(NULL),
    root_(NULL),
    search_(NULL),
    network_(new NetworkAccessManager(this)),
    context_menu_(NULL),
    session_id_(NULL),
    api_key_(QByteArray::fromBase64(kApiSecret)) {

  model()->player()->playlists()->RegisterSpecialPlaylistType(new GrooveSharkSearchPlaylistType(this));
}


GrooveSharkService::~GrooveSharkService() {
}

QStandardItem* GrooveSharkService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/grooveshark.png"), kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  if (!search_) {
    search_ = new QStandardItem(IconLoader::Load("edit-find"),
                                tr("Search GrooveShark (opens a new tab)"));
    search_->setData(Type_SearchResults, InternetModel::Role_Type);
    search_->setData(InternetModel::PlayBehaviour_DoubleClickAction,
                             InternetModel::Role_PlayBehaviour);
    root_->appendRow(search_);
  }

  return root_;
}

void GrooveSharkService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service: {
      EnsureConnected();
      break;
    }
    default:
      break;
  }
}

void GrooveSharkService::Search(const QString& text, Playlist* playlist, bool now) {
  QList<Param> parameters;
  QNetworkReply *reply;

  pending_search_playlist_ = playlist;

  parameters  << Param("query", text)
              << Param("country", "")
              << Param("limit", QString("%1").arg(kSongSearchLimit))
              << Param("offset", "");
  reply = CreateRequest("getSongSearchResults", parameters, false);
  connect(reply, SIGNAL(finished()), SLOT(SearchSongsFinished()));
}

void GrooveSharkService::ShowContextMenu(const QModelIndex& index, const QPoint& global_pos) {
  EnsureMenuCreated();
  context_menu_->popup(global_pos);
}

QModelIndex GrooveSharkService::GetCurrentIndex() {
  return context_item_;
}

void GrooveSharkService::UpdateTotalSongCount(int count) {
}

void GrooveSharkService::SearchSongsFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  reply->deleteLater();

  QJson::Parser parser;
  bool ok;
  QVariantMap result = parser.parse(reply, &ok).toMap();
  if (!ok) {
    qLog(Error) << "Error while parsing GrooveShark result";
  }
  qLog(Debug) << result;
  QVariantList result_songs = result["result"].toMap()["songs"].toList();
  SongList songs;
  for (int i=0; i<result_songs.size(); ++i) {
    QVariantMap result_song = result_songs[i].toMap();
    Song song;
    int song_id = result_song["SongID"].toInt();
    QString song_name = result_song["SongName"].toString();
    QString artist_name = result_song["ArtistName"].toString();
    QString album_name = result_song["AlbumName"].toString();
    song.Init(song_name, artist_name, album_name, 0);
    song.set_id(song_id);
    // Special kind of URL: because we need to request a stream key for each
    // play, we generate fake URL for now, and we will create a real streaming
    // URL when user will actually play the song.
    // TODO: Implement an UrlHandler
    song.set_url(QString("grooveshark://%1").arg(song_id));
    songs << song;
  }
  pending_search_playlist_->Clear();
  pending_search_playlist_->InsertSongs(songs);
}

void GrooveSharkService::EnsureMenuCreated() {
  if(!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
  }
}

void GrooveSharkService::EnsureConnected() {
  if (!session_id_.isEmpty()) {
    return;
  }
  // TODO
}

void GrooveSharkService::OpenSearchTab() {
  model()->player()->playlists()->New(tr("Search GrooveShark"), SongList(),
                                      GrooveSharkSearchPlaylistType::kName);
}

void GrooveSharkService::ItemDoubleClicked(QStandardItem* item) {
  if (item == search_) {
    OpenSearchTab();
  }
}

QNetworkReply *GrooveSharkService::CreateRequest(const QString& method_name, QList<Param> params,
                                       bool need_authentication) {
  QVariantMap request_params;

  request_params.insert("method", method_name);

  QVariantMap wsKey;
  wsKey.insert("wsKey", kApiKey);
  request_params.insert("header", wsKey);

  QVariantMap parameters;
  foreach(const Param& param, params) {
    parameters.insert(param.first, param.second);
  }
  request_params.insert("parameters", parameters);

  QJson::Serializer serializer;
  QByteArray post_params = serializer.serialize(request_params);

  qLog(Debug) << post_params;

  QUrl url(kUrl);
  url.setQueryItems( QList<Param>() << Param("sig", Utilities::HmacMd5(api_key_, post_params).toHex()));
  QNetworkRequest req(url);
  QNetworkReply *reply = network_->post(req, post_params);

  return reply;
}
