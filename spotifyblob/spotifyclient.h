#ifndef SPOTIFYCLIENT_H
#define SPOTIFYCLIENT_H

#include "spotifymessageutils.h"

#include <QObject>

#include <libspotify/api.h>

class QTcpSocket;
class QTimer;

class ResponseMessage;

class SpotifyClient : public QObject, protected SpotifyMessageUtils {
  Q_OBJECT

public:
  SpotifyClient(QObject* parent = 0);
  ~SpotifyClient();

  void Init(quint16 port);

private slots:
  void SocketReadyRead();
  void ProcessEvents();

private:
  void SendLoginCompleted(bool success, const QString& error);

  // Spotify session callbacks.
  static void LoggedInCallback(sp_session* session, sp_error error);
  static void NotifyMainThreadCallback(sp_session* session);
  static void LogMessageCallback(sp_session* session, const char* data);
  static void SearchCompleteCallback(sp_search* result, void* userdata);

  // Spotify playlist container callbacks.
  static void PlaylistAddedCallback(
    sp_playlistcontainer* pc, sp_playlist* playlist,
    int position, void* userdata);
  static void PlaylistRemovedCallback(
    sp_playlistcontainer* pc, sp_playlist* playlist,
    int position, void* userdata);
  static void PlaylistMovedCallback(
    sp_playlistcontainer* pc, sp_playlist* playlist,
    int position, int new_position, void* userdata);
  static void PlaylistContainerLoadedCallback(
    sp_playlistcontainer* pc, void* userdata);

  // Request handlers.
  void Login(const QString& username, const QString& password);
  void GetPlaylists();
  void Search(const QString& query);

  QTcpSocket* socket_;

  sp_session_config spotify_config_;
  sp_session_callbacks spotify_callbacks_;
  sp_playlistcontainer_callbacks playlistcontainer_callbacks_;
  sp_session* session_;

  QTimer* events_timer_;
};

#endif // SPOTIFYCLIENT_H
