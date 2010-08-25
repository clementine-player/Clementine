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

#include <QSettings>

#include "wiimotedev/shortcuts.h"
#include "wiimotedev/interface.h"

const char* WiimotedevShortcuts::kActionsGroup = "WiimotedevActions";
const char* WiimotedevShortcuts::kSettingsGroup = "WiimotedevSettings";

WiimotedevShortcuts::WiimotedevShortcuts(Player* player, QObject* parent)
 :QObject(parent),
  player_(player),
  wiimotedev_iface_(NULL),
  wiimotedev_buttons_(0),
  wiimotedev_device_(1),
  wiimotedev_active_(true),
  wiimotedev_focus_(false),
  wiimotedev_notification_(true)
{
  ReloadSettings();

  wiimotedev_iface_ = new DBusDeviceEventsInterface(WIIMOTEDEV_DBUS_SERVICE_NAME,
                                                    WIIMOTEDEV_DBUS_EVENTS_OBJECT,
                                                    QDBusConnection::systemBus(),
                                                    this);

  connect(wiimotedev_iface_, SIGNAL(dbusWiimoteGeneralButtons(quint32,quint64)),
          this, SLOT(DbusWiimoteGeneralButtons(quint32, quint64)));
}

void WiimotedevShortcuts::SetEnabled(bool enabled)
{
  disconnect(this, 0, player_, 0);
  if (!enabled) return;

  connect(this, SIGNAL(Next()), player_, SLOT(Next()));
  connect(this, SIGNAL(Previous()), player_, SLOT(Previous()));
  connect(this, SIGNAL(Play()), player_, SLOT(Play()));
  connect(this, SIGNAL(Stop()), player_, SLOT(Stop()));
  connect(this, SIGNAL(IncVolume()), player_, SLOT(VolumeUp()));
  connect(this, SIGNAL(DecVolume()), player_, SLOT(VolumeDown()));
  connect(this, SIGNAL(Mute()), player_, SLOT(Mute()));
  connect(this, SIGNAL(Pause()), player_, SLOT(Pause()));
  connect(this, SIGNAL(TogglePause()), player_, SLOT(PlayPause()));
  connect(this, SIGNAL(SeekBackward()), player_, SLOT(SeekBackward()));
  connect(this, SIGNAL(SeekForward()), player_, SLOT(SeekForward()));
  connect(this, SIGNAL(ShowOSD()), player_, SLOT(ShowOSD()));
}

void WiimotedevShortcuts::ReloadSettings() {
  settings_.beginGroup(WiimotedevShortcuts::kActionsGroup);

  if (!settings_.allKeys().count()) {
    SetDefaultSettings();
    settings_.sync();
  }

  actions_.clear();

  quint64 fvalue, svalue;
  bool fvalid, svalid;

  foreach (const QString& str, settings_.allKeys()) {
    fvalue = str.toULongLong(&fvalid, 10);
    svalue = settings_.value(str, 0).toULongLong(&svalid);
    if (fvalid && svalid) actions_[fvalue] = svalue;
  }

  settings_.endGroup();

  settings_.beginGroup(WiimotedevShortcuts::kSettingsGroup);
  SetEnabled(settings_.value("enabled", quint64(true)).toBool());
  wiimotedev_device_ = settings_.value("device", 1).toInt();
  wiimotedev_active_ = settings_.value("use_active_action", wiimotedev_active_).toBool();
  wiimotedev_focus_ = settings_.value("only_when_focused", wiimotedev_focus_).toBool();
  wiimotedev_notification_ = settings_.value("use_notification", wiimotedev_notification_).toBool();

  settings_.endGroup();
}

void WiimotedevShortcuts::SetDefaultSettings()
{
  QSettings settings;
  settings.beginGroup(WiimotedevShortcuts::kActionsGroup);
  settings.setValue(QString::number(WIIMOTE_BTN_LEFT), PlayerPreviousTrack);
  settings.setValue(QString::number(WIIMOTE_BTN_RIGHT), PlayerNextTrack);
  settings.setValue(QString::number(WIIMOTE_BTN_SHIFT_LEFT), PlayerPreviousTrack);
  settings.setValue(QString::number(WIIMOTE_BTN_SHIFT_RIGHT), PlayerNextTrack);
  settings.setValue(QString::number(WIIMOTE_BTN_PLUS), PlayerIncVolume);
  settings.setValue(QString::number(WIIMOTE_BTN_MINUS), PlayerDecVolume);
  settings.setValue(QString::number(WIIMOTE_BTN_1), PlayerTogglePause);
  settings.setValue(QString::number(WIIMOTE_BTN_2), PlayerShowOSD);
  settings.endGroup();

  settings.beginGroup(WiimotedevShortcuts::kSettingsGroup);
  settings.setValue("enabled", true);
  settings.setValue("device", 1);
  settings.setValue("use_active_action", true);
  settings.setValue("only_when_focused", false);
  settings.setValue("use_notification", true);
  settings.endGroup();

  settings.sync();
}

void WiimotedevShortcuts::EmitRequest(quint32 action) {
  switch (action) {
    case PlayerNextTrack: emit Next(); break;
    case PlayerPreviousTrack: emit Previous(); break;
    case PlayerPlay: emit Play(); break;
    case PlayerStop: emit Stop(); break;
    case PlayerIncVolume: emit IncVolume(); break;
    case PlayerDecVolume: emit DecVolume(); break;
    case PlayerMute: emit Mute(); break;
    case PlayerPause: emit Pause(); break;
    case PlayerTogglePause: emit TogglePause(); break;
    case PlayerSeekBackward: emit SeekBackward(); break;
    case PlayerSeekForward: emit SeekForward(); break;
    case PlayerStopAfter: emit StopAfter(); break;
    case PlayerShowOSD: emit ShowOSD(); break;
  }
}

void WiimotedevShortcuts::DbusWiimoteGeneralButtons(quint32 id, quint64 value) {
  if (id != wiimotedev_device_) return;

  quint64 buttons = value & ~(WIIMOTE_TILT_MASK | NUNCHUK_TILT_MASK);

  if (wiimotedev_buttons_ == buttons) return;

  QMapIterator<quint64, quint32> actions(actions_);

  while (actions.hasNext()) {
    actions.next();
    if (!actions.key()) continue;
    if ((actions.key() & buttons) == actions.key()) EmitRequest(actions.value());
  }

  wiimotedev_buttons_ = buttons;
}
