#include "lastfmservice.h"
#include "radioitem.h"
#include "song.h"
#include "lastfmstationdialog.h"
#include "lastfmconfigdialog.h"

#include <lastfm/ws.h>
#include <lastfm/misc.h>
#include <lastfm/XmlQuery>
#include <lastfm/Audioscrobbler>

#include <QSettings>
#include <QMenu>

const char* LastFMService::kServiceName = "Last.fm";
const char* LastFMService::kSettingsGroup = "Last.fm";
const char* LastFMService::kAudioscrobblerClientId = "tng";
const char* LastFMService::kApiKey = "75d20fb472be99275392aefa2760ea09";
const char* LastFMService::kSecret = "d3072b60ae626be12be69448f5c46e70";

LastFMService::LastFMService(QObject* parent)
  : RadioService(kServiceName, parent),
    tuner_(NULL),
    scrobbler_(NULL),
    config_(NULL),
    station_dialog_(new LastFMStationDialog),
    context_menu_(new QMenu),
    initial_tune_(false),
    scrobbling_enabled_(false),
    artist_list_(NULL),
    tag_list_(NULL),
    friends_list_(NULL),
    neighbours_list_(NULL)
{
  lastfm::ws::ApiKey = kApiKey;
  lastfm::ws::SharedSecret = kSecret;

  ReloadSettings();

  play_action_ = context_menu_->addAction(
      QIcon(":media-playback-start.png"), tr("Add to playlist"), this, SLOT(AddToPlaylist()));
  remove_action_ = context_menu_->addAction(
      QIcon(":list-remove.png"), tr("Remove"), this, SLOT(Remove()));
  context_menu_->addSeparator();
  add_artist_action_ = context_menu_->addAction(
      QIcon(":last.fm/icon_radio.png"), tr("Play artist radio..."), this, SLOT(AddArtistRadio()));
  add_tag_action_ = context_menu_->addAction(
      QIcon(":last.fm/icon_tag.png"), tr("Play tag radio..."), this, SLOT(AddTagRadio()));
  context_menu_->addAction(
      QIcon(":configure.png"), tr("Configure Last.fm..."), this, SLOT(ShowConfig()));

  remove_action_->setEnabled(false);
  add_artist_action_->setEnabled(false);
  add_tag_action_->setEnabled(false);
}

LastFMService::~LastFMService() {
  delete config_;
  delete station_dialog_;
  delete context_menu_;
}

void LastFMService::ReloadSettings() {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  lastfm::ws::Username = settings.value("Username").toString();
  lastfm::ws::SessionKey = settings.value("Session").toString();
  scrobbling_enabled_ = settings.value("ScrobblingEnabled", true).toBool();

  emit ScrobblingEnabledChanged(scrobbling_enabled_);
}

void LastFMService::ShowConfig() {
  if (!config_) {
    config_ = new LastFMConfigDialog;
  }

  config_->show();
}

bool LastFMService::IsAuthenticated() const {
  return !lastfm::ws::SessionKey.isEmpty();
}

RadioItem* LastFMService::CreateRootItem(RadioItem* parent) {
  RadioItem* item = new RadioItem(this, RadioItem::Type_Service, kServiceName, parent);
  item->icon = QIcon(":last.fm/as.png");
  return item;
}

void LastFMService::LazyPopulate(RadioItem *item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      // Normal radio types
      CreateStationItem(Type_MyRecommendations, tr("My Recommendations"), ":last.fm/recommended_radio.png", item);
      CreateStationItem(Type_MyRadio, tr("My Radio Station"), ":last.fm/personal_radio.png", item);
      CreateStationItem(Type_MyLoved, tr("My Loved Tracks"), ":last.fm/loved_radio.png", item);
      CreateStationItem(Type_MyNeighbourhood, tr("My Neighbourhood"), ":last.fm/neighbour_radio.png", item);

      // Types that have children
      artist_list_ = new RadioItem(this, Type_ArtistRadio, tr("Artist radio"), item);
      artist_list_->icon = QIcon(":last.fm/icon_radio.png");
      artist_list_->lazy_loaded = true;

      tag_list_ = new RadioItem(this, Type_TagRadio, tr("Tag radio"), item);
      tag_list_->icon = QIcon(":last.fm/icon_tag.png");
      tag_list_->lazy_loaded = true;

      RestoreList("artists", Type_Artist, QIcon(":last.fm/icon_radio.png"), artist_list_);
      RestoreList("tags", Type_Tag, QIcon(":last.fm/icon_tag.png"), tag_list_);

      friends_list_ = new RadioItem(this, Type_MyFriends, tr("Friends"), item);
      friends_list_->icon = QIcon(":last.fm/my_friends.png");

      neighbours_list_ = new RadioItem(this, Type_MyNeighbours, tr("Neighbours"), item);
      neighbours_list_->icon = QIcon(":last.fm/my_neighbours.png");

      if (!IsAuthenticated())
        ShowConfig();

      add_artist_action_->setEnabled(true);
      add_tag_action_->setEnabled(true);
      break;

    case Type_MyFriends:
      RefreshFriends();
      break;

    case Type_MyNeighbours:
      RefreshNeighbours();
      break;

    case Type_OtherUser:
      CreateStationItem(Type_OtherUserRadio, item->key, ":last.fm/recommended_radio.png", item)
          ->display_text = tr("%1's Radio Station").arg(item->key);
      CreateStationItem(Type_OtherUserLoved, item->key, ":last.fm/loved_radio.png", item)
          ->display_text = tr("%1's Loved Tracks").arg(item->key);
      CreateStationItem(Type_OtherUserNeighbourhood, item->key, ":last.fm/neighbour_radio.png", item)
          ->display_text = tr("%1's Neighborhood").arg(item->key);
      break;

    default:
      break;
  }

  item->lazy_loaded = true;
}

