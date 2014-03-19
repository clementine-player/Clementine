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

#ifndef MAGNATUNESETTINGSPAGE_H
#define MAGNATUNESETTINGSPAGE_H

#include "ui/settingspage.h"

class QAuthenticator;
class QNetworkReply;

class NetworkAccessManager;
class Ui_MagnatuneSettingsPage;

class MagnatuneSettingsPage : public SettingsPage {
  Q_OBJECT
 public:
  MagnatuneSettingsPage(SettingsDialog* dialog);
  ~MagnatuneSettingsPage();

  void Load();
  void Save();

 private slots:
  void Login();
  void Logout();
  void MembershipChanged(int value);
  void LoginFinished();
  void AuthenticationRequired(QNetworkReply* reply, QAuthenticator* auth);

 private:
  void UpdateLoginState();

 private:
  NetworkAccessManager* network_;
  Ui_MagnatuneSettingsPage* ui_;

  bool logged_in_;
};

#endif  // MAGNATUNESETTINGSPAGE_H
