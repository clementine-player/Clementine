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

#include "lastfmservice.h"
#include "lastfmstationdialog.h"
#include "radiomodel.h"
#include "radioplaylistitem.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"

#include <boost/scoped_ptr.hpp>

#include <lastfm/Audioscrobbler>
#include <lastfm/misc.h>
#include <lastfm/RadioStation>
#include <lastfm/ws.h>
#include <lastfm/XmlQuery>

#include <QMenu>
#include <QSettings>

using boost::scoped_ptr;
using lastfm::XmlQuery;

uint qHash(const lastfm::Track& track) {
  return qHash(track.title()) ^
         qHash(track.artist().name()) ^
         qHash(track.album().title());
}

const char* LastFMService::kServiceName = "Last.fm";
const char* LastFMService::kSettingsGroup = "Last.fm";
const char* LastFMService::kAudioscrobblerClientId = "tng";
const char* LastFMService::kApiKey = "75d20fb472be99275392aefa2760ea09";
const char* LastFMService::kSecret = "d3072b60ae626be12be69448f5c46e70";

const char* LastFMService::kUrlArtist   = "lastfm://artist/%1/similarartists";
const char* LastFMService::kUrlTag      = "lastfm://globaltags/%1";
const char* LastFMService::kUrlCustom   = "lastfm://rql/%1";

const char* LastFMService::kTitleArtist = QT_TR_NOOP("Last.fm Similar Artists to %1");
const char* LastFMService::kTitleTag    = QT_TR_NOOP("Last.fm Tag Radio: %1");
const char* LastFMService::kTitleCustom = QT_TR_NOOP("Last.fm Custom Radio: %1");

LastFMService::LastFMService(RadioModel* parent)
  : RadioService(kServiceName, parent),
    scrobbler_(NULL),
    station_dialog_(new LastFMStationDialog),
    context_menu_(new QMenu),
    initial_tune_(false),
    tune_task_id_(0),
    scrobbling_enabled_(false),
    artist_list_(NULL),
    tag_list_(NULL),
    custom_list_(NULL),
    friends_list_(NULL),
    neighbours_list_(NULL)
{
  ReloadSettings();

  play_action_ = context_menu_->addAction(
      IconLoader::Load("media-playback-start"), tr("Add to playlist"), this, SLOT(AddToPlaylist()));
  load_action_ = context_menu_->addAction(
      IconLoader::Load("media-playback-start"), tr("Load"), this, SLOT(LoadToPlaylist()));
  remove_action_ = context_menu_->addAction(
      IconLoader::Load("list-remove"), tr("Remove"), this, SLOT(Remove()));
  context_menu_->addSeparator();
  add_artist_action_ = context_menu_->addAction(
      QIcon(":last.fm/icon_radio.png"), tr("Play artist radio..."), this, SLOT(AddArtistRadio()));
  add_tag_action_ = context_menu_->addAction(
      QIcon(":last.fm/icon_tag.png"), tr("Play tag radio..."), this, SLOT(AddTagRadio()));
  add_custom_action_ = context_menu_->addAction(
      QIcon(":last.fm/icon_radio.png"), tr("Play custom radio..."), this, SLOT(AddCustomRadio()));
  context_menu_->addAction(
      IconLoader::Load("configure"), tr("Configure Last.fm..."), this, SLOT(ShowConfig()));

  remove_action_->setEnabled(false);
  add_artist_action_->setEnabled(false);
  add_tag_action_->setEnabled(false);
  add_custom_action_->setEnabled(false);
}

LastFMService::~LastFMService() {
}

void LastFMService::ReloadSettings() {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  lastfm::ws::Username = settings.value("Username").toString();
  lastfm::ws::SessionKey = settings.value("Session").toString();
  scrobbling_enabled_ = settings.value("ScrobblingEnabled", true).toBool();
  buttons_visible_ = settings.value("ShowLoveBanButtons", true).toBool();

  emit ScrobblingEnabledChanged(scrobbling_enabled_);
  emit ButtonVisibilityChanged(buttons_visible_);
}

