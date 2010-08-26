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

#include "ui/wiimotedevshortcutsconfig.h"
#include "ui_wiimotedevshortcutsconfig.h"

#include "wiimotedev/shortcuts.h"
#include "wiimotedev/consts.h"

WiimotedevShortcutsConfig::WiimotedevShortcutsConfig(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_WiimotedevShortcutsConfig)
{
  ui_->setupUi(this);
  ui_->list->header()->setResizeMode(QHeaderView::ResizeToContents);

  text_buttons_.insert(WIIMOTE_BTN_1, "wiiremote.1");
  text_buttons_.insert(WIIMOTE_BTN_2, "wiiremote.2");
  text_buttons_.insert(WIIMOTE_BTN_A, "wiiremote.a");
  text_buttons_.insert(WIIMOTE_BTN_B, "wiiremote.b");
  text_buttons_.insert(WIIMOTE_BTN_PLUS, "wiiremote.plus");
  text_buttons_.insert(WIIMOTE_BTN_MINUS, "wiiremote.minus");
  text_buttons_.insert(WIIMOTE_BTN_HOME, "wiiremote.home");
  text_buttons_.insert(WIIMOTE_BTN_UP, "wiiremote.up");
  text_buttons_.insert(WIIMOTE_BTN_DOWN, "wiiremote.down");
  text_buttons_.insert(WIIMOTE_BTN_LEFT, "wiiremote.left");
  text_buttons_.insert(WIIMOTE_BTN_RIGHT, "wiiremote.right");
  text_buttons_.insert(WIIMOTE_BTN_SHIFT_UP, "wiiremote.shift.up");
  text_buttons_.insert(WIIMOTE_BTN_SHIFT_DOWN, "wiiremote.shift.down");
  text_buttons_.insert(WIIMOTE_BTN_SHIFT_LEFT, "wiiremote.shift.left");
  text_buttons_.insert(WIIMOTE_BTN_SHIFT_RIGHT, "wiiremote.shift.right");
  text_buttons_.insert(WIIMOTE_BTN_SHIFT_FORWARD, "wiiremote.shift.forward");
  text_buttons_.insert(WIIMOTE_BTN_SHIFT_BACKWARD, "wiiremote.shift.backward");
  text_buttons_.insert(WIIMOTE_BTN_TILT_FRONT, "wiiremote.tilt.front");
  text_buttons_.insert(WIIMOTE_BTN_TILT_BACK, "wiiremote.tilt.back");
  text_buttons_.insert(WIIMOTE_BTN_TILT_LEFT, "wiiremote.tilt.left");
  text_buttons_.insert(WIIMOTE_BTN_TILT_RIGHT, "wiiremote.tilt.right");
  text_buttons_.insert(NUNCHUK_BTN_Z, "nunchuk.z");
  text_buttons_.insert(NUNCHUK_BTN_C, "nunchuk.c");
  text_buttons_.insert(NUNCHUK_BTN_STICK_UP, "nunchuk.stick.up");
  text_buttons_.insert(NUNCHUK_BTN_STICK_DOWN, "nunchuk.stick.down");
  text_buttons_.insert(NUNCHUK_BTN_STICK_LEFT, "nunchuk.stick.left");
  text_buttons_.insert(NUNCHUK_BTN_STICK_RIGHT, "nunchuk.stick.right");
  text_buttons_.insert(NUNCHUK_BTN_SHIFT_UP, "nunchuk.shift.up");
  text_buttons_.insert(NUNCHUK_BTN_SHIFT_DOWN, "nunchuk.shift.down");
  text_buttons_.insert(NUNCHUK_BTN_SHIFT_LEFT, "nunchuk.shift.left");
  text_buttons_.insert(NUNCHUK_BTN_SHIFT_RIGHT, "nunchuk.shift.right");
  text_buttons_.insert(NUNCHUK_BTN_SHIFT_FORWARD, "nunchuk.shift.forward");
  text_buttons_.insert(NUNCHUK_BTN_SHIFT_BACKWARD, "nunchuk.shift.backward");
  text_buttons_.insert(NUNCHUK_BTN_TILT_FRONT, "nunchuk.tilt.front");
  text_buttons_.insert(NUNCHUK_BTN_TILT_BACK, "nunchuk.tilt.back");
  text_buttons_.insert(NUNCHUK_BTN_TILT_LEFT, "nunchuk.tilt.left");
  text_buttons_.insert(NUNCHUK_BTN_TILT_RIGHT, "nunchuk.tilt.right");
  text_buttons_.insert(CLASSIC_BTN_X, "classic.x");
  text_buttons_.insert(CLASSIC_BTN_Y, "classic.y");
  text_buttons_.insert(CLASSIC_BTN_A, "classic.a");
  text_buttons_.insert(CLASSIC_BTN_B, "classic.b");
  text_buttons_.insert(CLASSIC_BTN_L, "classic.l");
  text_buttons_.insert(CLASSIC_BTN_R, "classic.r");
  text_buttons_.insert(CLASSIC_BTN_ZL, "classic.zl");
  text_buttons_.insert(CLASSIC_BTN_ZR, "classic.zr");
  text_buttons_.insert(CLASSIC_BTN_MINUS, "classic.minus");
  text_buttons_.insert(CLASSIC_BTN_PLUS, "classic.plus");
  text_buttons_.insert(CLASSIC_BTN_HOME, "classic.home");
  text_buttons_.insert(CLASSIC_BTN_UP, "classic.up");
  text_buttons_.insert(CLASSIC_BTN_DOWN, "classic.down");
  text_buttons_.insert(CLASSIC_BTN_LEFT, "classic.left");
  text_buttons_.insert(CLASSIC_BTN_RIGHT, "classic.right");
  text_buttons_.insert(CLASSIC_BTN_LSTICK_UP, "classic.lstick.up");
  text_buttons_.insert(CLASSIC_BTN_LSTICK_DOWN, "classic.lstick.down");
  text_buttons_.insert(CLASSIC_BTN_LSTICK_LEFT, "classic.lstick.left");
  text_buttons_.insert(CLASSIC_BTN_LSTICK_RIGHT, "classic.lstick.right");
  text_buttons_.insert(CLASSIC_BTN_RSTICK_UP, "classic.rstick.up");
  text_buttons_.insert(CLASSIC_BTN_RSTICK_DOWN, "classic.rstick.down");
  text_buttons_.insert(CLASSIC_BTN_RSTICK_LEFT, "classic.rstick.left");
  text_buttons_.insert(CLASSIC_BTN_RSTICK_RIGHT, "classic.rstick.right");
  text_buttons_.insert(WIIMOTE_BTN_SHIFT_SHAKE, "wiiremote.shift.shake");
  text_buttons_.insert(NUNCHUK_BTN_SHIFT_SHAKE, "nunchuk.shift.shake");

  text_actions_.insert(WiimotedevShortcuts::PlayerPlay, tr("Play"));
  text_actions_.insert(WiimotedevShortcuts::PlayerStop, tr("Stop"));
  text_actions_.insert(WiimotedevShortcuts::PlayerNextTrack, tr("Next track"));
  text_actions_.insert(WiimotedevShortcuts::PlayerPreviousTrack, tr("Previous track"));
  text_actions_.insert(WiimotedevShortcuts::PlayerMute, tr("Mute"));
  text_actions_.insert(WiimotedevShortcuts::PlayerPause, tr("Pause"));
  text_actions_.insert(WiimotedevShortcuts::PlayerTogglePause, tr("Play/Pause"));
  text_actions_.insert(WiimotedevShortcuts::PlayerSeekBackward, tr("Seek backward"));
  text_actions_.insert(WiimotedevShortcuts::PlayerSeekForward, tr("Seek forward"));
  text_actions_.insert(WiimotedevShortcuts::PlayerIncVolume, tr("Increase volume"));
  text_actions_.insert(WiimotedevShortcuts::PlayerDecVolume, tr("Decrease volume"));
  text_actions_.insert(WiimotedevShortcuts::PlayerShowOSD, tr("Show OSD"));


  settings_.beginGroup(WiimotedevShortcuts::kSettingsGroup);
  ui_->wiimotedev_enable->setChecked(settings_.value("enabled", true).toBool());
  ui_->wiimotedev_groupbox->setChecked(settings_.value("enabled", true).toBool());
  ui_->wiimotedev_active->setChecked(settings_.value("use_active_action", false).toBool());
  ui_->wiimotedev_focus->setChecked(settings_.value("only_when_focused", false).toBool());
  ui_->wiimotedev_notification->setChecked(settings_.value("use_notification", false).toBool());
  ui_->wiimotedev_device->setValue(settings_.value("device", 1).toUInt());
  settings_.endGroup();

  LoadSettings();

  connect(ui_->wiimotedev_enable, SIGNAL(clicked(bool)), this, SLOT(WiimotedevEnabledChecked(bool)));
  connect(ui_->list, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(ItemClicked(QTreeWidgetItem*)));
  connect(ui_->wiimotedev_delete_action, SIGNAL(clicked()), this, SLOT(DeleteAction()));
  connect(ui_->wiimotedev_reload, SIGNAL(clicked()), this , SLOT(DefaultSettings()));
}

