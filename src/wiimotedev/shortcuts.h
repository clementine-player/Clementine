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

#ifndef WIIMOTEDEV_SHORTCUTS_H
#define WIIMOTEDEV_SHORTCUTS_H

#include <QObject>
#include <QMap>
#include "core/player.h"

class DBusDeviceEventsInterface;
class QSettings;

class WiimotedevShortcuts :public QObject {
  Q_OBJECT
public:
  static const char* kActionsGroup;
  static const char* kSettingsGroup;

  WiimotedevShortcuts(Player* player = 0, QObject* parent = 0);

public slots:
  void ReloadSettings();
  void SetDefaultSettings();

private slots:
  void DbusWiimoteGeneralButtons(quint32 id, quint64 value);

private:
  DBusDeviceEventsInterface* wiimotedev_iface_;
  Player* player_;
  quint64 wiimotedev_buttons_;
  QSettings settings_;

  quint32 wiimotedev_device_; 
  bool wiimotedev_active_;
  bool wiimotedev_enable_;
  bool wiimotedev_focus_;
  bool wiimotedev_notification_;

public:
  enum Action {
    WiimotedevActive = 0,
    WiimotedevDeactive,
    PlayerNextTrack,
    PlayerPreviousTrack,
    PlayerPlay,
    PlayerStop,
    PlayerIncVolume,
    PlayerDecVolume,
    PlayerMute,
    PlayerPause,
    PlayerTogglePause,
    PlayerSeekBackward,
    PlayerSeekForward,
    PlayerStopAfter,
    PlayerShowOSD,
    ActionNone  = 0xff
  };

private:
  QHash <quint64, quint32> actions_;

};

#endif // WIIMOTEDEV_SHORTCUTS_H
