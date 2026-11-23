#ifndef INCOMINGDATAPARSER_H
#define INCOMINGDATAPARSER_H

#include <memory>

#include "core/application.h"
#include "core/player.h"
#include "networkremote/remotesongloader.h"
#include "remoteclient.h"
#include "remotecontrolmessages.pb.h"
#include "ui/mainwindow.h"

class IncomingDataParser : public QObject {
  Q_OBJECT
 public:
  IncomingDataParser(Application* app);
  ~IncomingDataParser();

  void SetRemoteRootFiles(const QString& files_root_folder) {
    files_root_folder_ = files_root_folder;
  }
  void SetStreamingEnabled(bool enabled) { streaming_enabled_ = enabled; }

 public slots:
  void Parse(const cpb::remote::Message& msg);
  void ReloadSettings();

 signals:
  void SendClementineInfo(int client_id);
  void SendFirstData(int client_id, bool send_playlist_songs);
  void SendPlaylistsRequested(int client_id, bool include_closed);
  void SendPlaylistSongs(int client_id, int id);
  void New(const QString& new_playlist_name);
  void Open(int id);
  void Clear(int id);
  void Close(int id);
  void Rename(int id, const QString& new_playlist_name);
  void Favorite(int id, bool favorite);
  void GetLyrics(int client_id);
  void Love();

  void Play();
  void PlayPause();
  void Pause();
  void Stop();
  void StopAfterCurrent();
  void Next();
  void Previous();
  void SetVolume(int volume);
  void PlayAt(int i, Engine::TrackChangeFlags change, bool reshuffle);
  void ChangeSongRequested(int playlist_id, int song_index, bool enqueue_mode);
  void ShuffleCurrent();
  void SetRepeatMode(PlaylistSequence::RepeatMode mode);
  void SetShuffleMode(PlaylistSequence::ShuffleMode mode);
  void InsertSongs(int id, const SongList& songs, int pos, bool play_now,
                   bool enqueue, bool enqueue_next);
  void InsertSongsInNewPlaylist(const QString& name, const SongList& songs,
                                int pos, bool play_now, bool enqueue,
                                bool enqueue_next);
  void RemoveSongs(int id, const QList<int>& indices);
  void SeekTo(int seconds);
  void SendLibrary(int client_id);
  void RateCurrentSong(double);

  void DoGlobalSearch(int client_id, const QString& query);

  void SendSavedRadios(int client_id);
  void SendListFiles(const QString& relative_path, int client_id);
  void AddToPlaylistSignal(QMimeData* data);
  void SetCurrentPlaylistIfNotPlaying(int id);
  void DownloadSongsRequested(int client_id,
                              const QByteArray& serialized_request);

  // Remote playback. These cross to the main thread, so they carry
  // serialized messages and client ids.
  void RendererConnected(int client_id, const QByteArray& caps,
                         const QString& local_address, quint16 local_port,
                         const QString& peer_address);
  void RendererMessage(int client_id, const QByteArray& data);
  // Browsing the Internet sidebar, for the InternetBrowser on the main
  // thread.
  void BrowseMessage(int client_id, const QByteArray& data);
  // Listing and playing the library's smart playlists, for the
  // RemoteSmartPlaylists on the main thread.
  void SmartPlaylistMessage(int client_id, const QByteArray& data);
  // Syncing songs to remotes, for the MediaSync on the main thread. Only with
  // --media-sync.
  void SyncMessage(int client_id, const QString& peer_address,
                   const QByteArray& data);

 private:
  Application* app_;
  MainWindow::PlaylistAddBehaviour doubleclick_playlist_addmode_;
  QString files_root_folder_;
  bool streaming_enabled_;
  std::unique_ptr<RemoteSongLoader> song_loader_;

  void GetPlaylistSongs(RemoteClient* client, const cpb::remote::Message& msg);
  void ChangeSong(const cpb::remote::Message& msg);
  void SetRepeatMode(const cpb::remote::Repeat& repeat);
  void SetShuffleMode(const cpb::remote::Shuffle& shuffle);
  void InsertUrls(const cpb::remote::Message& msg);
  void RemoveSongs(const cpb::remote::Message& msg);
  void ClientConnect(const cpb::remote::Message& msg, RemoteClient* client);
  void SendPlaylists(RemoteClient* client, const cpb::remote::Message& msg);
  void OpenPlaylist(const cpb::remote::Message& msg);
  void ClosePlaylist(const cpb::remote::Message& msg);
  void UpdatePlaylist(const cpb::remote::Message& msg);
  void RateSong(const cpb::remote::Message& msg);
  void GlobalSearch(RemoteClient* client, const cpb::remote::Message& msg);
  void AppendFilesToPlaylist(const cpb::remote::Message& msg);

  // Whether a client may add `url` to a playlist (see RemoteFiles::MayAdd).
  bool MayAdd(const QUrl& url) const;
  // Replaces the covers a client gave `song` with the library's, or drops
  // them when they'd be loaded from a file (see RemoteFiles::IsSafeCover).
  void TrustCoverFromLibrary(Song* song) const;
  // Whether a song loaded from a URL a client sent may be added: see
  // RemoteFiles::MayAdd. Drops covers it shouldn't have.
  RemoteSongLoader::Accept AcceptSong() const;
  // Adds songs from the Files tab, as APPEND_FILES asks.
  void AddFilesToPlaylist(const cpb::remote::RequestAppendFiles& req_append,
                          const SongList& songs);
  Song CreateSongFromProtobuf(const cpb::remote::SongMetadata& pb);
};

#endif  // INCOMINGDATAPARSER_H