void LastFMService::ShowConfig() {
  emit OpenSettingsAtPage(SettingsDialog::Page_Lastfm);
}

bool LastFMService::IsAuthenticated() const {
  return !lastfm::ws::SessionKey.isEmpty();
}

QStandardItem* LastFMService::CreateRootItem() {
  QStandardItem* item = new QStandardItem(QIcon(":last.fm/as.png"), kServiceName);
  item->setData(true, RadioModel::Role_CanLazyLoad);
  return item;
}

void LastFMService::LazyPopulate(QStandardItem* parent) {
  switch (parent->data(RadioModel::Role_Type).toInt()) {
    case RadioModel::Type_Service:
      // Normal radio types
      CreateStationItem(parent,
          tr("My Recommendations"),
          ":last.fm/recommended_radio.png",
          "lastfm://user/USERNAME/recommended",
          tr("My Last.fm Recommended Radio"));
      CreateStationItem(parent,
          tr("My Radio Station"),
          ":last.fm/personal_radio.png",
          "lastfm://user/USERNAME/library",
          tr("My Last.fm Library"));
      CreateStationItem(parent,
          tr("My Mix Radio"),
          ":last.fm/loved_radio.png",
          "lastfm://user/USERNAME/mix",
          tr("My Last.fm Mix Radio"));
      CreateStationItem(parent,
          tr("My Neighborhood"),
          ":last.fm/neighbour_radio.png",
          "lastfm://user/USERNAME/neighbours",
          tr("My Last.fm Neighborhood"));

      // Types that have children
      artist_list_ = new QStandardItem(QIcon(":last.fm/icon_radio.png"), tr("Artist radio"));
      artist_list_->setData(Type_Artists, RadioModel::Role_Type);
      parent->appendRow(artist_list_);

      tag_list_ = new QStandardItem(QIcon(":last.fm/icon_tag.png"), tr("Tag radio"));
      tag_list_->setData(Type_Tags, RadioModel::Role_Type);
      parent->appendRow(tag_list_);

      custom_list_ = new QStandardItem(QIcon(":last.fm/icon_radio.png"), tr("Custom radio"));
      custom_list_->setData(Type_Custom, RadioModel::Role_Type);
      parent->appendRow(custom_list_);

      RestoreList("artists", kUrlArtist, kTitleArtist, QIcon(":last.fm/icon_radio.png"), artist_list_);
      RestoreList("tags", kUrlTag, kTitleTag, QIcon(":last.fm/icon_tag.png"), tag_list_);
      RestoreList("custom", kUrlCustom, kTitleCustom, QIcon(":last.fm/icon_radio.png"), custom_list_);

      friends_list_ = new QStandardItem(QIcon(":last.fm/my_friends.png"), tr("Friends"));
      friends_list_->setData(Type_Friends, RadioModel::Role_Type);
      friends_list_->setData(true, RadioModel::Role_CanLazyLoad);
      parent->appendRow(friends_list_);

      neighbours_list_ = new QStandardItem(QIcon(":last.fm/my_neighbours.png"), tr("Neighbors"));
      neighbours_list_->setData(Type_Neighbours, RadioModel::Role_Type);
      neighbours_list_->setData(true, RadioModel::Role_CanLazyLoad);
      parent->appendRow(neighbours_list_);

      if (!IsAuthenticated())
        ShowConfig();

      add_artist_action_->setEnabled(true);
      add_tag_action_->setEnabled(true);
      add_custom_action_->setEnabled(true);
      break;

    case Type_Friends:
      RefreshFriends();
      break;

    case Type_Neighbours:
      RefreshNeighbours();
      break;

    case Type_OtherUser:
      CreateStationItem(parent,
          tr("Last.fm Radio Station - %1").arg(parent->text()),
          ":last.fm/personal_radio.png",
          "lastfm://user/" + parent->text() + "/library",
          tr("Last.fm Library - %1").arg(parent->text()));
      CreateStationItem(parent,
          tr("Last.fm Mix Radio - %1").arg(parent->text()),
          ":last.fm/loved_radio.png",
          "lastfm://user/" + parent->text() + "/mix",
          tr("Last.fm Mix Radio - %1").arg(parent->text()));
      CreateStationItem(parent,
          tr("Last.fm Neighbor Radio - %1").arg(parent->text()),
          ":last.fm/neighbour_radio.png",
          "lastfm://user/" + parent->text() + "/neighbours",
          tr("Last.fm Neighbor Radio - %1").arg(parent->text()));
      break;

    default:
      break;
  }
}

