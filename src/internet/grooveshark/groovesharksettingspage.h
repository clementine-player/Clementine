/* This file is part of Clementine.
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_GROOVESHARK_GROOVESHARKSETTINGSPAGE_H_
#define INTERNET_GROOVESHARK_GROOVESHARKSETTINGSPAGE_H_

#include "ui/settingspage.h"

class NetworkAccessManager;
class Ui_GroovesharkSettingsPage;
class GroovesharkService;

class GroovesharkSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit GroovesharkSettingsPage(SettingsDialog* dialog);
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

#endif  // INTERNET_GROOVESHARK_GROOVESHARKSETTINGSPAGE_H_