void WiimotedevShortcutsConfig::LoadSettings(){
  settings_.beginGroup(WiimotedevShortcuts::kActionsGroup);
  ui_->list->clear();

  quint64 fvalue, svalue;
  bool fvalid, svalid;

  foreach (const QString& str, settings_.allKeys()) {
    fvalue = str.toULongLong(&fvalid, 10);
    svalue = settings_.value(str, 0).toULongLong(&svalid);
    if (fvalid && svalid) {
      Shortcut s;
      s.object = new QTreeWidgetItem(ui_->list, QStringList() << GetReadableWiiremoteSequence(fvalue) << text_actions_[svalue]);
      s.button = fvalue;
      s.action = svalue;
      actions_ << s;
    }
    ui_->list->sortItems(1, Qt::AscendingOrder);
  }

  settings_.endGroup();
}

void WiimotedevShortcutsConfig::DefaultSettings()
{
  settings_.beginGroup(WiimotedevShortcuts::kActionsGroup);
  foreach (const QString& key, settings_.allKeys()) {
    settings_.remove(key);
  }
  settings_.setValue(QString::number(WIIMOTE_BTN_LEFT), WiimotedevShortcuts::PlayerPreviousTrack);
  settings_.setValue(QString::number(WIIMOTE_BTN_RIGHT), WiimotedevShortcuts::PlayerNextTrack);
  settings_.setValue(QString::number(WIIMOTE_BTN_SHIFT_LEFT), WiimotedevShortcuts::PlayerPreviousTrack);
  settings_.setValue(QString::number(WIIMOTE_BTN_SHIFT_RIGHT), WiimotedevShortcuts::PlayerNextTrack);
  settings_.setValue(QString::number(WIIMOTE_BTN_PLUS), WiimotedevShortcuts::PlayerIncVolume);
  settings_.setValue(QString::number(WIIMOTE_BTN_MINUS), WiimotedevShortcuts::PlayerDecVolume);
  settings_.setValue(QString::number(WIIMOTE_BTN_1), WiimotedevShortcuts::PlayerTogglePause);
  settings_.setValue(QString::number(WIIMOTE_BTN_2), WiimotedevShortcuts::PlayerShowOSD);
  settings_.endGroup();
  settings_.sync();

  LoadSettings();
}

void WiimotedevShortcutsConfig::WiimotedevEnabledChecked(bool checked) {
  ui_->wiimotedev_groupbox->setEnabled(checked);
}

void WiimotedevShortcutsConfig::ItemClicked(QTreeWidgetItem* selected_item) {
  selected_item_ = selected_item;
}

QString WiimotedevShortcutsConfig::GetReadableWiiremoteSequence(quint64 value) {
  QMapIterator<quint64, QString> text(text_buttons_);

  QStringList list;
  while (text.hasNext()) {
    text.next();
    if ((text.key() & value) == text.key())
      list << text.value();
  }

  QString output;
  for (int i = 0; i < (list.count() - 1); ++i)
    output += list.at(i) + " + ";
  output += list.last();

  return output;
}

void WiimotedevShortcutsConfig::DeleteAction() {
  for (int i = 0; i < actions_.count(); ++i) {
    if (actions_.at(i).object == selected_item_) {
      delete selected_item_;
      actions_.removeAt(i);
      break;
    }
  }
}
