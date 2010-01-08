#ifndef OSD_H
#define OSD_H

#include <QObject>
#include <QSystemTrayIcon>

#include "engine_fwd.h"
#include "song.h"

class OSD : public QObject {
  Q_OBJECT

 public:
  OSD(QSystemTrayIcon* tray_icon, QObject* parent = 0);

  void Init();
  void ShowMessage(const QString& summary,
                   const QString& message = QString::null,
                   const QString& icon = QString::null);

 public slots:
  void SongChanged(const Song& song);
  void Paused();
  void Stopped();

 private:
  QSystemTrayIcon* tray_icon_;
};

#endif // OSD_H
