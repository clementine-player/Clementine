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

#ifndef WIIMOTEDEV_SHORTCUTS_H
#define WIIMOTEDEV_SHORTCUTS_H

#include <memory>

#include <QWidget>

#include "dbus/wiimotedev.h"
#include "core/player.h"
#include "widgets/osd.h"

class QSettings;

class WiimotedevShortcuts :public QObject {
  Q_OBJECT
public:
  static const char* kActionsGroup;
  static const char* kSettingsGroup;

  WiimotedevShortcuts(OSD* osd, QWidget* window, QObject* parent = 0);

  enum Action {
    WiimotedevActiveDeactive = 0,
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

public slots:
  void SetWiimotedevInterfaceActived(bool actived);
  void ReloadSettings();

private slots:
  void DbusWiimoteBatteryLife(uint id, uchar life);
  void DbusWiimoteConnected(uint id);
  void DbusWiimoteDisconnected(uint id);
  void DbusWiimoteGeneralButtons(uint id, qulonglong value);

private:
  OSD* osd_;
  QWidget* main_window_;
  Player* player_;

  bool low_battery_notification_;
  bool critical_battery_notification_;
  bool actived_;

  bool wiimotedev_active_;
  quint64 wiimotedev_buttons_;
  quint32 wiimotedev_device_;
  bool wiimotedev_enable_;
  bool wiimotedev_focus_;
  std::unique_ptr<OrgWiimotedevDeviceEventsInterface> wiimotedev_iface_;
  bool wiimotedev_notification_;

  QHash <quint64, quint32> actions_;
  QSettings settings_;

signals:
  void WiiremoteActived(int);
  void WiiremoteDeactived(int);
  void WiiremoteConnected(int);
  void WiiremoteDisconnected(int);
  void WiiremoteLowBattery(int, int);
  void WiiremoteCriticalBattery(int, int);
};

#endif // WIIMOTEDEV_SHORTCUTS_H
