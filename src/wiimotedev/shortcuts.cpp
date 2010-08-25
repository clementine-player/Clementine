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

const char* WiimotedevShortcuts::kSettingsGroup = "Wiimotedev";

WiimotedevShortcuts::WiimotedevShortcuts(Player *player, QObject *parent)
 :QObject(parent),
  player_(player),
  wiimotedev_iface_(NULL),
  wiimotedev_buttons_(0)
{
  settings_.beginGroup(WiimotedevShortcuts::kSettingsGroup);
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
  if (!enabled) {
    disconnect(this, 0, player_, 0);
    return;
  }

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
  settings_.sync();
  quint64 value;
  if (value = settings_.value(QString::fromUtf8("next_track"), quint64(WIIMOTE_BTN_RIGHT)).toInt()) actions_[PlayerNextTrack] = value;
  if (value = settings_.value(QString::fromUtf8("previous_track"), quint64(WIIMOTE_BTN_LEFT)).toInt()) actions_[PlayerPreviousTrack] = value;
  if (value = settings_.value(QString::fromUtf8("play"), quint64(0)).toInt()) actions_[PlayerPlay] = value;
  if (value = settings_.value(QString::fromUtf8("stop"), quint64(0)).toInt()) actions_[PlayerStop] = value;
  if (value = settings_.value(QString::fromUtf8("inc_volume"), quint64(WIIMOTE_BTN_PLUS)).toInt()) actions_[PlayerIncVolume] = value;
  if (value = settings_.value(QString::fromUtf8("dec_volume"), quint64(WIIMOTE_BTN_MINUS)).toInt()) actions_[PlayerDecVolume] = value;
  if (value = settings_.value(QString::fromUtf8("mute"), quint64(0)).toInt()) actions_[PlayerMute] = value;
  if (value = settings_.value(QString::fromUtf8("pause"), quint64(0)).toInt()) actions_[PlayerPause] = value;
  if (value = settings_.value(QString::fromUtf8("togglepause"), quint64(WIIMOTE_BTN_1)).toInt()) actions_[PlayerTogglePause] = value;
  if (value = settings_.value(QString::fromUtf8("seek_backward"), quint64(WIIMOTE_BTN_UP)).toInt()) actions_[PlayerSeekBackward] = value;
  if (value = settings_.value(QString::fromUtf8("seek_forward"), quint64(WIIMOTE_BTN_DOWN)).toInt()) actions_[PlayerSeekForward] = value;
  if (value = settings_.value(QString::fromUtf8("show_osd"), quint64(WIIMOTE_BTN_2)).toInt()) actions_[PlayerShowOSD] = value;

  SetEnabled(settings_.value(QString::fromUtf8("enabled"), quint64(true)).toBool());
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
  if (id != 1) return;

  quint64 buttons = value & ~(WIIMOTE_TILT_MASK | NUNCHUK_TILT_MASK);

  if (wiimotedev_buttons_ == buttons) return;

  QMapIterator< quint32, quint64> actions(actions_);

  while (actions.hasNext()) {
    actions.next();
    if (actions.value() == 0) continue;
    if ((actions.value() & buttons) == actions.value()) EmitRequest(actions.key());
  }

  wiimotedev_buttons_ = buttons;
}
