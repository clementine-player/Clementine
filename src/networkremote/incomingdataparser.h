#ifndef INCOMINGDATAPARSER_H
#define INCOMINGDATAPARSER_H

#include "core/application.h"
#include "core/player.h"
#include "remoteclient.h"
#include "remotecontrolmessages.pb.h"
#include "ui/mainwindow.h"

class IncomingDataParser : public QObject {
  Q_OBJECT
 public:
  IncomingDataParser(Application* app);
  ~IncomingDataParser();

  bool close_connection();

  void SetRemoteRootFiles(const QString& files_root_folder) {
    files_root_folder_ = files_root_folder;
  }

 public slots:
  void Parse(const pb::remote::Message& msg);
  void ReloadSettings();

 signals:
  void SendClementineInfo();
  void SendFirstData(bool send_playlist_songs);
  void SendAllPlaylists();
  void SendAllActivePlaylists();
  void SendPlaylistSongs(int id);
  void New(const QString& new_playlist_name);
  void Open(int id);
  void Close(int id);
  void Rename(int id, const QString& new_playlist_name);
  void Favorite(int id, bool favorite);
  void GetLyrics();
  void Love();
  void Ban();

  void Play();
  void PlayPause();
  void Pause();
  void Stop();
  void StopAfterCurrent();
  void Next();
  void Previous();
  void SetVolume(int volume);
  void PlayAt(int i, Engine::TrackChangeFlags change, bool reshuffle);
  void Enque(int id, int i);
  void SetActivePlaylist(int id);
  void ShuffleCurrent();
  void SetRepeatMode(PlaylistSequence::RepeatMode mode);
  void SetShuffleMode(PlaylistSequence::ShuffleMode mode);
  void InsertUrls(int id, const QList<QUrl>& urls, int pos, bool play_now,
                  bool enqueue);
  void InsertSongs(int id, const SongList& songs, int pos, bool play_now,
                   bool enqueue);
  void RemoveSongs(int id, const QList<int>& indices);
  void SeekTo(int seconds);
  void SendLibrary(RemoteClient* client);
  void RateCurrentSong(double);

  void DoGlobalSearch(QString, RemoteClient*);

  void SendListFiles(QString);
  void AddToPlaylistSignal(QMimeData* data);
  void SendSavedRadios();
  void SetCurrentPlaylist(int id);

 private:
  Application* app_;
  bool close_connection_;
  MainWindow::PlaylistAddBehaviour doubleclick_playlist_addmode_;
  QString files_root_folder_;

  void GetPlaylistSongs(const pb::remote::Message& msg);
  void ChangeSong(const pb::remote::Message& msg);
  void SetRepeatMode(const pb::remote::Repeat& repeat);
  void SetShuffleMode(const pb::remote::Shuffle& shuffle);
  void InsertUrls(const pb::remote::Message& msg);
  void RemoveSongs(const pb::remote::Message& msg);
  void ClientConnect(const pb::remote::Message& msg, RemoteClient* client);
  void SendPlaylists(const pb::remote::Message& msg);
  void OpenPlaylist(const pb::remote::Message& msg);
  void ClosePlaylist(const pb::remote::Message& msg);
  void UpdatePlaylist(const pb::remote::Message& msg);
  void RateSong(const pb::remote::Message& msg);
  void GlobalSearch(RemoteClient* client, const pb::remote::Message& msg);
  void AppendFilesToPlaylist(const pb::remote::Message& msg);

  Song CreateSongFromProtobuf(const pb::remote::SongMetadata& pb);
};

#endif  // INCOMINGDATAPARSER_H
