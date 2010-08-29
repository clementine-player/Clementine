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

#include "ui/wiimotedevshortcutgrabber.h"
#include "ui_wiimotedevshortcutgrabber.h"
#include "wiimotedev/interface.h"
#include "ui/wiimotedevshortcutsconfig.h"


WiimotedevShortcutGrabber::WiimotedevShortcutGrabber(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_WiimotedevShortcutGrabber),
    config_(qobject_cast<WiimotedevShortcutsConfig*>(parent)),
    wiimotedev_device_(1),
    wiimotedev_buttons_(0)
{
  ui_->setupUi(this);

  if (QDBusConnection::systemBus().isConnected()) {
    wiimotedev_iface_ = new DBusDeviceEventsInterface(WIIMOTEDEV_DBUS_SERVICE_NAME,
                                                      WIIMOTEDEV_DBUS_EVENTS_OBJECT,
                                                      QDBusConnection::systemBus(),
                                                      this);

    connect(wiimotedev_iface_, SIGNAL(dbusWiimoteGeneralButtons(quint32,quint64)),
            this, SLOT(DbusWiimoteGeneralButtons(quint32, quint64)));
  }

  foreach (const QString& name, config_->text_actions_.values())
    ui_->comboBox->addItem(name);

}

WiimotedevShortcutGrabber::~WiimotedevShortcutGrabber() {
  delete ui_;
}

void WiimotedevShortcutGrabber::DbusWiimoteGeneralButtons(quint32 id, quint64 value) {
  if (wiimotedev_device_ != id) return;

  quint64 buttons = value & ~(WIIMOTE_TILT_MASK | NUNCHUK_TILT_MASK);
  if (wiimotedev_buttons_ == buttons) return;

  ui_->combo->setText(config_->GetReadableWiiremoteSequence(buttons));

  wiimotedev_buttons_ = buttons;
}
