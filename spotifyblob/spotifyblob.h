#ifndef SPOTIFY_H
#define SPOTIFY_H

#include <QObject>

#include <libspotify/api.h>

class SpotifyClient;

class QTimer;

class SpotifyBlob : public QObject {
  Q_OBJECT

public:
  SpotifyBlob(SpotifyClient* client, QObject* parent = 0);
  ~SpotifyBlob();

private slots:
  void ProcessEvents();
  void Login(const QString& username, const QString& password);
  void Search(const QString& query);

private:
  // Spotify callbacks.
  static void LoggedIn(sp_session* session, sp_error error);
  static void NotifyMainThread(sp_session* session);
  static void LogMessage(sp_session* session, const char* data);

  static void SearchComplete(sp_search* result, void* userdata);

  void Notify();
  void LoggedIn(sp_error error);

  SpotifyClient* client_;

  sp_session_config spotify_config_;
  sp_session_callbacks spotify_callbacks_;
  sp_session* session_;

  QTimer* events_timer_;
};

#endif // SPOTIFY_H
