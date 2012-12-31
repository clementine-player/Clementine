#ifndef OUTGOINGXMLCREATOR_H
#define OUTGOINGXMLCREATOR_H

#include <QDomDocument>
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

class OutgoingXmlCreator : public QObject {
    Q_OBJECT
public:
  OutgoingXmlCreator(Application* app);
  ~OutgoingXmlCreator();

  void SetClients(QList<QTcpSocket*>* clients);

public slots:
  void SendClementineInfos();
  void SendAllPlaylists();
  void SendFirstData();
  void SendPlaylistSongs(int id);
  void VolumeChanged(int volume);
  void ActiveChanged(Playlist*);
  void CurrentSongChanged(const Song& song, const QString& uri, const QImage& img);
  void StateChanged(Engine::State);
  void SendKeepAlive();

private:
  Application* app_;
  QList<QTcpSocket*>* clients_;
  Song current_song_;
  QString current_uri_;
  QImage current_image_;
  Engine::State last_state_;
  QTimer* keep_alive_timer_;
  int keep_alive_timeout_;

  void SendDataToClients(QByteArray data);
  void CreateXmlHeader(QDomDocument* doc, QString action);
  QDomElement CreateSong(QDomDocument* doc, Song* song, const QString* art_uri, int index);
  QDomElement CreateSongTag(QDomDocument* doc, QString tag, QString text);
};

#endif // OUTGOINGXMLCREATOR_H
