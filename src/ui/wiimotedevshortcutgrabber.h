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

#ifndef WIIMOTEDEVSHORTCUTGRABBER_H
#define WIIMOTEDEVSHORTCUTGRABBER_H

#include <QDialog>

class WiimotedevShortcutsConfig;
class DBusDeviceEventsInterface;
class Ui_WiimotedevShortcutGrabber;

class WiimotedevShortcutGrabber : public QDialog {
  Q_OBJECT

 public:
  WiimotedevShortcutGrabber(QWidget* parent = 0);
  ~WiimotedevShortcutGrabber();

 private slots:
  void DbusWiimoteGeneralButtons(quint32 id, quint64 value);

 private:
  Ui_WiimotedevShortcutGrabber* ui_;
  WiimotedevShortcutsConfig* config_;
  DBusDeviceEventsInterface* wiimotedev_iface_;
  quint32 wiimotedev_device_;
  quint64 wiimotedev_buttons_;
  quint32 action;

};

#endif // WIIMOTEDEVSHORTCUTGRABBER_H
