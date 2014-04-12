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

#ifndef GROOVESHARKSETTINGSPAGE_H
#define GROOVESHARKSETTINGSPAGE_H

#include "ui/settingspage.h"

class NetworkAccessManager;
class Ui_GroovesharkSettingsPage;
class GroovesharkService;

class GroovesharkSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  GroovesharkSettingsPage(SettingsDialog* dialog);
  ~GroovesharkSettingsPage();

  void Load();
  void Save();

 private slots:
  void Login();
  void LoginFinished(bool success);
  void Logout();

 private:
  void UpdateLoginState();

 private:
  Ui_GroovesharkSettingsPage* ui_;
  GroovesharkService* service_;

  bool validated_;
  QString original_username_;
  QString original_password_;
};

#endif  // GROOVESHARKSETTINGSPAGE_H
