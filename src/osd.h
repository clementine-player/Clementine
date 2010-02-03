#ifndef OSD_H
#define OSD_H

#include <QObject>
#include <QSystemTrayIcon>

#include "engine_fwd.h"
#include "song.h"

#ifdef Q_WS_X11
# ifndef _NOTIFY_NOTIFICATION_H_
    struct NotifyNotification;
# endif
#endif

class OSD : public QObject {
  Q_OBJECT

 public:
  OSD(QSystemTrayIcon* tray_icon, QObject* parent = 0);

  static const char* kSettingsGroup;

  enum Behaviour {
    Disabled = 0,
    Native,
    TrayPopup,
  };

 public slots:
  void ReloadSettings();

  void SongChanged(const Song& song);
  void Paused();
  void Stopped();
  void VolumeChanged(int value);

 private:
  void ShowMessage(const QString& summary,
                   const QString& message = QString::null,
                   const QString& icon = QString::null);

  // These are implemented in the OS-specific files
  void Init();
  bool CanShowNativeMessages() const;
  void ShowMessageNative(const QString& summary,
                         const QString& message = QString::null,
                         const QString& icon = QString::null);

 private:
  QSystemTrayIcon* tray_icon_;
  int timeout_;
  Behaviour behaviour_;

#ifdef Q_WS_X11
  NotifyNotification* notification_;
#endif
};

#endif // OSD_H
