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

WiimotedevShortcutsConfig::WiimotedevShortcutsConfig(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_WiimotedevShortcutsConfig)
{
  ui_->setupUi(this);
  ui_->list->header()->setResizeMode(QHeaderView::ResizeToContents);

  settings_.beginGroup(WiimotedevShortcuts::kSettingsGroup);
  ui_->wiimotedev_enable->setChecked(settings_.value("enabled", true).toBool());
  ui_->wiimotedev_groupbox->setChecked(settings_.value("enabled", true).toBool());
  ui_->wiimotedev_active->setChecked(settings_.value("use_active_action", false).toBool());
  ui_->wiimotedev_focus->setChecked(settings_.value("only_when_focused", false).toBool());
  ui_->wiimotedev_notification->setChecked(settings_.value("use_notification", false).toBool());
  ui_->wiimotedev_device->setValue(settings_.value("device", 1).toUInt());
  settings_.endGroup();

  connect(ui_->wiimotedev_enable, SIGNAL(clicked(bool)), this, SLOT(WiimotedevEnabledChecked(bool)));
}

void WiimotedevShortcutsConfig::WiimotedevEnabledChecked(bool checked) {
  ui_->wiimotedev_groupbox->setEnabled(checked);
}
