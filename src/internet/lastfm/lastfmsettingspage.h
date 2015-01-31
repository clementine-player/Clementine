/* This file is part of Clementine.
   Copyright 2009-2011, 2013, David Sansome <me@davidsansome.com>
   Copyright 2011, Andrea Decorte <adecorte@gmail.com>
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

#ifndef INTERNET_LASTFM_LASTFMSETTINGSPAGE_H_
#define INTERNET_LASTFM_LASTFMSETTINGSPAGE_H_

#include "ui/settingspage.h"

class LastFMService;
class Ui_LastFMSettingsPage;

class LastFMSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit LastFMSettingsPage(SettingsDialog* dialog);
  ~LastFMSettingsPage();

  void Load();
  void Save();

 private slots:
  void Login();
  void AuthenticationComplete(bool success, const QString& error_message);
  void Logout();
  void AdvancedOptionsChanged(int state);
  void AddHttpServer();

 private:
  LastFMService* service_;
  Ui_LastFMSettingsPage* ui_;

  bool waiting_for_auth_;

  void RefreshControls(bool authenticated);
};

#endif  // INTERNET_LASTFM_LASTFMSETTINGSPAGE_H_
