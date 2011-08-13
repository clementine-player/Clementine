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

#ifndef REMOTESETTINGSPAGE_H
#define REMOTESETTINGSPAGE_H

#include "core/network.h"
#include "ui/settingspage.h"

class Ui_RemoteSettingsPage;

class RemoteSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  RemoteSettingsPage(SettingsDialog* dialog);
  ~RemoteSettingsPage();

  static QString DefaultAgentName();

  static const char* kSettingsGroup;

  void Load();
  void Save();

 private slots:
  void Login();
  void LoginFinished();
  void LoginComplete(const QString& token);
  void SignOut();

 private:
  void ValidateGoogleAccount(const QString& username, const QString& password);

  Ui_RemoteSettingsPage* ui_;
  bool waiting_for_auth_;
  NetworkAccessManager* network_;
};

#endif // REMOTESETTINGSPAGE_H