QStandardItem* LastFMService::CreateStationItem(
    QStandardItem* parent, const QString& name, const QString& icon,
    const QString& url, const QString& title) {
  QStandardItem* ret = new QStandardItem(QIcon(icon), name);
  ret->setData(url, RadioModel::Role_Url);
  ret->setData(title, RadioModel::Role_Title);
  ret->setData(RadioModel::PlayBehaviour_SingleItem, RadioModel::Role_PlayBehaviour);
  parent->appendRow(ret);
  return ret;
}

void LastFMService::Authenticate(const QString& username, const QString& password) {
  QMap<QString, QString> params;
  params["method"] = "auth.getMobileSession";
  params["username"] = username;
  params["authToken"] = lastfm::md5((username + lastfm::md5(password.toUtf8())).toUtf8());

  QNetworkReply* reply = lastfm::ws::post(params);
  connect(reply, SIGNAL(finished()), SLOT(AuthenticateReplyFinished()));
}

void LastFMService::SignOut() {
  lastfm::ws::Username.clear();
  lastfm::ws::SessionKey.clear();

  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("Username", QString());
  settings.setValue("Session", QString());
}

void LastFMService::AuthenticateReplyFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    emit AuthenticationComplete(false);
    return;
  }

  // Parse the reply
  try {
    lastfm::XmlQuery const lfm = lastfm::ws::parse(reply);
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError)
      throw std::runtime_error("");
#endif

    lastfm::ws::Username = lfm["session"]["name"].text();
    lastfm::ws::SessionKey = lfm["session"]["key"].text();
  } catch (std::runtime_error& e) {
    qDebug() << e.what();
    emit AuthenticationComplete(false);
    return;
  }

  // Save the session key
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("Username", lastfm::ws::Username);
  settings.setValue("Session", lastfm::ws::SessionKey);

  // Invalidate the scrobbler - it will get recreated later
  delete scrobbler_;
  scrobbler_ = NULL;

  emit AuthenticationComplete(true);
}

QUrl LastFMService::FixupUrl(const QUrl& url) {
  QUrl ret;
  ret.setEncodedUrl(url.toEncoded().replace(
      "USERNAME", QUrl::toPercentEncoding(lastfm::ws::Username)));
  return ret;
}

PlaylistItem::SpecialLoadResult LastFMService::StartLoading(const QUrl& url) {
  if (url.scheme() != "lastfm")
    return PlaylistItem::SpecialLoadResult();
  if (!IsAuthenticated())
    return PlaylistItem::SpecialLoadResult();

  if (!tune_task_id_)
    tune_task_id_ = model()->task_manager()->StartTask(tr("Loading Last.fm radio"));

  last_url_ = url;
  initial_tune_ = true;
  Tune(lastfm::RadioStation(FixupUrl(url)));

  return PlaylistItem::SpecialLoadResult(
      PlaylistItem::SpecialLoadResult::WillLoadAsynchronously, url);
}

