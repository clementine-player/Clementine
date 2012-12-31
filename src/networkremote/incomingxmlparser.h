#ifndef INCOMINGXMLPARSER_H
#define PARSEINCOMINGXML_H

#include <QDomDocument>

#include "core/player.h"
#include "core/application.h"

class IncomingXmlParser : public QObject {
  Q_OBJECT
public:
  IncomingXmlParser(Application* app);
  ~IncomingXmlParser();

  void Parse(QString* xml_data);
  bool close_connection();

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

private:
  Application* app_;
  bool close_connection_;

  void ChangeVolume(QDomNode& child);
  void GetPlaylistSongs(QDomNode& child);
  void ChangeSong(QDomNode& child);
};

#endif // PARSEINCOMINGXML_H
