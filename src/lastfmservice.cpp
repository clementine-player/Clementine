#include "lastfmservice.h"
#include "lastfmconfig.h"
#include "radioitem.h"
#include "song.h"

#include <lastfm/ws.h>
#include <lastfm/misc.h>
#include <lastfm/XmlQuery>

#include <QSettings>

const char* LastFMService::kSettingsGroup = "Last.fm";

LastFMService::LastFMService(QObject* parent)
  : RadioService("Last.fm", parent),
    tuner_(NULL),
    initial_tune_(false)
{
  lastfm::ws::ApiKey = "75d20fb472be99275392aefa2760ea09";
  lastfm::ws::SharedSecret = "d3072b60ae626be12be69448f5c46e70";

  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  lastfm::ws::Username = settings.value("username").toString();
  lastfm::ws::SessionKey = settings.value("session").toString();

  config_ = new LastFMConfig(this);
}

LastFMService::~LastFMService() {
  delete config_;
}

RadioItem* LastFMService::CreateRootItem(RadioItem* parent) {
  RadioItem* item = new RadioItem(this, RadioItem::Type_Service, "Last.fm", parent);
  item->icon = QIcon(":last.fm/as.png");
  return item;
}

void LastFMService::LazyPopulate(RadioItem *item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      // Create child items
      CreateStationItem(Type_MyRecommendations, "My Recommendations",
                        ":last.fm/recommended_radio.png", item);
      CreateStationItem(Type_MyRadio, "My Radio Station",
                        ":last.fm/personal_radio.png", item);
      CreateStationItem(Type_MyLoved, "My Loved Tracks",
                        ":last.fm/loved_radio.png", item);
      CreateStationItem(Type_MyNeighbourhood, "My Neighbourhood",
                        ":last.fm/neighbour_radio.png", item);

      if (lastfm::ws::SessionKey.isEmpty())
        config_->show();
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
  settings.setValue("username", lastfm::ws::Username);
  settings.setValue("session", lastfm::ws::SessionKey);

  emit AuthenticationComplete(true);
}

QList<RadioItem::PlaylistData> LastFMService::DataForItem(RadioItem* item) {
  QList<RadioItem::PlaylistData> ret;

  const QString user(lastfm::ws::Username);

  switch (item->type) {
    case Type_MyRecommendations:
      ret << RadioItem::PlaylistData(user + "'s Recommended Radio",
                                     "lastfm://user/" + lastfm::ws::Username + "/recommended");
      break;

    case Type_MyLoved:
      ret << RadioItem::PlaylistData(user + "'s Loved Tracks",
                                     "lastfm://user/" + lastfm::ws::Username + "/loved");
      break;

    case Type_MyNeighbourhood:
      ret << RadioItem::PlaylistData(user + "'s Neighbour Radio",
                                     "lastfm://user/" + lastfm::ws::Username + "/neighbours");
      break;

    case Type_MyRadio:
      ret << RadioItem::PlaylistData(user + "'s Library",
                                     "lastfm://user/" + lastfm::ws::Username + "/library");
      break;
  }

  return ret;
}

void LastFMService::StartLoading(const QUrl& url) {
  if (url.scheme() != "lastfm")
    return;
  if (lastfm::ws::SessionKey.isEmpty())
    return;

  emit LoadingStarted();

  delete tuner_;

  last_url_ = url;
  initial_tune_ = true;
  tuner_ = new lastfm::RadioTuner(lastfm::RadioStation(url));

  connect(tuner_, SIGNAL(trackAvailable()), SLOT(TunerTrackAvailable()));
  connect(tuner_, SIGNAL(error(lastfm::ws::Error)), SLOT(TunerError(lastfm::ws::Error)));
}

void LastFMService::LoadNext(const QUrl &) {
  lastfm::Track track = tuner_->takeNextTrack();

  if (track.isNull()) {
    emit StreamFinished();
    return;
  }

  emit StreamReady(last_url_, track.url());

  Song metadata;
  metadata.InitFromLastFM(track);
  emit StreamMetadataFound(last_url_, metadata);
}

void LastFMService::TunerError(lastfm::ws::Error error) {
  qDebug() << "Last.fm error" << error;
  if (!initial_tune_)
    return;

  emit LoadingFinished();

  if (error == lastfm::ws::NotEnoughContent) {
    emit StreamFinished();
    return;
  }

  emit StreamError(ErrorString(error));
}

QString LastFMService::ErrorString(lastfm::ws::Error error) const {
  switch (error) {
    case lastfm::ws::InvalidService: return "Invalid service";
    case lastfm::ws::InvalidMethod: return "Invalid method";
    case lastfm::ws::AuthenticationFailed: return "Authentication failed";
    case lastfm::ws::InvalidFormat: return "Invalid format";
    case lastfm::ws::InvalidParameters: return "Invalid parameters";
    case lastfm::ws::InvalidResourceSpecified: return "Invalid resource specified";
    case lastfm::ws::OperationFailed: return "Operation failed";
    case lastfm::ws::InvalidSessionKey: return "Invalid session key";
    case lastfm::ws::InvalidApiKey: return "Invalid API key";
    case lastfm::ws::ServiceOffline: return "Service offline";
    case lastfm::ws::SubscribersOnly: return "This stream is for paid subscribers only";

    case lastfm::ws::TryAgainLater: return "Last.fm is currently busy, please try again in a few minutes";

    case lastfm::ws::NotEnoughContent: return "Not enough content";
    case lastfm::ws::NotEnoughMembers: return "Not enough members";
    case lastfm::ws::NotEnoughFans: return "Not enough fans";
    case lastfm::ws::NotEnoughNeighbours: return "Not enough neighbours";

    case lastfm::ws::MalformedResponse: return "Malformed response";

    case lastfm::ws::UnknownError:
    default:
      return "Unknown error";
  }
}

void LastFMService::TunerTrackAvailable() {
  if (initial_tune_) {
    emit LoadingFinished();

    LoadNext(last_url_);
    initial_tune_ = false;
  }
}