PlaylistItem::SpecialLoadResult LastFMService::LoadNext(const QUrl&) {
  if (playlist_.empty()) {
    return PlaylistItem::SpecialLoadResult();
  }

  lastfm::MutableTrack track = playlist_.dequeue();
  track.stamp();

  last_track_ = track;
  if (playlist_.empty()) {
    FetchMoreTracks();
  }

  next_metadata_ = track;
  StreamMetadataReady();

  return PlaylistItem::SpecialLoadResult(
      PlaylistItem::SpecialLoadResult::TrackAvailable, last_url_, last_track_.url());
}

void LastFMService::StreamMetadataReady() {
  Song metadata;
  metadata.InitFromLastFM(next_metadata_);

  if (art_urls_.contains(next_metadata_))
    metadata.set_art_automatic(art_urls_[next_metadata_]);

  emit StreamMetadataFound(last_url_, metadata);
}

void LastFMService::TunerError(lastfm::ws::Error error) {
  qDebug() << "Last.fm error" << error;
  if (!initial_tune_)
    return;

  model()->task_manager()->SetTaskFinished(tune_task_id_);
  tune_task_id_ = 0;

  if (error == lastfm::ws::NotEnoughContent) {
    emit AsyncLoadFinished(PlaylistItem::SpecialLoadResult(
        PlaylistItem::SpecialLoadResult::NoMoreTracks, last_url_));
    return;
  }

  emit StreamError(ErrorString(error));
}

QString LastFMService::ErrorString(lastfm::ws::Error error) const {
  switch (error) {
    case lastfm::ws::InvalidService: return tr("Invalid service");
    case lastfm::ws::InvalidMethod: return tr("Invalid method");
    case lastfm::ws::AuthenticationFailed: return tr("Authentication failed");
    case lastfm::ws::InvalidFormat: return tr("Invalid format");
    case lastfm::ws::InvalidParameters: return tr("Invalid parameters");
    case lastfm::ws::InvalidResourceSpecified: return tr("Invalid resource specified");
    case lastfm::ws::OperationFailed: return tr("Operation failed");
    case lastfm::ws::InvalidSessionKey: return tr("Invalid session key");
    case lastfm::ws::InvalidApiKey: return tr("Invalid API key");
    case lastfm::ws::ServiceOffline: return tr("Service offline");
    case lastfm::ws::SubscribersOnly: return tr("This stream is for paid subscribers only");

    case lastfm::ws::TryAgainLater: return tr("Last.fm is currently busy, please try again in a few minutes");

    case lastfm::ws::NotEnoughContent: return tr("Not enough content");
    case lastfm::ws::NotEnoughMembers: return tr("Not enough members");
    case lastfm::ws::NotEnoughFans: return tr("Not enough fans");
    case lastfm::ws::NotEnoughNeighbours: return tr("Not enough neighbors");

    case lastfm::ws::MalformedResponse: return tr("Malformed response");

    case lastfm::ws::UnknownError:
    default:
      return tr("Unknown error");
  }
}

void LastFMService::TunerTrackAvailable() {
  if (initial_tune_) {
    emit AsyncLoadFinished(LoadNext(last_url_));
    initial_tune_ = false;
  }
}

bool LastFMService::InitScrobbler() {
  if (!IsAuthenticated() || !IsScrobblingEnabled())
    return false;

  if (!scrobbler_)
    scrobbler_ = new lastfm::Audioscrobbler(kAudioscrobblerClientId);

  return true;
}

lastfm::Track LastFMService::TrackFromSong(const Song &song) const {
  if (song.title() == last_track_.title() &&
      song.artist() == last_track_.artist() &&
      song.album() == last_track_.album())
    return last_track_;

  lastfm::Track ret;
  song.ToLastFM(&ret);
  return ret;

}

void LastFMService::NowPlaying(const Song &song) {
  if (!InitScrobbler())
    return;

  last_track_ = TrackFromSong(song);

  lastfm::MutableTrack mtrack(last_track_);
  mtrack.stamp();
  last_track_ = mtrack;

  scrobbler_->nowPlaying(mtrack);
}

