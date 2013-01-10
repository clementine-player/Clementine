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
  void SendClementineInfos();
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

private:
  Application* app_;
  bool close_connection_;

  void GetPlaylistSongs(pb::remote::Message* msg);
  void ChangeSong(pb::remote::Message* msg);
};

#endif // INCOMINGDATAPARSER_H
