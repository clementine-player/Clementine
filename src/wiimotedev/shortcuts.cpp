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

#include <QSettings>

#include "wiimotedev/consts.h"
#include "wiimotedev/shortcuts.h"

const char* WiimotedevShortcuts::kActionsGroup = "WiimotedevActions";
const char* WiimotedevShortcuts::kSettingsGroup = "WiimotedevSettings";

WiimotedevShortcuts::WiimotedevShortcuts(OSD* osd, QWidget* window, QObject* parent)
 :QObject(parent),
  osd_(osd),
  main_window_(window),
  player_(qobject_cast<Player*>(parent)),
  low_battery_notification_(true),
  critical_battery_notification_(true),
  actived_(false),
  wiimotedev_active_(true),
  wiimotedev_buttons_(0),
  wiimotedev_device_(1),
  wiimotedev_enable_(true),
  wiimotedev_focus_(false),
  wiimotedev_notification_(true)
{
  connect(this, SIGNAL(WiiremoteActived(int)), osd_, SLOT(WiiremoteActived(int)));
  connect(this, SIGNAL(WiiremoteDeactived(int)), osd_, SLOT(WiiremoteDeactived(int)));
  connect(this, SIGNAL(WiiremoteConnected(int)), osd_, SLOT(WiiremoteConnected(int)));
  connect(this, SIGNAL(WiiremoteDisconnected(int)), osd_, SLOT(WiiremoteDisconnected(int)));
  connect(this, SIGNAL(WiiremoteLowBattery(int,int)), osd_, SLOT(WiiremoteLowBattery(int,int)));
  connect(this, SIGNAL(WiiremoteCriticalBattery(int,int)), osd_, SLOT(WiiremoteCriticalBattery(int,int)));

  ReloadSettings();
}

void WiimotedevShortcuts::SetWiimotedevInterfaceActived(bool actived) {
  if (!QDBusConnection::systemBus().isConnected())
    return;

  // http://code.google.com/p/clementine-player/issues/detail?id=670
  // Probably dbus bug, or something else

  if (actived && !wiimotedev_iface_) {
    wiimotedev_iface_.reset(new OrgWiimotedevDeviceEventsInterface(
        WIIMOTEDEV_DBUS_SERVICE_NAME, WIIMOTEDEV_DBUS_EVENTS_OBJECT,
        QDBusConnection::systemBus(), this));

    connect(wiimotedev_iface_.get(), SIGNAL(dbusWiimoteGeneralButtons(uint,qulonglong)),
            this, SLOT(DbusWiimoteGeneralButtons(uint,qulonglong)));
    connect(wiimotedev_iface_.get(), SIGNAL(dbusWiimoteConnected(uint)), this,
            SLOT(DbusWiimoteConnected(uint)));
    connect(wiimotedev_iface_.get(), SIGNAL(dbusWiimoteDisconnected(uint)), this,
            SLOT(DbusWiimoteDisconnected(uint)));
    connect(wiimotedev_iface_.get(), SIGNAL(dbusWiimoteBatteryLife(uint,uchar)), this,
            SLOT(DbusWiimoteBatteryLife(uint,uchar)));

    if (!wiimotedev_iface_.get()->isValid())
      qWarning("Error connecting to the Wiimotedev-daemon DBUS service");
  }

  if (!actived && wiimotedev_iface_)
    wiimotedev_iface_.reset();
}

void WiimotedevShortcuts::ReloadSettings() {
  settings_.sync();
  actions_.clear();

  settings_.beginGroup(WiimotedevShortcuts::kActionsGroup);

  quint64 fvalue, svalue;
  bool fvalid, svalid;

  foreach (const QString& str, settings_.allKeys()) {
    fvalue = str.toULongLong(&fvalid, 10);
    svalue = settings_.value(str, 0).toULongLong(&svalid);
    if (fvalid && svalid)
      actions_[fvalue] = svalue;
  }

  settings_.endGroup();

  settings_.beginGroup(WiimotedevShortcuts::kSettingsGroup);
  wiimotedev_enable_ = settings_.value("enabled", false).toBool();
  wiimotedev_device_ = settings_.value("device", 1).toInt();
  wiimotedev_active_ = settings_.value("use_active_action", true).toBool();
  wiimotedev_focus_ = settings_.value("only_when_focused", false).toBool();
  wiimotedev_notification_ = settings_.value("use_notification", true).toBool();
  settings_.endGroup();

  SetWiimotedevInterfaceActived(wiimotedev_enable_);
}

void WiimotedevShortcuts::DbusWiimoteGeneralButtons(uint id, qulonglong value) {
  if (id != wiimotedev_device_ || !wiimotedev_enable_ || !player_) return;

  if (wiimotedev_focus_ && !main_window_->isActiveWindow())
    return;

  quint64 buttons = value & ~(
      WIIMOTE_TILT_MASK |
      NUNCHUK_TILT_MASK |
      WIIMOTE_BTN_SHIFT_SHAKE |
      NUNCHUK_BTN_SHIFT_SHAKE);

  if (wiimotedev_buttons_ == buttons) return;

  if (actions_.contains(buttons)) {

    if (wiimotedev_active_) {
      if (actions_.value(buttons, ActionNone) == WiimotedevActiveDeactive) {
        actived_ = !actived_;
        if (wiimotedev_notification_) {
          if (actived_)
            emit WiiremoteActived(id); else
            emit WiiremoteDeactived(id);
        }
      }
    }

    if (actived_ || !wiimotedev_active_) {
      switch (actions_.value(buttons, ActionNone)) {
        case PlayerNextTrack: player_->Next(); break;
        case PlayerPreviousTrack: player_->Previous(); break;
        case PlayerPlay: player_->Play(); break;
        case PlayerStop: player_->Stop(); break;
        case PlayerIncVolume: player_->VolumeUp(); break;
        case PlayerDecVolume: player_->VolumeDown(); break;
        case PlayerMute: player_->Mute(); break;
        case PlayerPause: player_->Pause(); break;
        case PlayerTogglePause: player_->PlayPause(); break;
        case PlayerSeekBackward: player_->SeekBackward(); break;
        case PlayerSeekForward: player_->SeekForward(); break;
        case PlayerStopAfter: player_->Stop(); break;
        case PlayerShowOSD: player_->ShowOSD(); break;
      }
    }
  }

  wiimotedev_buttons_ = buttons;
}

void WiimotedevShortcuts::DbusWiimoteConnected(uint id) {
  if (wiimotedev_device_ == id && wiimotedev_notification_)
    emit WiiremoteConnected(id);
}

void WiimotedevShortcuts::DbusWiimoteDisconnected(uint id) {
  if (wiimotedev_device_ == id && wiimotedev_notification_)
    emit WiiremoteDisconnected(id);
}

void WiimotedevShortcuts::DbusWiimoteBatteryLife(uint id, uchar life) {
  if (wiimotedev_device_ == id && wiimotedev_notification_) {
    if (life <= 30 && life > 20 && low_battery_notification_) {
      low_battery_notification_ = false;
      emit WiiremoteLowBattery(id, life);
    }

    if (life <= 20 && critical_battery_notification_) {
      critical_battery_notification_ = false;
      low_battery_notification_ = false;
      emit WiiremoteCriticalBattery(id, life);
    }
  }
}


