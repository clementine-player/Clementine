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
#include "lastfmurlhandler.h"
#include "internetmodel.h"
#include "internetplaylistitem.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "covers/coverproviders.h"
#include "covers/lastfmcoverprovider.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"

#include <boost/scoped_ptr.hpp>

#include <lastfm/Audioscrobbler>
#include <lastfm/misc.h>
#include <lastfm/RadioStation>
#include <lastfm/Scrobble>
#include <lastfm/ScrobbleCache>
#include <lastfm/ScrobblePoint>
#include <lastfm/ws.h>
#include <lastfm/XmlQuery>

#include <QMenu>
#include <QSettings>

using boost::scoped_ptr;
using lastfm::Scrobble;
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

LastFMService::LastFMService(InternetModel* parent)
  : InternetService(kServiceName, parent, parent),
    url_handler_(new LastFMUrlHandler(this, this)),
    scrobbler_(NULL),
    already_scrobbled_(false),
    station_dialog_(new LastFMStationDialog),
    context_menu_(new QMenu),
    initial_tune_(false),
    tune_task_id_(0),
    scrobbling_enabled_(false),
    artist_list_(NULL),
    tag_list_(NULL),
    custom_list_(NULL),
    friends_list_(NULL),
    neighbours_list_(NULL),
    connection_problems_(false)
{
  //we emit the signal the first time to be sure the buttons are in the right state
  emit ScrobblingEnabledChanged(scrobbling_enabled_);

  context_menu_->addActions(GetPlaylistActions());
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

  model()->player()->RegisterUrlHandler(url_handler_);
  model()->cover_providers()->AddProvider(new LastFmCoverProvider(this));
}

LastFMService::~LastFMService() {
}

void LastFMService::ReloadSettings() {
  bool scrobbling_enabled_old = scrobbling_enabled_;
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  lastfm::ws::Username = settings.value("Username").toString();
  lastfm::ws::SessionKey = settings.value("Session").toString();
  scrobbling_enabled_ = settings.value("ScrobblingEnabled", true).toBool();
  buttons_visible_ = settings.value("ShowLoveBanButtons", true).toBool();
  scrobble_button_visible_ = settings.value("ShowScrobbleButton", true).toBool();

  //avoid emitting signal if it's not changed
  if(scrobbling_enabled_old != scrobbling_enabled_)
    emit ScrobblingEnabledChanged(scrobbling_enabled_);
  emit ButtonVisibilityChanged(buttons_visible_);
  emit ScrobbleButtonVisibilityChanged(scrobble_button_visible_);
}

void LastFMService::ShowConfig() {
  emit OpenSettingsAtPage(SettingsDialog::Page_Lastfm);
}

bool LastFMService::IsAuthenticated() const {
  return !lastfm::ws::SessionKey.isEmpty();
}

bool LastFMService::IsSubscriber() const {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  return settings.value("Subscriber", false).toBool();
}

QStandardItem* LastFMService::CreateRootItem() {
  QStandardItem* item = new QStandardItem(QIcon(":last.fm/as.png"), kServiceName);
  item->setData(true, InternetModel::Role_CanLazyLoad);
  return item;
}

