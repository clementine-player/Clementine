/* This file is part of Clementine.
   Copyright 2021, Fabio Bas <ctrlaltca@gmail.com>

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

#ifndef INTERNET_RADIOBROWSER_RADIOBROWSERSETTINGSPAGE_H_
#define INTERNET_RADIOBROWSER_RADIOBROWSERSETTINGSPAGE_H_

#include "radiobrowserservice.h"
#include "ui/settingspage.h"
#include "ui_radiobrowsersettingspage.h"

class RadioBrowserSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit RadioBrowserSettingsPage(SettingsDialog* dialog);
  ~RadioBrowserSettingsPage(){};

  void Load();
  void Save();

 private slots:
  void ServerEditingFinished();
  void RestoreDefaultServer();

 private:
  std::unique_ptr<Ui_RadioBrowserSettingsPage> ui_;
  RadioBrowserService* service_;
};

#endif  // INTERNET_RADIOBROWSER_RADIOBROWSERSETTINGSPAGE_H_