RadioItem* LastFMService::CreateStationItem(ItemType type, const QString& name,
                                            const QString& icon, RadioItem* parent) {
  RadioItem* ret = new RadioItem(this, type, name, parent);
  ret->lazy_loaded = true;
  ret->icon = QIcon(icon);
  ret->playable = true;

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

void LastFMService::AuthenticateReplyFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    emit AuthenticationComplete(false);
    return;
  }

  // Parse the reply
  try {
    lastfm::XmlQuery const lfm = lastfm::ws::parse(reply);

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

QUrl LastFMService::UrlForItem(const RadioItem* item) const {
  switch (item->type) {
    case Type_MyRecommendations:
      return "lastfm://user/" + lastfm::ws::Username + "/recommended";

    case Type_MyLoved:
      return "lastfm://user/" + lastfm::ws::Username + "/loved";

    case Type_MyNeighbourhood:
      return "lastfm://user/" + lastfm::ws::Username + "/neighbours";

    case Type_MyRadio:
      return "lastfm://user/" + lastfm::ws::Username + "/library";

    case Type_OtherUser:
    case Type_OtherUserRadio:
      return "lastfm://user/" + item->key + "/library";

    case Type_OtherUserLoved:
      return "lastfm://user/" + item->key + "/loved";

    case Type_OtherUserNeighbourhood:
      return "lastfm://user/" + item->key + "/neighbours";

    case Type_Artist:
      return "lastfm://artist/" + item->key + "/similarartists";

    case Type_Tag:
      return "lastfm://globaltags/" + item->key;
  }
  return QUrl();
}

QString LastFMService::TitleForItem(const RadioItem* item) const {
  const QString me(lastfm::ws::Username);

  switch (item->type) {
    case Type_MyRecommendations: return tr("%1's Recommended Radio").arg(me);
    case Type_MyLoved:           return tr("%1's Loved Tracks").arg(me);
    case Type_MyNeighbourhood:   return tr("%1's Neighbour Radio").arg(me);
    case Type_MyRadio:           return tr("%1's Library").arg(me);
    case Type_OtherUser:
    case Type_OtherUserRadio:    return tr("%1's Library").arg(item->key);
    case Type_OtherUserLoved:    return tr("%1's Loved Tracks").arg(item->key);
    case Type_OtherUserNeighbourhood: return tr("%1's Neighbour Radio").arg(item->key);
    case Type_Artist:            return tr("Similar Artists to %1").arg(item->key);
    case Type_Tag:               return tr("Tag Radio: %1").arg(item->key);
  }
  return QString();
}

void LastFMService::StartLoading(const QUrl& url) {
  if (url.scheme() != "lastfm")
    return;
  if (!IsAuthenticated())
    return;

  emit TaskStarted(MultiLoadingIndicator::LoadingLastFM);

  delete tuner_;

  last_url_ = url;
  initial_tune_ = true;
  tuner_ = new lastfm::RadioTuner(lastfm::RadioStation(url));

  connect(tuner_, SIGNAL(trackAvailable()), SLOT(TunerTrackAvailable()));
  connect(tuner_, SIGNAL(error(lastfm::ws::Error)), SLOT(TunerError(lastfm::ws::Error)));
}

void LastFMService::LoadNext(const QUrl &) {
  last_track_ = tuner_->takeNextTrack();

  if (last_track_.isNull()) {
    emit StreamFinished();
    return;
  }

  Song metadata;
  metadata.InitFromLastFM(last_track_);

  emit StreamMetadataFound(last_url_, metadata);
  emit StreamReady(last_url_, last_track_.url());
}

void LastFMService::TunerError(lastfm::ws::Error error) {
  qDebug() << "Last.fm error" << error;
  if (!initial_tune_)
    return;

  emit TaskFinished(MultiLoadingIndicator::LoadingLastFM);

  if (error == lastfm::ws::NotEnoughContent) {
    emit StreamFinished();
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
    case lastfm::ws::NotEnoughNeighbours: return tr("Not enough neighbours");

    case lastfm::ws::MalformedResponse: return tr("Malformed response");

    case lastfm::ws::UnknownError:
    default:
      return tr("Unknown error");
  }
}

void LastFMService::TunerTrackAvailable() {
  if (initial_tune_) {
    emit TaskFinished(MultiLoadingIndicator::LoadingLastFM);

    LoadNext(last_url_);
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

  scrobbler_->nowPlaying(last_track_);
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
}

void LastFMService::Ban() {
  lastfm::MutableTrack mtrack(last_track_);
  mtrack.ban();

  Scrobble();
  LoadNext(last_url_);
}

void LastFMService::ShowContextMenu(RadioItem* item, const QPoint &global_pos) {
  context_item_ = item;

  switch (item->type) {
    case Type_Artist:
    case Type_Tag:
      remove_action_->setEnabled(true);
      break;

    default:
      remove_action_->setEnabled(false);
      break;
  }

  play_action_->setEnabled(item->playable);
  context_menu_->popup(global_pos);
}

void LastFMService::RefreshFriends() {
  if (!friends_list_ || !IsAuthenticated())
    return;

  friends_list_->ClearNotify();

  lastfm::AuthenticatedUser user;
  QNetworkReply* reply = user.getFriends();
  connect(reply, SIGNAL(finished()), SLOT(RefreshFriendsFinished()));
}

void LastFMService::RefreshNeighbours() {
  if (!friends_list_ || !IsAuthenticated())
    return;

  neighbours_list_->ClearNotify();

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
  } catch (std::runtime_error& e) {
    qDebug() << e.what();
    return;
  }

  foreach (const lastfm::User& f, friends) {
    RadioItem* item = new RadioItem(this, Type_OtherUser, f);
    item->icon = QIcon(":last.fm/icon_user.png");
    item->playable = true;
    item->InsertNotify(friends_list_);
  }
}

void LastFMService::RefreshNeighboursFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply)
    return;

  QList<lastfm::User> neighbours;

  try {
    neighbours = lastfm::User::list(reply);
  } catch (std::runtime_error& e) {
    qDebug() << e.what();
    return;
  }

  foreach (const lastfm::User& n, neighbours) {
    RadioItem* item = new RadioItem(this, Type_OtherUser, n);
    item->icon = QIcon(":last.fm/user_purple.png");
    item->playable = true;
    item->InsertNotify(neighbours_list_);
  }
}