void LastFMService::LazyPopulate(QStandardItem* parent) {
  switch (parent->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      // Normal radio types
      CreateStationItem(parent,
          tr("My Recommendations"),
          ":last.fm/recommended_radio.png",
          QUrl("lastfm://user/USERNAME/recommended"),
          tr("My Last.fm Recommended Radio"));
      CreateStationItem(parent,
          tr("My Radio Station"),
          ":last.fm/personal_radio.png",
          QUrl("lastfm://user/USERNAME/library"),
          tr("My Last.fm Library"));
      CreateStationItem(parent,
          tr("My Mix Radio"),
          ":last.fm/loved_radio.png",
          QUrl("lastfm://user/USERNAME/mix"),
          tr("My Last.fm Mix Radio"));
      CreateStationItem(parent,
          tr("My Neighborhood"),
          ":last.fm/neighbour_radio.png",
          QUrl("lastfm://user/USERNAME/neighbours"),
          tr("My Last.fm Neighborhood"));

      // Types that have children
      artist_list_ = new QStandardItem(QIcon(":last.fm/icon_radio.png"), tr("Artist radio"));
      artist_list_->setData(Type_Artists, InternetModel::Role_Type);
      parent->appendRow(artist_list_);

      tag_list_ = new QStandardItem(QIcon(":last.fm/icon_tag.png"), tr("Tag radio"));
      tag_list_->setData(Type_Tags, InternetModel::Role_Type);
      parent->appendRow(tag_list_);

      custom_list_ = new QStandardItem(QIcon(":last.fm/icon_radio.png"), tr("Custom radio"));
      custom_list_->setData(Type_Custom, InternetModel::Role_Type);
      parent->appendRow(custom_list_);

      RestoreList("artists", kUrlArtist, tr(kTitleArtist), QIcon(":last.fm/icon_radio.png"), artist_list_);
      RestoreList("tags", kUrlTag, tr(kTitleTag), QIcon(":last.fm/icon_tag.png"), tag_list_);
      RestoreList("custom", kUrlCustom, tr(kTitleCustom), QIcon(":last.fm/icon_radio.png"), custom_list_);

      friends_list_ = new QStandardItem(QIcon(":last.fm/my_friends.png"), tr("Friends"));
      friends_list_->setData(Type_Friends, InternetModel::Role_Type);
      friends_list_->setData(true, InternetModel::Role_CanLazyLoad);
      parent->appendRow(friends_list_);

      neighbours_list_ = new QStandardItem(QIcon(":last.fm/my_neighbours.png"), tr("Neighbors"));
      neighbours_list_->setData(Type_Neighbours, InternetModel::Role_Type);
      neighbours_list_->setData(true, InternetModel::Role_CanLazyLoad);
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
          QUrl("lastfm://user/" + parent->text() + "/library"),
          tr("Last.fm Library - %1").arg(parent->text()));
      CreateStationItem(parent,
          tr("Last.fm Mix Radio - %1").arg(parent->text()),
          ":last.fm/loved_radio.png",
          QUrl("lastfm://user/" + parent->text() + "/mix"),
          tr("Last.fm Mix Radio - %1").arg(parent->text()));
      CreateStationItem(parent,
          tr("Last.fm Neighbor Radio - %1").arg(parent->text()),
          ":last.fm/neighbour_radio.png",
          QUrl("lastfm://user/" + parent->text() + "/neighbours"),
          tr("Last.fm Neighbor Radio - %1").arg(parent->text()));
      break;

    default:
      break;
  }
}

QStandardItem* LastFMService::CreateStationItem(
    QStandardItem* parent, const QString& name, const QString& icon,
    const QUrl& url, const QString& title) {
  Song song;
  song.set_url(url);
  song.set_title(title);

  QStandardItem* ret = new QStandardItem(QIcon(icon), name);
  ret->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
  ret->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
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
  // If we need more detailed error reporting, handle error(NetworkError) signal
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
  reply->deleteLater();

  // Parse the reply
  try {
    lastfm::XmlQuery const lfm = lastfm::ws::parse(reply);
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError)
      throw std::runtime_error("");
#endif

    lastfm::ws::Username = lfm["session"]["name"].text();
    lastfm::ws::SessionKey = lfm["session"]["key"].text();
    QString subscribed = lfm["session"]["subscriber"].text();
    const bool is_subscriber = (subscribed.toInt() == 1);

    // Save the session key
    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    settings.setValue("Username", lastfm::ws::Username);
    settings.setValue("Session", lastfm::ws::SessionKey);
    settings.setValue("Subscriber", is_subscriber);
  } catch (std::runtime_error& e) {
    qLog(Error) << e.what();
    emit AuthenticationComplete(false);
    return;
  }

  // Invalidate the scrobbler - it will get recreated later
  delete scrobbler_;
  scrobbler_ = NULL;

  emit AuthenticationComplete(true);
}

void LastFMService::UpdateSubscriberStatus() {
  QMap<QString, QString> params;
  params["method"] = "user.getInfo";
  params["user"] = lastfm::ws::Username;

  QNetworkReply* reply = lastfm::ws::post(params);
  connect(reply, SIGNAL(finished()), SLOT(UpdateSubscriberStatusFinished()));
}

void LastFMService::UpdateSubscriberStatusFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);
  reply->deleteLater();

  bool is_subscriber = false;

  try {
    const lastfm::XmlQuery lfm = lastfm::ws::parse(reply);
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError)
      throw std::runtime_error("");
#endif

    connection_problems_ = false;
    QString subscriber = lfm["user"]["subscriber"].text();
    is_subscriber = (subscriber.toInt() == 1);

    QSettings settings;
    settings.beginGroup(kSettingsGroup);
    settings.setValue("Subscriber", is_subscriber);
    qLog(Info) << lastfm::ws::Username << "Subscriber status:" << is_subscriber;
  } catch (lastfm::ws::ParseError e) {
    qLog(Error) << "Last.fm parse error: " << e.enumValue();
    connection_problems_ = e.enumValue() == lastfm::ws::MalformedResponse;
  } catch (std::runtime_error& e) {
    qLog(Error) << e.what();
  }

  emit UpdatedSubscriberStatus(is_subscriber);
}

