#ifndef OUTGOINGDATACREATOR_H
#define OUTGOINGDATACREATOR_H

#include <QHash>
#include <QImage>
#include <QList>
#include <QMap>
#include <QQueue>
#include <QTcpSocket>
#include <QTimer>
#include <memory>

#include "core/application.h"
#include "core/player.h"
#include "core/urlhandler.h"
#include "engines/engine_fwd.h"
#include "engines/enginebase.h"
#include "playlist/playlist.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlistmanager.h"
#include "remoteclient.h"
#include "remotecontrolmessages.pb.h"
#include "songinfo/lyricsfetcher.h"

class OutgoingDataCreator : public QObject {
  Q_OBJECT
 public:
  OutgoingDataCreator(Application* app);
  ~OutgoingDataCreator();

  static const quint32 kFileChunkSize;

  void SetClients(QList<RemoteClient*>* clients);
  void SetRemoteRootFiles(const QString& files_root_folder) {
    files_root_folder_ = files_root_folder;
  }
  void SetMusicExtensions(const QStringList& files_music_extensions) {
    files_music_extensions_ = files_music_extensions;
  }
  void SetAllowDownloads(bool allow_downloads) {
    allow_downloads_ = allow_downloads;
  }
  void SetStreamingEnabled(bool enabled) { streaming_enabled_ = enabled; }
  void SetSyncEnabled(bool enabled) { sync_enabled_ = enabled; }
  void SendDataToClients(cpb::remote::Message* msg);
  static void CreateSong(const Song& song, const QImage& art, const int index,
                         cpb::remote::SongMetadata* song_metadata);
  // Whether a remote can keep |song| on the phone, asking |handler|, the
  // UrlHandler for its scheme (null for none).
  static cpb::remote::Downloadability DownloadabilityOf(
      const Song& song, const UrlHandler* handler);

 signals:
  void RequestTrackPosition();

 public slots:
  void SendClementineInfo(int client_id, Engine::State state);
  void SendFirstData(int client_id, const Song& current_song, int current_row,
                     int current_downloadability, Engine::State state,
                     int volume, qint64 position_nanosec,
                     const QByteArray& playlists_pb,
                     const QByteArray& playlist_songs_pb,
                     PlaylistSequence::RepeatMode repeat_mode,
                     PlaylistSequence::ShuffleMode shuffle_mode);
  void SendRawToClient(int client_id, const QByteArray& serialized_proto);
  void SendRawToAllClients(const QByteArray& serialized_proto);
  void VolumeChanged(int volume);
  void CurrentSongChanged(const Song& song, const QString& uri,
                          const QImage& img, int current_row,
                          int downloadability);
  void SendSongMetadata();
  void StateChanged(Engine::State state);
  void SendKeepAlive();
  void SendRepeatMode(PlaylistSequence::RepeatMode mode);
  void SendShuffleMode(PlaylistSequence::ShuffleMode mode);
  void UpdateTrackPosition();
  void SendTrackPosition(qint64 position_nanosec);
  void DisconnectAllClients();
  void GetLyrics(int client_id);
  void SendLyrics(int id, const Lyrics& lyrics);
  void SendLibrary(int client_id);
  void EnableKittens(bool aww);
  void SendKitten(const QImage& kitten);

  void SendListFiles(QString relative_path, int client_id);

 private:
  RemoteClient* FindClient(int client_id) const;
  void SendDataToClient(int client_id, cpb::remote::Message* msg);
  static void SetEngineState(Engine::State state,
                             cpb::remote::ResponseClementineInfo* msg);
  static void FillRepeatMessage(PlaylistSequence::RepeatMode mode,
                                cpb::remote::Message* msg);
  static void FillShuffleMessage(PlaylistSequence::ShuffleMode mode,
                                 cpb::remote::Message* msg);

  Application* app_;
  QList<RemoteClient*>* clients_;
  Song current_song_;
  QString current_uri_;
  QImage current_image_;
  int current_row_;
  int current_downloadability_;
  Engine::State last_state_;
  QTimer* keep_alive_timer_;
  QTimer* track_position_timer_;
  int keep_alive_timeout_;
  int last_track_position_;
  bool aww_;
  QString files_root_folder_;
  QStringList files_music_extensions_;
  bool allow_downloads_;
  bool streaming_enabled_;
  bool sync_enabled_;

  LyricsFetcher* lyrics_fetcher_;
  QHash<int, int> lyrics_requests_;
};

#endif  // OUTGOINGDATACREATOR_H
