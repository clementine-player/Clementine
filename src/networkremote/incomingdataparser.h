#ifndef INCOMINGDATAPARSER_H
#define INCOMINGDATAPARSER_H

#include "core/player.h"
#include "core/application.h"
#include "remotecontrolmessages.pb.h"
#include "remoteclient.h"

class IncomingDataParser : public QObject {
  Q_OBJECT
public:
  IncomingDataParser(Application* app);
  ~IncomingDataParser();

  bool close_connection();

public slots:
  void Parse(const pb::remote::Message& msg);

signals:
  void SendClementineInfo();
  void SendFirstData(bool send_playlist_songs);
  void SendAllPlaylists();
  void SendAllActivePlaylists();
  void SendPlaylistSongs(int id);
  void Open(int id);
  void Close(int id);
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
  void SetActivePlaylist(int id);
  void ShuffleCurrent();
  void SetRepeatMode(PlaylistSequence::RepeatMode mode);
  void SetShuffleMode(PlaylistSequence::ShuffleMode mode);
  void InsertUrls(int id, const QList<QUrl>& urls, int pos, bool play_now, bool enqueue);
  void RemoveSongs(int id, const QList<int>& indices);
  void SeekTo(int seconds);
  void SendSongs(const pb::remote::RequestDownloadSongs& request, RemoteClient* client);
  void ResponseSongOffer(RemoteClient* client, bool accepted);
  void SendLibrary(RemoteClient* client);
  void RateCurrentSong(double);

private:
  Application* app_;
  bool close_connection_;

  void GetPlaylistSongs(const pb::remote::Message& msg);
  void ChangeSong(const pb::remote::Message& msg);
  void SetRepeatMode(const pb::remote::Repeat& repeat);
  void SetShuffleMode(const pb::remote::Shuffle& shuffle);
  void InsertUrls(const pb::remote::Message& msg);
  void RemoveSongs(const pb::remote::Message& msg);
  void ClientConnect(const pb::remote::Message& msg);
  void SendPlaylists(const pb::remote::Message& msg);
  void OpenPlaylist(const pb::remote::Message& msg);
  void ClosePlaylist(const pb::remote::Message& msg);
  void RateSong(const pb::remote::Message& msg);
};

#endif // INCOMINGDATAPARSER_H