void LastFMService::AddToPlaylist() {
  emit AddItemToPlaylist(context_item_);
}

void LastFMService::AddArtistRadio() {
  AddArtistOrTag("artists", LastFMStationDialog::Artist, Type_Artist, QIcon(":last.fm/icon_radio.png"), artist_list_);
}

void LastFMService::AddTagRadio() {
  AddArtistOrTag("tags", LastFMStationDialog::Tag, Type_Tag, QIcon(":last.fm/icon_tag.png"), tag_list_);
}

void LastFMService::AddArtistOrTag(const QString& name,
                                   LastFMStationDialog::Type dialog_type, ItemType item_type,
                                   const QIcon& icon, RadioItem* list) {
  station_dialog_->SetType(dialog_type);
  if (station_dialog_->exec() == QDialog::Rejected)
    return;

  if (station_dialog_->content().isEmpty())
    return;

  RadioItem* item = new RadioItem(this, item_type, station_dialog_->content());
  item->icon = icon;
  item->playable = true;
  item->lazy_loaded = true;
  item->InsertNotify(list);
  emit AddItemToPlaylist(item);

  SaveList(name, list);
}

void LastFMService::SaveList(const QString& name, RadioItem* list) const {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  settings.beginWriteArray(name, list->children.count());
  for (int i=0 ; i<list->children.count() ; ++i) {
    settings.setArrayIndex(i);
    settings.setValue("key", list->children[i]->key);
  }
  settings.endArray();
}

void LastFMService::RestoreList(const QString &name, ItemType item_type,
                                const QIcon& icon, RadioItem *list) {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  list->ClearNotify();

  int count = settings.beginReadArray(name);
  for (int i=0 ; i<count ; ++i) {
    settings.setArrayIndex(i);
    RadioItem* item = new RadioItem(this, item_type,
                                    settings.value("key").toString(), list);
    item->icon = icon;
    item->playable = true;
    item->lazy_loaded = true;
  }
  settings.endArray();
}

void LastFMService::Remove() {
  int type = context_item_->type;

  context_item_->parent->DeleteNotify(context_item_->row);

  if (type == Type_Artist)
    SaveList("artists", artist_list_);
  else if (type == Type_Tag)
    SaveList("tags", tag_list_);
}
