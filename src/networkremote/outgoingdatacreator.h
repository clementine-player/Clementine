#ifndef OUTGOINGDATACREATOR_H
#define OUTGOINGDATACREATOR_H

#include <QImage>
#include <QList>
#include <QMap>
#include <QQueue>
#include <QTcpSocket>
#include <QTimer>
#include <memory>

#include "core/application.h"
#include "core/player.h"
#include "engines/engine_fwd.h"
#include "engines/enginebase.h"
#include "globalsearch/globalsearch.h"
#include "playlist/playlist.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlistmanager.h"
#include "remoteclient.h"
#include "remotecontrolmessages.pb.h"
#include "songinfo/collapsibleinfopane.h"
#include "songinfo/songinfofetcher.h"
#include "songinfo/songinfoprovider.h"
#include "songinfo/songinfoview.h"
#include "songinfo/ultimatelyricslyric.h"
#include "songinfo/ultimatelyricsprovider.h"
#include "songinfo/ultimatelyricsreader.h"

typedef QList<SongInfoProvider*> ProviderList;

struct GlobalSearchRequest {
  int id_;
  QString query_;
  RemoteClient* client_;
  GlobalSearchRequest() : id_(-1), client_(nullptr) {}
  GlobalSearchRequest(int i, const QString& q, RemoteClient* c)
      : id_(i), query_(q), client_(c) {}
};

class OutgoingDataCreator : public QObject {
  Q_OBJECT
 public:
  OutgoingDataCreator(Application* app);
  ~OutgoingDataCreator();

  static const quint32 kFileChunkSize;

  void SetClients(QList<RemoteClient*>* clients);

  static void CreateSong(const Song& song, const QImage& art, const int index,
                         pb::remote::SongMetadata* song_metadata);

 public slots:
  void SendClementineInfo();
  void SendAllPlaylists();
  void SendAllActivePlaylists();
  void SendFirstData(bool send_playlist_songs);
  void SendPlaylistSongs(int id);
  void PlaylistChanged(Playlist*);
  void VolumeChanged(int volume);
  void PlaylistAdded(int id, const QString& name, bool favorite);
  void PlaylistDeleted(int id);
  void PlaylistClosed(int id);
  void PlaylistRenamed(int id, const QString& new_name);
  void ActiveChanged(Playlist*);
  void CurrentSongChanged(const Song& song, const QString& uri,
                          const QImage& img);
  void SendSongMetadata();
  void StateChanged(Engine::State);
  void SendKeepAlive();
  void SendRepeatMode(PlaylistSequence::RepeatMode mode);
  void SendShuffleMode(PlaylistSequence::ShuffleMode mode);
  void UpdateTrackPosition();
  void DisconnectAllClients();
  void GetLyrics();
  void SendLyrics(int id, const SongInfoFetcher::Result& result);
  void SendLibrary(RemoteClient* client);
  void EnableKittens(bool aww);
  void SendKitten(const QImage& kitten);

  void DoGlobalSearch(const QString& query, RemoteClient* client);
  void ResultsAvailable(int id, const SearchProvider::ResultList& results);
  void SearchFinished(int id);

 private:
  Application* app_;
  QList<RemoteClient*>* clients_;
  Song current_song_;
  QString current_uri_;
  QImage current_image_;
  Engine::State last_state_;
  QTimer* keep_alive_timer_;
  QTimer* track_position_timer_;
  int keep_alive_timeout_;
  int last_track_position_;
  bool aww_;

  std::unique_ptr<UltimateLyricsReader> ultimate_reader_;
  QMap<int, SongInfoFetcher::Result> results_;
  SongInfoFetcher* fetcher_;

  QMap<int, GlobalSearchRequest> global_search_result_map_;

  void SendDataToClients(pb::remote::Message* msg);
  void SetEngineState(pb::remote::ResponseClementineInfo* msg);
  void CheckEnabledProviders();
  SongInfoProvider* ProviderByName(const QString& name) const;
};

#endif  // OUTGOINGDATACREATOR_H
