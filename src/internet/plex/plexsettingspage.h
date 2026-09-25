/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_PLEX_PLEXSETTINGSPAGE_H_
#define INTERNET_PLEX_PLEXSETTINGSPAGE_H_

#include "internet/plex/plexparser.h"
#include "internet/plex/plexservice.h"
#include "ui/settingspage.h"

class Ui_PlexSettingsPage;
class PlexAuthenticator;

class PlexSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit PlexSettingsPage(SettingsDialog* dialog);
  ~PlexSettingsPage() override;

  void Load() override;
  void Save() override;

 private slots:
  void SignIn();
  void CheckPin();
  void SignOut();
  void AuthCodeReady(const QString& code);
  void Authenticated(const QString& user_token);
  void ServersFound(const QList<PlexServer>& servers);
  void AuthFailed(const QString& message);
  void LoginStateChanged(PlexService::LoginState state);

 private:
  void UpdateSignedInState();

  Ui_PlexSettingsPage* ui_;
  PlexService* service_;
  PlexAuthenticator* authenticator_;

  QString user_token_;
  QList<PlexServer> servers_;
};

#endif  // INTERNET_PLEX_PLEXSETTINGSPAGE_H_
