#ifndef INCOMINGDATAPARSER_H
#define INCOMINGDATAPARSER_H

#include "core/player.h"
#include "core/application.h"
#include "remotecontrolmessages.pb.h"

class IncomingDataParser : public QObject {
  Q_OBJECT
public:
  IncomingDataParser(Application* app);
  ~IncomingDataParser();

  bool close_connection();

public slots:
  void Parse(const QByteArray& pb_data);

signals:
  void SendClementineInfo();
  void SendFirstData();
  void SendAllPlaylists();
  void SendPlaylistSongs(int id);

  void Play();
  void PlayPause();
  void Pause();
  void Stop();
  void Next();
  void Previous();
  void SetVolume(int volume);
  void PlayAt(int i, Engine::TrackChangeFlags change, bool reshuffle);
  void SetActivePlaylist(int id);
  void ShuffleCurrent();
  void SetRepeatMode(PlaylistSequence::RepeatMode mode);
  void SetShuffleMode(PlaylistSequence::ShuffleMode mode);

private slots:
  void PlaylistManagerInitialized();

private:
  Application* app_;
  bool close_connection_;

  void GetPlaylistSongs(const pb::remote::Message& msg);
  void ChangeSong(const pb::remote::Message& msg);
  void SetRepeatMode(const pb::remote::Repeat& repeat);
  void SetShuffleMode(const pb::remote::Shuffle& shuffle);
};

#endif // INCOMINGDATAPARSER_H
