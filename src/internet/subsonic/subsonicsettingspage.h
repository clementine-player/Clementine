/* This file is part of Clementine.
   Copyright 2011, 2013, Alan Briolat <alan.briolat@gmail.com>
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

#ifndef INTERNET_SUBSONIC_SUBSONICSETTINGSPAGE_H_
#define INTERNET_SUBSONIC_SUBSONICSETTINGSPAGE_H_

#include "subsonicservice.h"
#include "ui/settingspage.h"

class Ui_SubsonicSettingsPage;

class SubsonicSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit SubsonicSettingsPage(SettingsDialog* dialog);
  ~SubsonicSettingsPage();

  void Load();
  void Save();

 public slots:
  void LoginStateChanged(SubsonicService::LoginState newstate);

 private slots:
  void ServerEditingFinished();
  void Login();
  void Logout();

 private:
  Ui_SubsonicSettingsPage* ui_;
  SubsonicService* service_;
};

#endif  // INTERNET_SUBSONIC_SUBSONICSETTINGSPAGE_H_