void LastFMService::Scrobble() {
  if (!InitScrobbler())
    return;

  scrobbler_->cache(last_track_);
  scrobbler_->submit();
}

void LastFMService::Love() {
  if (!IsAuthenticated())
    ShowConfig();

  lastfm::MutableTrack mtrack(last_track_);
  mtrack.love();
  last_track_ = mtrack;
}

void LastFMService::Ban() {
  lastfm::MutableTrack mtrack(last_track_);
  mtrack.ban();
  last_track_ = mtrack;

  Scrobble();
  emit AsyncLoadFinished(LoadNext(last_url_));
}

void LastFMService::ShowContextMenu(const QModelIndex& index, const QPoint &global_pos) {
  context_item_ = model()->itemFromIndex(index);

  switch (index.parent().data(RadioModel::Role_Type).toInt()) {
    case Type_Artists:
    case Type_Tags:
    case Type_Custom:
      remove_action_->setEnabled(true);
      break;

    default:
      remove_action_->setEnabled(false);
      break;
  }

  const bool playable = model()->IsPlayable(index);
  play_action_->setEnabled(playable);
  load_action_->setEnabled(playable);
  context_menu_->popup(global_pos);
}

void LastFMService::RefreshFriends() {
  if (!friends_list_ || !IsAuthenticated())
    return;

  if (friends_list_->hasChildren())
    friends_list_->removeRows(0, friends_list_->rowCount());

  lastfm::AuthenticatedUser user;
  QNetworkReply* reply = user.getFriends();
  connect(reply, SIGNAL(finished()), SLOT(RefreshFriendsFinished()));
}

void LastFMService::RefreshNeighbours() {
  if (!neighbours_list_ || !IsAuthenticated())
    return;

  if (neighbours_list_->hasChildren())
    neighbours_list_->removeRows(0, neighbours_list_->rowCount());

  lastfm::AuthenticatedUser user;
  QNetworkReply* reply = user.getNeighbours();
  connect(reply, SIGNAL(finished()), SLOT(RefreshNeighboursFinished()));
}

void LastFMService::RefreshFriendsFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  QList<lastfm::User> friends;

  try {
    friends = lastfm::User::list(reply);
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError)
      throw std::runtime_error("");
#endif
  } catch (std::runtime_error& e) {
    qDebug() << e.what();
    return;
  }

  foreach (const lastfm::User& f, friends) {
    QStandardItem* item = new QStandardItem(QIcon(":last.fm/icon_user.png"), f.name());
    item->setData(QUrl("lastfm://user/" + f.name() + "/library"), RadioModel::Role_Url);
    item->setData(tr("Last.fm Library - %1").arg(f.name()), RadioModel::Role_Title);
    item->setData(true, RadioModel::Role_CanLazyLoad);
    item->setData(Type_OtherUser, RadioModel::Role_Type);
    item->setData(RadioModel::PlayBehaviour_SingleItem, RadioModel::Role_PlayBehaviour);
    friends_list_->appendRow(item);
  }
}

void LastFMService::RefreshNeighboursFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  QList<lastfm::User> neighbours;

  try {
    neighbours = lastfm::User::list(reply);
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError)
      throw std::runtime_error("");
#endif
  } catch (std::runtime_error& e) {
    qDebug() << e.what();
    return;
  }

  foreach (const lastfm::User& n, neighbours) {
    QStandardItem* item = new QStandardItem(QIcon(":last.fm/user_purple.png"), n.name());
    item->setData(QUrl("lastfm://user/" + n.name() + "/library"), RadioModel::Role_Url);
    item->setData(tr("Last.fm Library - %1").arg(n.name()), RadioModel::Role_Title);
    item->setData(true, RadioModel::Role_CanLazyLoad);
    item->setData(Type_OtherUser, RadioModel::Role_Type);
    item->setData(RadioModel::PlayBehaviour_SingleItem, RadioModel::Role_PlayBehaviour);
    neighbours_list_->appendRow(item);
  }
}

