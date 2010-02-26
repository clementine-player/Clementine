#ifndef OSD_H
#define OSD_H

#include <QImage>
#include <QObject>
#include <QSystemTrayIcon>

#include "engine_fwd.h"
#include "song.h"

#ifdef Q_WS_X11
# ifndef _NOTIFY_NOTIFICATION_H_
    struct GdkPixbuf;
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

  // Implemented in the OS-specific files
  static bool SupportsNativeNotifications();
  static bool SupportsTrayPopups();

 public slots:
  void ReloadSettings();

  void SongChanged(const Song& song);
  void Paused();
  void Stopped();
  void VolumeChanged(int value);

 private:
  void ShowMessage(const QString& summary,
                   const QString& message = QString(),
                   const QString& icon = QString(),
                   const QImage& image = QImage());

  // These are implemented in the OS-specific files
  void Init();
  void ShowMessageNative(const QString& summary,
                         const QString& message = QString(),
                         const QString& icon = QString());
  void ShowMessageNative(const QString& summary,
                         const QString& message,
                         const QImage& image);

 private:
  QSystemTrayIcon* tray_icon_;
  int timeout_;
  Behaviour behaviour_;

#ifdef Q_WS_X11
  NotifyNotification* notification_;
  GdkPixbuf* pixbuf_;
#endif

#ifdef Q_OS_DARWIN
  class GrowlNotificationWrapper;
  GrowlNotificationWrapper* wrapper_;
#endif  // Q_OS_DARWIN
};

#endif // OSD_H
