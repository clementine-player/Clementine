/* This file is part of Clementine.
   Copyright 2015, Marco Kirchner <kirchnermarco@gmail.com>
   Copyright 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef INTERNET_PLAYMUSIC_PLAYMUSICSETTINGSPAGE_H_
#define INTERNET_PLAYMUSIC_PLAYMUSICSETTINGSPAGE_H_

#include "ui/settingspage.h"

class PlayMusicService;
class Ui_PlayMusicSettingsPage;

class PlayMusicSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit PlayMusicSettingsPage(SettingsDialog* parent = nullptr);
  ~PlayMusicSettingsPage();

  void Load();
  void Save();

 private slots:
  void LoginClicked();
  void LogoutClicked();
  void LoginStateChanged();

 private:
  void SetLoggedIn(bool loggedIn);

  Ui_PlayMusicSettingsPage* ui_;

  PlayMusicService* service_;
};

#endif  // INTERNET_PLAYMUSIC_PLAYMUSICSETTINGSPAGE_H_
