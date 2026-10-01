/* This file is part of Clementine.
   Copyright 2010-2013, David Sansome <me@davidsansome.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
   Copyright 2014, Chocobozzz <florian.bigard@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "jamendoservice.h"

#include <QDesktopServices>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenu>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "core/application.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "internet/jamendo/jamendourlhandler.h"
#include "ui/iconloader.h"

const char* JamendoService::kServiceName = "Jamendo";
const char* JamendoService::kSettingsGroup = "Jamendo";
const char* JamendoService::kUrlScheme = "jamendo";

namespace {

const char* kApiUrl = "https://api.jamendo.com/v3.0/";
// Registered at https://devportal.jamendo.com for Clementine. Every request
// counts against its quota, which every Clementine user shares.
const char* kClientId = "71a779eb";
// mp32 is Jamendo's VBR MP3, its best quality that doesn't need a login.
// "from" credits the plays to Clementine.
const char* kStreamUrl =
    "https://prod-1.storage.jamendo.com/?trackid=%1&format=mp32&from=app-%2";
const char* kHomepage = "https://www.jamendo.com/";

// How many tracks or albums to list at once.
const int kListLimit = 50;
// The API sometimes answers with no results for a query that has some, so
// a request that comes back empty is tried again this many times.
const int kEmptyRetries = 1;

struct Genre {
  const char* tag;
  const char* name;
};

// Some of Jamendo's music tags, which it groups its catalogue by.
const Genre kGenres[] = {
    {"ambient", QT_TRANSLATE_NOOP("JamendoService", "Ambient")},
    {"blues", QT_TRANSLATE_NOOP("JamendoService", "Blues")},
    {"chillout", QT_TRANSLATE_NOOP("JamendoService", "Chillout")},
    {"classical", QT_TRANSLATE_NOOP("JamendoService", "Classical")},
    {"country", QT_TRANSLATE_NOOP("JamendoService", "Country")},
    {"dance", QT_TRANSLATE_NOOP("JamendoService", "Dance")},
    {"electronic", QT_TRANSLATE_NOOP("JamendoService", "Electronic")},
    {"folk", QT_TRANSLATE_NOOP("JamendoService", "Folk")},
    {"funk", QT_TRANSLATE_NOOP("JamendoService", "Funk")},
    {"hiphop", QT_TRANSLATE_NOOP("JamendoService", "Hip-hop")},
    {"jazz", QT_TRANSLATE_NOOP("JamendoService", "Jazz")},
    {"latin", QT_TRANSLATE_NOOP("JamendoService", "Latin")},
    {"lounge", QT_TRANSLATE_NOOP("JamendoService", "Lounge")},
    {"metal", QT_TRANSLATE_NOOP("JamendoService", "Metal")},
    {"pop", QT_TRANSLATE_NOOP("JamendoService", "Pop")},
    {"punk", QT_TRANSLATE_NOOP("JamendoService", "Punk")},
    {"reggae", QT_TRANSLATE_NOOP("JamendoService", "Reggae")},
    {"rock", QT_TRANSLATE_NOOP("JamendoService", "Rock")},
    {"soundtrack", QT_TRANSLATE_NOOP("JamendoService", "Soundtrack")},
    {"world", QT_TRANSLATE_NOOP("JamendoService", "World")},
};

}  // namespace

JamendoService::JamendoService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      network_(new NetworkAccessManager(this)),
      url_handler_(new JamendoUrlHandler(this)),
      root_(nullptr),
      search_results_(nullptr),
      search_box_(new QLineEdit),
      open_share_url_(nullptr),
      refresh_(nullptr) {
  app_->player()->RegisterUrlHandler(url_handler_);

  // The local copy of Jamendo's catalogue older versions kept.
  const QString old_database =
      Utilities::GetConfigPath(Utilities::Path_Root) + "/jamendo.db";
  if (QFile::exists(old_database) && QFile::remove(old_database)) {
    qLog(Info) << "Removed" << old_database;
  }

  search_box_->setPlaceholderText(tr("Search Jamendo"));
  search_box_->setClearButtonEnabled(true);
  connect(search_box_, &QLineEdit::returnPressed, this,
          &JamendoService::Search);
}

JamendoService::~JamendoService() { delete search_box_; }

QUrl JamendoService::StreamUrl(const QString& track_id) {
  return QUrl(QString(kStreamUrl).arg(track_id, kClientId));
}

QStandardItem* JamendoService::CreateRootItem() {
  root_ = new QStandardItem(IconLoader::Load("jamendo", IconLoader::Provider),
                            kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

QWidget* JamendoService::HeaderWidget() const { return search_box_; }

QStandardItem* JamendoService::CreateList(const QString& text, Type type,
                                          const QString& query) {
  QStandardItem* item = new QStandardItem(
      IconLoader::Load("folder-sound", IconLoader::Base), text);
  item->setData(type, InternetModel::Role_Type);
  item->setData(query, Role_Query);
  item->setData(true, InternetModel::Role_CanLazyLoad);
  if (type == Type_Tracks) {
    item->setData(InternetModel::PlayBehaviour_MultipleItems,
                  InternetModel::Role_PlayBehaviour);
  }
  return item;
}

void JamendoService::PopulateRoot() {
  root_->appendRow(CreateList(tr("Popular this week"), Type_Tracks,
                              "order=popularity_week"));
  root_->appendRow(CreateList(tr("Popular this month"), Type_Tracks,
                              "order=popularity_month"));
  root_->appendRow(CreateList(tr("Most popular of all time"), Type_Tracks,
                              "order=popularity_total"));
  root_->appendRow(
      CreateList(tr("Popular albums"), Type_Albums, "order=popularity_month"));
  root_->appendRow(
      CreateList(tr("New albums"), Type_Albums, "order=releasedate_desc"));

  QStandardItem* genres = new QStandardItem(
      IconLoader::Load("folder-sound", IconLoader::Base), tr("Genres"));
  genres->setData(Type_Genres, InternetModel::Role_Type);
  for (const Genre& genre : kGenres) {
    QUrlQuery query;
    query.addQueryItem("tags", genre.tag);
    query.addQueryItem("order", "popularity_month");
    genres->appendRow(CreateList(tr(genre.name), Type_Tracks,
                                 query.toString(QUrl::FullyEncoded)));
  }
  root_->appendRow(genres);
}

void JamendoService::LazyPopulate(QStandardItem* item) {
  if (item == root_) {
    PopulateRoot();
  } else {
    Fetch(item);
  }
}

void JamendoService::Fetch(QStandardItem* item) {
  QUrlQuery query(item->data(Role_Query).toString());
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case Type_Tracks:
      query.addQueryItem("limit", QString::number(kListLimit));
      Request(item, "tracks", query, kEmptyRetries);
      break;
    case Type_Albums:
      query.addQueryItem("limit", QString::number(kListLimit));
      Request(item, "albums", query, kEmptyRetries);
      break;
    case Type_Album:
      // (tracks?album_id= finds nothing.)
      query.addQueryItem("id", item->data(Role_Id).toString());
      Request(item, "albums/tracks", query, kEmptyRetries);
      break;
    default:
      break;
  }
}

void JamendoService::Request(QStandardItem* item, const QString& endpoint,
                             const QUrlQuery& query, int retries_left) {
  QUrlQuery full_query(query);
  full_query.addQueryItem("client_id", kClientId);
  full_query.addQueryItem("format", "json");

  QUrl url(kApiUrl + endpoint + "/");
  url.setQuery(full_query);

  const int task_id =
      app_->task_manager()->StartTask(tr("Loading %1").arg(item->text()));
  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  const QPersistentModelIndex index(item->index());
  connect(reply, &QNetworkReply::finished, this, [=] {
    RequestFinished(reply, index, endpoint, query, retries_left, task_id);
  });
}

void JamendoService::RequestFinished(QNetworkReply* reply,
                                     const QPersistentModelIndex& index,
                                     const QString& endpoint,
                                     const QUrlQuery& query, int retries_left,
                                     int task_id) {
  reply->deleteLater();
  app_->task_manager()->SetTaskFinished(task_id);

  // The list might have been refreshed or searched again since.
  if (!index.isValid()) return;
  QStandardItem* item = model()->itemFromIndex(index);

  QJsonDocument document = ParseJsonReply(reply);
  if (document.isNull()) return;

  QJsonObject headers = document.object()["headers"].toObject();
  if (headers["status"].toString() != "success") {
    // Eg. code 6: the quota has run out.
    qLog(Error) << "Jamendo request failed:" << headers;
    app_->AddError(tr("Jamendo request failed:\n%1")
                       .arg(headers["error_message"].toString()));
    return;
  }

  QJsonArray results = document.object()["results"].toArray();
  if (results.isEmpty() && retries_left > 0) {
    Request(item, endpoint, query, retries_left - 1);
    return;
  }

  if (item->hasChildren()) item->removeRows(0, item->rowCount());
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case Type_Tracks:
      AddTracks(item, results, true);
      break;
    case Type_Albums:
      AddAlbums(item, results);
      break;
    case Type_Album:
      AddTracks(item, AlbumTracks(results.first().toObject()), false);
      break;
  }
}

QJsonArray JamendoService::AlbumTracks(const QJsonObject& album) {
  // An album's tracks leave out what they share with it.
  QJsonArray ret;
  for (const QJsonValue& value : album["tracks"].toArray()) {
    QJsonObject track = value.toObject();
    track["artist_name"] = album["artist_name"];
    track["album_name"] = album["name"];
    track["image"] = album["image"];
    track["releasedate"] = album["releasedate"];
    track["shareurl"] =
        QString("https://www.jamendo.com/track/%1").arg(track["id"].toString());
    ret << track;
  }
  return ret;
}

Song JamendoService::TrackToSong(const QJsonObject& track) const {
  Song song;
  song.set_valid(true);
  song.set_title(track["name"].toString());
  song.set_artist(track["artist_name"].toString());
  song.set_album(track["album_name"].toString());
  // Numbers, or strings in an album's tracks.
  const int position = track["position"].toVariant().toInt();
  if (position > 0) song.set_track(position);
  song.set_length_nanosec(track["duration"].toVariant().toLongLong() *
                          kNsecPerSec);
  song.set_year(track["releasedate"].toString().left(4).toInt());
  song.set_art_automatic(track["image"].toString());
  song.set_url(
      QUrl(QString("%1://track/%2").arg(kUrlScheme, track["id"].toString())));
  return song;
}

void JamendoService::AddTracks(QStandardItem* parent, const QJsonArray& tracks,
                               bool with_artist) {
  QList<QJsonObject> sorted;
  for (const QJsonValue& value : tracks) sorted << value.toObject();
  // An album's tracks, in order.
  if (!with_artist) {
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const QJsonObject& a, const QJsonObject& b) {
                       return a["position"].toVariant().toInt() <
                              b["position"].toVariant().toInt();
                     });
  }

  for (const QJsonObject& track : sorted) {
    const Song song = TrackToSong(track);
    const QString text =
        with_artist ? QString("%1 - %2").arg(song.artist(), song.title())
                    : song.title();

    QStandardItem* item = new QStandardItem(
        IconLoader::Load("audio-x-generic", IconLoader::Base), text);
    item->setData(InternetModel::Type_Track, InternetModel::Role_Type);
    item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
    item->setData(InternetModel::PlayBehaviour_SingleItem,
                  InternetModel::Role_PlayBehaviour);
    item->setData(track["shareurl"].toString(), Role_ShareUrl);
    parent->appendRow(item);
  }
}

void JamendoService::AddAlbums(QStandardItem* parent,
                               const QJsonArray& albums) {
  for (const QJsonValue& value : albums) {
    QJsonObject album = value.toObject();
    QStandardItem* item = new QStandardItem(
        IconLoader::Load("x-clementine-album", IconLoader::Base),
        QString("%1 - %2").arg(album["artist_name"].toString(),
                               album["name"].toString()));
    item->setData(Type_Album, InternetModel::Role_Type);
    item->setData(album["id"].toString(), Role_Id);
    item->setData(album["shareurl"].toString(), Role_ShareUrl);
    item->setData(true, InternetModel::Role_CanLazyLoad);
    item->setData(InternetModel::PlayBehaviour_MultipleItems,
                  InternetModel::Role_PlayBehaviour);
    parent->appendRow(item);
  }
}

void JamendoService::Search() {
  const QString text = search_box_->text().trimmed();
  if (text.isEmpty()) return;

  // The root's lists come first, so the results can go above them.
  if (root_->data(InternetModel::Role_CanLazyLoad).toBool()) {
    root_->setData(false, InternetModel::Role_CanLazyLoad);
    PopulateRoot();
  }

  // A new list for each search, so a slow reply to an earlier one can't fill
  // it in.
  if (search_results_) root_->removeRow(search_results_->row());
  QUrlQuery query;
  query.addQueryItem("search", text);
  query.addQueryItem("boost", "popularity_total");
  search_results_ = CreateList(tr("Search results for \"%1\"").arg(text),
                               Type_Tracks, query.toString(QUrl::FullyEncoded));
  search_results_->setData(false, InternetModel::Role_CanLazyLoad);
  root_->insertRow(0, search_results_);

  Fetch(search_results_);
  emit ScrollToIndex(search_results_->index());
}

void JamendoService::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_.reset(new QMenu);
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addSeparator();
    open_share_url_ = context_menu_->addAction(
        IconLoader::Load("applications-internet", IconLoader::Base),
        tr("Open on Jamendo"), this, SLOT(OpenShareUrl()));
    refresh_ = context_menu_->addAction(
        IconLoader::Load("view-refresh", IconLoader::Base), tr("Refresh"), this,
        SLOT(Refresh()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("download", IconLoader::Base),
                             tr("Open %1 in browser").arg("jamendo.com"), this,
                             SLOT(Homepage()));
  }

  context_index_ = QPersistentModelIndex(model()->current_index());
  QStandardItem* item = context_index_.isValid()
                            ? model()->itemFromIndex(context_index_)
                            : nullptr;
  const int type = item ? item->data(InternetModel::Role_Type).toInt() : -1;
  const bool playable =
      item && item->data(InternetModel::Role_PlayBehaviour).toInt() !=
                  InternetModel::PlayBehaviour_None;

  for (QAction* action : GetPlaylistActions()) action->setEnabled(playable);
  open_share_url_->setVisible(item &&
                              !item->data(Role_ShareUrl).toString().isEmpty());
  refresh_->setVisible(type == Type_Tracks || type == Type_Albums ||
                       type == Type_Album);

  context_menu_->popup(global_pos);
}

void JamendoService::OpenShareUrl() {
  if (!context_index_.isValid()) return;
  QDesktopServices::openUrl(
      QUrl(context_index_.data(Role_ShareUrl).toString()));
}

void JamendoService::Refresh() {
  if (!context_index_.isValid()) return;
  QStandardItem* item = model()->itemFromIndex(context_index_);
  if (item) Fetch(item);
}

void JamendoService::Homepage() { QDesktopServices::openUrl(QUrl(kHomepage)); }
