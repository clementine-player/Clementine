/* This file is part of Clementine.
   Copyright 2014, Chocobozzz <djidane14ff@hotmail.fr>
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

#ifndef INTERNET_SEAFILESETTINGSPAGE_H_
#define INTERNET_SEAFILESETTINGSPAGE_H_

#include "ui/settingspage.h"

#include <QModelIndex>
#include <QWidget>

class Ui_SeafileSettingsPage;
class SeafileService;

class SeafileSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit SeafileSettingsPage(SettingsDialog* dialog);
  ~SeafileSettingsPage();

  void Load();
  void Save();

 private slots:
  void Login();
  void Logout();
  // Map -> key : library's id, value : library's name
  void GetLibrariesFinished(const QMap<QString, QString>& libraries);

 private:
  std::unique_ptr<Ui_SeafileSettingsPage> ui_;
  SeafileService* service_;
};

#endif  // INTERNET_SEAFILESETTINGSPAGE_H_