QUrl LastFMService::FixupUrl(const QUrl& url) {
  QUrl ret;
  ret.setEncodedUrl(url.toEncoded().replace(
      "USERNAME", QUrl::toPercentEncoding(lastfm::ws::Username)));
  return ret;
}

QUrl LastFMService::DeququeNextMediaUrl() {
  if (playlist_.empty()) {
    return QUrl();
  }

  lastfm::MutableTrack track = playlist_.dequeue();
  track.stamp();

  already_scrobbled_ = false;
  last_track_ = track;
  if (playlist_.empty()) {
    FetchMoreTracks();
  }

  next_metadata_ = track;
  StreamMetadataReady();

  return last_track_.url();
}

void LastFMService::StreamMetadataReady() {
  Song metadata;
  metadata.InitFromLastFM(next_metadata_);

  if (art_urls_.contains(next_metadata_))
    metadata.set_art_automatic(art_urls_[next_metadata_]);

  emit StreamMetadataFound(last_url_, metadata);
}

void LastFMService::TunerError(lastfm::ws::Error error) {
  qLog(Warning) << "Last.fm error" << error;
  if (!initial_tune_)
    return;

  model()->task_manager()->SetTaskFinished(tune_task_id_);
  tune_task_id_ = 0;

  if (error == lastfm::ws::NotEnoughContent) {
    url_handler_->TunerError();
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
    url_handler_->TunerTrackAvailable();
    initial_tune_ = false;
  }
}

bool LastFMService::InitScrobbler() {
  if (!IsAuthenticated() || !IsScrobblingEnabled())
    return false;

  if (!scrobbler_)
    scrobbler_ = new lastfm::Audioscrobbler(kAudioscrobblerClientId);

  //reemit the signal since the sender is private
  connect(scrobbler_, SIGNAL(status(int)), SIGNAL(ScrobblerStatus(int)));
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

  // Scrobbling streams is difficult if we don't have length of each individual
  // part.  In Song::ToLastFm we set the Track's source to
  // NonPersonalisedBroadcast if it's such a stream, so we have to scrobble it
  // when we change to a different track, but only if enough time has elapsed
  // since it started playing.
  if (!last_track_.isNull() &&
      last_track_.source() == lastfm::Track::NonPersonalisedBroadcast) {
    const int duration_secs = last_track_.timestamp().secsTo(QDateTime::currentDateTime());
    if (duration_secs >= ScrobblePoint::kScrobbleMinLength) {
      lastfm::MutableTrack mtrack(last_track_);
      mtrack.setDuration(duration_secs);

      qLog(Info) << "Scrobbling stream track" << mtrack.title() << "length" << duration_secs;
      scrobbler_->cache(mtrack);
      scrobbler_->submit();

      emit ScrobbledRadioStream();
    }
  }

  lastfm::MutableTrack mtrack(TrackFromSong(song));
  mtrack.stamp();
  already_scrobbled_ = false;
  last_track_ = mtrack;

  //check immediately if the song is valid
  Scrobble::Invalidity invalidity;

  if (!lastfm::Scrobble(last_track_).isValid( &invalidity )) {
    //for now just notify this, we can also see the cause
    emit ScrobblerStatus(-1);
    return;
  }

  scrobbler_->nowPlaying(mtrack);
}

void LastFMService::Scrobble() {
  if (!InitScrobbler())
    return;

  ScrobbleCache cache(lastfm::ws::Username);
  qLog(Debug) << "There are" << cache.tracks().count() << "tracks in the last.fm cache.";
  scrobbler_->cache(last_track_);

  // Let's mark a track as cached, useful when the connection is down
  emit ScrobblerStatus(30);
  scrobbler_->submit();

  already_scrobbled_ = true;
}

void LastFMService::Love() {
  if (!IsAuthenticated())
    ShowConfig();

  lastfm::MutableTrack mtrack(last_track_);
  mtrack.love();
  last_track_ = mtrack;

  if (already_scrobbled_) {
    // The love only takes effect when the song is scrobbled, but we've already
    // scrobbled this one so we have to do it again.
    Scrobble();
  }
}

void LastFMService::Ban() {
  lastfm::MutableTrack mtrack(last_track_);
  mtrack.ban();
  last_track_ = mtrack;

  Scrobble();
  model()->player()->Next();
}

