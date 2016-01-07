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

#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

#include "widgets/osd.h"

class SettingsDialog;

class SettingsPage : public QWidget {
  Q_OBJECT

 public:
  SettingsPage(SettingsDialog* dialog);

  // Return false to grey out the page's item in the list.
  virtual bool IsEnabled() const { return true; }

  // Load is called when the dialog is shown, Save when the user clicks OK, and
  // Cancel when the user clicks on Cancel
  virtual void Load() = 0;
  virtual void Save() = 0;
  virtual void Cancel() {}

  // The dialog that this page belongs to.
  SettingsDialog* dialog() const { return dialog_; }

signals:
  void NotificationPreview(OSD::Behaviour, QString, QString);
  void SetWiimotedevInterfaceActived(bool);
  void InhibitSuspendWhilePlaying(bool);

 private:
  SettingsDialog* dialog_;
};

#endif  // SETTINGSPAGE_H