void LastFMService::AddToPlaylist() {
  emit AddItemToPlaylist(context_item_, false);
}

void LastFMService::LoadToPlaylist() {
  emit AddItemToPlaylist(context_item_, true);
}

void LastFMService::AddArtistRadio() {
  AddArtistOrTag("artists", LastFMStationDialog::Artist,
                 kUrlArtist, tr(kTitleArtist),
                 ":last.fm/icon_radio.png", artist_list_);
}

void LastFMService::AddTagRadio() {
  AddArtistOrTag("tags", LastFMStationDialog::Tag,
                 kUrlTag, tr(kTitleTag),
                 ":last.fm/icon_tag.png", tag_list_);
}

void LastFMService::AddCustomRadio() {
  AddArtistOrTag("custom", LastFMStationDialog::Custom,
                 kUrlCustom, tr(kTitleCustom),
                 ":last.fm/icon_radio.png", custom_list_);
}

void LastFMService::AddArtistOrTag(const QString& name,
                                   LastFMStationDialog::Type dialog_type,
                                   const QString& url_pattern,
                                   const QString& title_pattern,
                                   const QString& icon, QStandardItem* list) {
  station_dialog_->SetType(dialog_type);
  if (station_dialog_->exec() == QDialog::Rejected)
    return;

  if (station_dialog_->content().isEmpty())
    return;

  QString content = station_dialog_->content();
  QString url_content;
  if (name == "custom")
    url_content = content.toUtf8().toBase64();
  else
    url_content = content;

  QStandardItem* item = new QStandardItem(QIcon(icon), content);
  item->setData(url_pattern.arg(url_content), RadioModel::Role_Url);
  item->setData(title_pattern.arg(content), RadioModel::Role_Title);
  item->setData(RadioModel::PlayBehaviour_SingleItem, RadioModel::Role_PlayBehaviour);
  list->appendRow(item);
  emit AddItemToPlaylist(item, false);

  SaveList(name, list);
}

void LastFMService::SaveList(const QString& name, QStandardItem* list) const {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  settings.beginWriteArray(name, list->rowCount());
  for (int i=0 ; i<list->rowCount() ; ++i) {
    settings.setArrayIndex(i);
    settings.setValue("key", list->child(i)->text());
  }
  settings.endArray();
}

void LastFMService::RestoreList(const QString& name,
                                const QString& url_pattern,
                                const QString& title_pattern,
                                const QIcon& icon, QStandardItem* parent) {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  if (parent->hasChildren())
    parent->removeRows(0, parent->rowCount());

  int count = settings.beginReadArray(name);
  for (int i=0 ; i<count ; ++i) {
    settings.setArrayIndex(i);
    QString content = settings.value("key").toString();
    QString url_content;
    if (name == "custom")
      url_content = content.toUtf8().toBase64();
    else
      url_content = content;

    QStandardItem* item = new QStandardItem(icon, content);
    item->setData(url_pattern.arg(url_content), RadioModel::Role_Url);
    item->setData(title_pattern.arg(content), RadioModel::Role_Title);
    item->setData(RadioModel::PlayBehaviour_SingleItem, RadioModel::Role_PlayBehaviour);
    parent->appendRow(item);
  }
  settings.endArray();
}

void LastFMService::Remove() {
  int type = context_item_->parent()->data(RadioModel::Role_Type).toInt();

  context_item_->parent()->removeRow(context_item_->row());

  if (type == Type_Artists)
    SaveList("artists", artist_list_);
  else if (type == Type_Tags)
    SaveList("tags", tag_list_);
  else if (type == Type_Custom)
    SaveList("custom", custom_list_);
}

