#ifndef OUTGOINGDATACREATOR_H
#define OUTGOINGDATACREATOR_H

#include <QTcpSocket>
#include <QImage>
#include <QList>
#include <QTimer>

#include "core/player.h"
#include "core/application.h"
#include "engines/enginebase.h"
#include "engines/engine_fwd.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "remotecontrolmessages.pb.h"
#include "remoteclient.h"

class OutgoingDataCreator : public QObject {
    Q_OBJECT
public:
  OutgoingDataCreator(Application* app);
  ~OutgoingDataCreator();

  void SetClients(QList<RemoteClient*>* clients);

public slots:
  void SendClementineInfos();
  void SendAllPlaylists();
  void SendFirstData();
  void SendPlaylistSongs(int id);
  void PlaylistChanged(Playlist*);
  void VolumeChanged(int volume);
  void ActiveChanged(Playlist*);
  void CurrentSongChanged(const Song& song, const QString& uri, const QImage& img);
  void StateChanged(Engine::State);
  void SendKeepAlive();

private:
  Application* app_;
  QList<RemoteClient*>* clients_;
  Song current_song_;
  QString current_uri_;
  QImage current_image_;
  Engine::State last_state_;
  QTimer* keep_alive_timer_;
  int keep_alive_timeout_;

  void SendDataToClients(pb::remote::Message* msg);
  void SetEngineState(pb::remote::Message* msg);
  void CreateSong(pb::remote::SongMetadata* song_metadata, Song* song, const QString* art_uri, int index);
};

#endif // OUTGOINGDATACREATOR_H
