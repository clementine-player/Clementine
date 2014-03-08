/* This file is part of Clementine.
   Copyright 2013, Vlad Maltsev <shedwardx@gmail.com>

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

#ifndef VKSETTINGSPAGE_H
#define VKSETTINGSPAGE_H

#include "ui/settingspage.h"

#include <QModelIndex>
#include <QWidget>

class VkService;
class Ui_VkSettingsPage;

class VkSettingsPage : public SettingsPage {
  Q_OBJECT

public:
  explicit VkSettingsPage(SettingsDialog* parent);
  ~VkSettingsPage();

  void Load();
  void Save();

private slots:
  void LoginSuccess(bool success);

  void Login();
  void Logout();

  void CacheDirBrowse();
  void ResetCasheFilenames();

private:
  void LoginWidgets();
  void LogoutWidgets();
  Ui_VkSettingsPage* ui_;
  VkService* service_;
};

#endif  // VKSETTINGSPAGE_H
