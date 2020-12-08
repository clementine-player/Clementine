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

#ifndef NETWORKREMOTESETTINGSPAGE_H
#define NETWORKREMOTESETTINGSPAGE_H

#include "settingspage.h"

class Ui_NetworkRemoteSettingsPage;

class NetworkRemoteSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  NetworkRemoteSettingsPage(SettingsDialog* dialog);
  ~NetworkRemoteSettingsPage();

  void Load();
  void Save();

 protected:
  bool eventFilter(QObject* object, QEvent* event);

 private slots:
  void Options();

 private:
  static const char* kPlayStoreUrl;
  static const char* kPlayStoreUrl2;
  static const char* kAppleStoreUrl;
  static const char* kLatestReleasesUrl;

  Ui_NetworkRemoteSettingsPage* ui_;
};

#endif  // NETWORKREMOTESETTINGSPAGE_H
