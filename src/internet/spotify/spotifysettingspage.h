/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>
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

#ifndef INTERNET_SPOTIFY_SPOTIFYSETTINGSPAGE_H_
#define INTERNET_SPOTIFY_SPOTIFYSETTINGSPAGE_H_

#include "ui/settingspage.h"

class NetworkAccessManager;
class Ui_SpotifySettingsPage;
class SpotifyService;

class SpotifySettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit SpotifySettingsPage(SettingsDialog* dialog);
  ~SpotifySettingsPage();

  void Load();
  void Save();

 public slots:
  void BlobStateChanged();
  void DownloadBlob();

 private slots:
  void Login();
  void LoginFinished(bool success);
  void Logout();

 private:
  void UpdateLoginState();

 private:
  Ui_SpotifySettingsPage* ui_;
  SpotifyService* service_;

  bool validated_;
  QString original_username_;
  QString original_password_;
};

#endif  // INTERNET_SPOTIFY_SPOTIFYSETTINGSPAGE_H_
