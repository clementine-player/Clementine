#include "spotifyblob.h"
#include "spotifyclient.h"
#include "spotifykey.h"

#include <QDir>
#include <QTimer>
#include <QtDebug>

SpotifyBlob::SpotifyBlob(SpotifyClient* client, QObject* parent)
  : QObject(parent),
    client_(client),
    session_(NULL),
    events_timer_(new QTimer(this)) {
  memset(&spotify_callbacks_, 0, sizeof(spotify_callbacks_));
  memset(&spotify_config_, 0, sizeof(spotify_config_));

  spotify_callbacks_.logged_in = &LoggedIn;
  spotify_callbacks_.notify_main_thread = &NotifyMainThread;
  spotify_callbacks_.log_message = &LogMessage;

  spotify_config_.api_version = SPOTIFY_API_VERSION;  // From libspotify/api.h
  spotify_config_.cache_location = strdup(QDir::tempPath().toLocal8Bit().constData());
  spotify_config_.settings_location = strdup(QDir::tempPath().toLocal8Bit().constData());
  spotify_config_.application_key = g_appkey;
  spotify_config_.application_key_size = g_appkey_size;
  spotify_config_.callbacks = &spotify_callbacks_;
  spotify_config_.userdata = this;
  spotify_config_.user_agent = "Clementine Player";

  events_timer_->setSingleShot(true);
  connect(events_timer_, SIGNAL(timeout()), SLOT(ProcessEvents()));

  connect(client, SIGNAL(Login(QString,QString)), SLOT(Login(QString,QString)));
}

SpotifyBlob::~SpotifyBlob() {
  if (session_) {
    sp_session_release(session_);
  }

  free(const_cast<char*>(spotify_config_.cache_location));
  free(const_cast<char*>(spotify_config_.settings_location));
}

void SpotifyBlob::Login(const QString& username, const QString& password) {
  sp_error error = sp_session_create(&spotify_config_, &session_);
  if (error != SP_ERROR_OK) {
    qWarning() << "Failed to create session" << sp_error_message(error);
    client_->LoginCompleted(false, sp_error_message(error));
    return;
  }

  sp_session_login(session_, username.toUtf8().constData(), password.toUtf8().constData());
}

void SpotifyBlob::LoggedIn(sp_session* session, sp_error error) {
  qDebug() << Q_FUNC_INFO;
  SpotifyBlob* me = reinterpret_cast<SpotifyBlob*>(sp_session_userdata(session));
  me->LoggedIn(error);
}

void SpotifyBlob::LoggedIn(sp_error error) {
  if (error != SP_ERROR_OK) {
    qWarning() << "Failed to login" << sp_error_message(error);
  }

  client_->LoginCompleted(error == SP_ERROR_OK, sp_error_message(error));
}

void SpotifyBlob::NotifyMainThread(sp_session* session) {
  qDebug() << Q_FUNC_INFO;
  SpotifyBlob* me = reinterpret_cast<SpotifyBlob*>(sp_session_userdata(session));
  me->Notify();
}

// Called by spotify from an internal thread to notify us that its events need processing.
void SpotifyBlob::Notify() {
  metaObject()->invokeMethod(this, "ProcessEvents", Qt::QueuedConnection);
}

void SpotifyBlob::ProcessEvents() {
  qDebug() << Q_FUNC_INFO;
  int next_timeout_ms;
  sp_session_process_events(session_, &next_timeout_ms);
  qDebug() << next_timeout_ms << events_timer_;
  events_timer_->start(next_timeout_ms);
  qDebug() << "Started";
}

void SpotifyBlob::LogMessage(sp_session* session, const char* data) {
  qDebug() << Q_FUNC_INFO;
}

void SpotifyBlob::Search(const QString& query) {
  sp_search_create(
      session_,
      query.toUtf8().constData(),
      0,   // track offset
      10,  // track count
      0,   // album offset
      10,  // album count
      0,   // artist offset
      10,  // artist count
      &SearchComplete,
      this);
}

void SpotifyBlob::SearchComplete(sp_search* result, void* userdata) {
  sp_error error = sp_search_error(result);
  if (error != SP_ERROR_OK) {
    qWarning() << "Search failed";
    sp_search_release(result);
    return;
  }

  int artists = sp_search_num_artists(result);
  for (int i = 0; i < artists; ++i) {
    sp_artist* artist = sp_search_artist(result, i);
    qDebug() << "Found artist:" << sp_artist_name(artist);
  }

  sp_search_release(result);
}