void LastFMService::FetchMoreTracks() {
  QMap<QString, QString> params;
  params["method"] = "radio.getPlaylist";
  params["rtp"] = "1";
  QNetworkReply* reply = lastfm::ws::post(params);
  connect(reply, SIGNAL(finished()), SLOT(FetchMoreTracksFinished()));
}

void LastFMService::FetchMoreTracksFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    qWarning() << "Invalid reply on radio.getPlaylist";
    emit AsyncLoadFinished(PlaylistItem::SpecialLoadResult(
        PlaylistItem::SpecialLoadResult::NoMoreTracks, reply->url()));
    return;
  }
  reply->deleteLater();
  model()->task_manager()->SetTaskFinished(tune_task_id_);
  tune_task_id_ = 0;

  try {
    const XmlQuery& query = lastfm::ws::parse(reply);
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError)
      throw std::runtime_error("");
#endif

    const XmlQuery& playlist = query["playlist"];
    foreach (const XmlQuery& q, playlist["trackList"].children("track")) {
      lastfm::MutableTrack t;
      t.setUrl(q["location"].text());
      t.setExtra("trackauth", q["extension"]["trackauth"].text());
      t.setTitle(q["title"].text());
      t.setArtist(q["creator"].text());
      t.setAlbum(q["album"].text());
      t.setDuration(q["duration"].text().toInt() / 1000);
      t.setSource(lastfm::Track::LastFmRadio);

      art_urls_[t] = q["image"].text();
      playlist_ << t;
    }
  } catch (std::runtime_error& e) {
    // For some reason a catch block that takes a lastfm::ws::ParseError&
    // doesn't get called, even when a lastfm::ws::ParseError is thrown...
    // Hacks like this remind me of Java...
    if (QString(typeid(e).name()).contains("ParseError")) {
      // dynamic_cast throws a std::bad_cast ... *boggle*
      emit StreamError(tr("Couldn't load the last.fm radio station")
                       .arg(e.what()));
    } else {
      emit StreamError(tr("An unknown last.fm error occurred: %1").arg(e.what()));
    }
    return;
  }

  TunerTrackAvailable();
}

void LastFMService::Tune(const lastfm::RadioStation& station) {
  playlist_.clear();

  // Remove all the old album art URLs
  art_urls_.clear();

  QMap<QString, QString> params;
  params["method"] = "radio.tune";
  params["station"] = station.url();
  QNetworkReply* reply = lastfm::ws::post(params);
  connect(reply, SIGNAL(finished()), SLOT(TuneFinished()));
}

void LastFMService::TuneFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    qWarning() << "Invalid reply on radio.tune";
    emit AsyncLoadFinished(PlaylistItem::SpecialLoadResult(
        PlaylistItem::SpecialLoadResult::NoMoreTracks, reply->url()));
    return;
  }

  FetchMoreTracks();
  reply->deleteLater();
}

PlaylistItem::Options LastFMService::playlistitem_options() const {
  return PlaylistItem::SpecialPlayBehaviour |
         PlaylistItem::LastFMControls |
         PlaylistItem::PauseDisabled |
         PlaylistItem::ContainsMultipleTracks;
}

PlaylistItemPtr LastFMService::PlaylistItemForUrl(const QUrl& url) {
  // This is a bit of a hack, it's only used by the artist/song info tag
  // widgets for tag radio and similar artists radio.

  PlaylistItemPtr ret;

  if (url.scheme() != "lastfm")
    return ret;

  QStringList sections(url.path().split("/", QString::SkipEmptyParts));

  if (sections.count() == 2 && url.host() == "artist" && sections[1] == "similarartists") {
    ret.reset(new RadioPlaylistItem(this, url,
        tr("Last.fm Similar Artists to %1").arg(sections[0]), QString()));
  } else if (sections.count() == 1 && url.host() == "globaltags") {
    ret.reset(new RadioPlaylistItem(this, url,
        tr("Last.fm Tag Radio: %1").arg(sections[0]), QString()));
  }

  return ret;
}
