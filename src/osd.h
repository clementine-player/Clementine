/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OSD_H
#define OSD_H

#include <QImage>
#include <QObject>
#include <QSystemTrayIcon>

#include "engine_fwd.h"
#include "song.h"

#ifdef Q_WS_X11
#include <QDBusArgument>
#include <boost/scoped_ptr.hpp>
#include "notification.h"

QDBusArgument& operator<< (QDBusArgument& arg, const QImage& image);
const QDBusArgument& operator>> (const QDBusArgument& arg, QImage& image);

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
  bool show_on_volume_change_;
  bool show_art_;

#ifdef Q_OS_DARWIN
  class GrowlNotificationWrapper;
  GrowlNotificationWrapper* wrapper_;
#endif  // Q_OS_DARWIN

#ifdef Q_WS_X11
  boost::scoped_ptr<org::freedesktop::Notifications> interface_;
 private slots:
  void CallFinished(QDBusPendingCallWatcher* watcher);
#endif
};

#endif // OSD_H
