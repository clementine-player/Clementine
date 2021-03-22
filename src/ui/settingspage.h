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

class SettingsDialog;

class SettingsPage : public QWidget {
  Q_OBJECT

 public:
  SettingsPage(SettingsDialog* dialog);

  // Return false to grey out the page's item in the list.
  virtual bool IsEnabled() const { return true; }

  // Called when the dialog is shown.
  virtual void Load() = 0;
  // Called when Apply is selected.
  virtual void Apply();
  // Called when OK is selected.
  virtual void Accept();
  // Called when Cancel is selected.
  virtual void Reject();

  // The dialog that this page belongs to.
  SettingsDialog* dialog() const { return dialog_; }

 protected:
  void showEvent(QShowEvent* event);

  bool maybe_changed_;

 private:
  virtual void Save() = 0;
  virtual void Cancel() {}

  SettingsDialog* dialog_;
};

#endif  // SETTINGSPAGE_H
