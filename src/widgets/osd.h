/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include <memory>

#include <QDateTime>
#include <QImage>
#include <QObject>

#include "config.h"
#include "engines/engine_fwd.h"
#include "core/song.h"
#include "playlist/playlistsequence.h"

class Application;
class OrgFreedesktopNotificationsInterface;
class OSDPretty;
class SystemTrayIcon;

class QDBusPendingCallWatcher;

#ifdef HAVE_DBUS
#include <QDBusArgument>
#include <QDBusPendingCall>

QDBusArgument& operator<<(QDBusArgument& arg, const QImage& image);
const QDBusArgument& operator>>(const QDBusArgument& arg, QImage& image);
#endif

class OSD : public QObject {
  Q_OBJECT

 public:
  OSD(SystemTrayIcon* tray_icon, Application* app, QObject* parent = nullptr);
  ~OSD();

  static const char* kSettingsGroup;

  enum Behaviour {
    Disabled = 0,
    Native,
    TrayPopup,
    Pretty,
  };

  // Implemented in the OS-specific files
  static bool SupportsNativeNotifications();
  static bool SupportsTrayPopups();

  void ReloadPrettyOSDSettings();

  void SetPrettyOSDToggleMode(bool toggle);

 public slots:
  void ReloadSettings();

  void Paused();
  void Stopped();
  void StopAfterToggle(bool stop);
  void PlaylistFinished();
  void VolumeChanged(int value);
  void MagnatuneDownloadFinished(const QStringList& albums);
  void RepeatModeChanged(PlaylistSequence::RepeatMode mode);
  void ShuffleModeChanged(PlaylistSequence::ShuffleMode mode);

  void ReshowCurrentSong();

#ifdef HAVE_WIIMOTEDEV
  void WiiremoteActived(int id);
  void WiiremoteDeactived(int id);
  void WiiremoteConnected(int id);
  void WiiremoteDisconnected(int id);
  void WiiremoteLowBattery(int id, int live);
  void WiiremoteCriticalBattery(int id, int live);
#endif

  void ShowPreview(const Behaviour type, const QString& line1,
                   const QString& line2, const Song& song);

 private:
  void ShowMessage(const QString& summary, const QString& message = QString(),
                   const QString& icon = QString(),
                   const QImage& image = QImage());

  // These are implemented in the OS-specific files
  void Init();
  void ShowMessageNative(const QString& summary, const QString& message,
                         const QString& icon = QString(),
                         const QImage& image = QImage());
  QString ReplaceVariable(const QString& variable, const Song& song);

 private slots:
#if defined(HAVE_DBUS)
  void CallFinished(QDBusPendingCallWatcher* watcher);
#endif
  void AlbumArtLoaded(const Song& song, const QString& uri,
                      const QImage& image);

 private:
  SystemTrayIcon* tray_icon_;
  Application* app_;
  int timeout_msec_;
  Behaviour behaviour_;
  bool show_on_volume_change_;
  bool show_art_;
  bool show_on_play_mode_change_;
  bool show_on_pause_;
  bool use_custom_text_;
  QString custom_text1_;
  QString custom_text2_;
  bool preview_mode_;

  bool force_show_next_;
  bool ignore_next_stopped_;

  OSDPretty* pretty_popup_;

  Song last_song_;
  QString last_image_uri_;
  QImage last_image_;

#ifdef HAVE_DBUS
  std::unique_ptr<OrgFreedesktopNotificationsInterface> interface_;
  uint notification_id_;
  QDateTime last_notification_time_;
#endif
};

#endif  // OSD_H
