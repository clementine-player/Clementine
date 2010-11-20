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

#ifndef WIIMOTEDEVSHORTCUTGRABBER_H
#define WIIMOTEDEVSHORTCUTGRABBER_H

#include <QDialog>
#include <QTimeLine>

#include <boost/scoped_ptr.hpp>
#include "dbus/wiimotedev.h"
#include "ui/wiimotedevshortcutsconfig.h"

class Ui_WiimotedevShortcutGrabber;

class WiimotedevShortcutGrabber : public QDialog {
  Q_OBJECT
public:
  WiimotedevShortcutGrabber(quint32 action = 0, quint32 device = 1, QWidget* parent = 0);
  ~WiimotedevShortcutGrabber();

private slots:
  void DbusWiimoteGeneralButtons(uint id, qulonglong value);
  void RememberSwingChecked(bool checked);
  void Timeout(int);

private:
  QTimeLine line_;
  quint32 pref_action_;

  Ui_WiimotedevShortcutGrabber* ui_;
  WiimotedevShortcutsConfig* config_;
  boost::scoped_ptr<OrgWiimotedevDeviceEventsInterface> wiimotedev_iface_;
  quint32 wiimotedev_device_;
  quint64 wiimotedev_buttons_;

  quint64 remember_wiimote_shifts_;
  quint64 remember_nunchuk_shifts_;

signals:
  void AddShortcut(quint64 buttons, quint32 action);

};

#endif // WIIMOTEDEVSHORTCUTGRABBER_H