void LastFMService::ShowContextMenu(const QModelIndex& index, const QPoint &global_pos) {
  context_item_ = model()->itemFromIndex(index);

  switch (index.parent().data(InternetModel::Role_Type).toInt()) {
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
  GetAppendToPlaylistAction()->setEnabled(playable);
  GetReplacePlaylistAction()->setEnabled(playable);
  GetOpenInNewPlaylistAction()->setEnabled(playable);
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
    qLog(Error) << e.what();
    return;
  }

  foreach (const lastfm::User& f, friends) {
    Song song;
    song.set_url(QUrl("lastfm://user/" + f.name() + "/library"));
    song.set_title(tr("Last.fm Library - %1").arg(f.name()));

    QStandardItem* item = new QStandardItem(QIcon(":last.fm/icon_user.png"), f.name());
    item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
    item->setData(true, InternetModel::Role_CanLazyLoad);
    item->setData(Type_OtherUser, InternetModel::Role_Type);
    item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
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
    qLog(Error) << e.what();
    return;
  }

  foreach (const lastfm::User& n, neighbours) {
    Song song;
    song.set_url(QUrl("lastfm://user/" + n.name() + "/library"));
    song.set_title(tr("Last.fm Library - %1").arg(n.name()));

    QStandardItem* item = new QStandardItem(QIcon(":last.fm/user_purple.png"), n.name());
    item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
    item->setData(true, InternetModel::Role_CanLazyLoad);
    item->setData(Type_OtherUser, InternetModel::Role_Type);
    item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
    neighbours_list_->appendRow(item);
  }
}

QModelIndex LastFMService::GetCurrentIndex() {
  return context_item_->index();
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
  QString url;
  if (name == "custom" && content.startsWith("lastfm://")) {
    url = content;
  } else if (name == "custom") {
    url = url_pattern.arg(QString(content.toUtf8().toBase64()));
  } else {
    url = url_pattern.arg(content);
  }

  Song song;
  song.set_url(QUrl((url)));
  song.set_title(title_pattern.arg(content));

  QStandardItem* item = new QStandardItem(QIcon(icon), content);
  item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
  item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
  list->appendRow(item);
  emit AddItemToPlaylist(item->index(), AddMode_Append);

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
    QString url;
    if (name == "custom" && content.startsWith("lastfm://")) {
      url = content;
    } else if (name == "custom") {
      url = url_pattern.arg(QString(content.toUtf8().toBase64()));
    } else {
      url = url_pattern.arg(content);
    }

    Song song;
    song.set_url(QUrl(url));
    song.set_title(title_pattern.arg(content));

    QStandardItem* item = new QStandardItem(icon, content);
    item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
    item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
    parent->appendRow(item);
  }
  settings.endArray();
}

void LastFMService::Remove() {
  int type = context_item_->parent()->data(InternetModel::Role_Type).toInt();

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
    qLog(Warning) << "Invalid reply on radio.getPlaylist";
    url_handler_->TunerError();
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

void LastFMService::Tune(const QUrl& url) {
  if (!tune_task_id_)
    tune_task_id_ = model()->task_manager()->StartTask(tr("Loading Last.fm radio"));

  last_url_ = url;
  initial_tune_ = true;
  const lastfm::RadioStation station(FixupUrl(url));

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
    qLog(Warning) << "Invalid reply on radio.tune";
    url_handler_->TunerError();
    return;
  }

  FetchMoreTracks();
  reply->deleteLater();
}

PlaylistItem::Options LastFMService::playlistitem_options() const {
  return PlaylistItem::LastFMControls |
         PlaylistItem::PauseDisabled;
}

PlaylistItemPtr LastFMService::PlaylistItemForUrl(const QUrl& url) {
  // This is a bit of a hack, it's only used by the artist/song info tag
  // widgets for tag radio and similar artists radio.

  if (url.scheme() != "lastfm")
    return PlaylistItemPtr();

  QStringList sections(url.path().split("/", QString::SkipEmptyParts));

  Song song;
  song.set_url(url);

  if (sections.count() == 2 && url.host() == "artist" && sections[1] == "similarartists") {
    song.set_title(tr(kTitleArtist).arg(sections[0]));
  } else if (sections.count() == 1 && url.host() == "globaltags") {
    song.set_title(tr(kTitleTag).arg(sections[0]));
  } else {
    return PlaylistItemPtr();
  }

  return PlaylistItemPtr(new InternetPlaylistItem(this, song));
}

void LastFMService::ToggleScrobbling() {
  //toggle status
  scrobbling_enabled_ = !scrobbling_enabled_;

  //save to the settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("ScrobblingEnabled", scrobbling_enabled_);
  s.endGroup();

  emit ScrobblingEnabledChanged(scrobbling_enabled_);
}
