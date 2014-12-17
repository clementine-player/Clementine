/* This file is part of Clementine.
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#ifndef INTERNET_SKYDRIVESETTINGSPAGE_H_
#define INTERNET_SKYDRIVESETTINGSPAGE_H_

#include "ui/settingspage.h"

#include <QModelIndex>
#include <QWidget>

class SkydriveService;
class Ui_SkydriveSettingsPage;

class SkydriveSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit SkydriveSettingsPage(SettingsDialog* parent = nullptr);
  ~SkydriveSettingsPage();

  void Load();
  void Save();

  // QObject
  bool eventFilter(QObject* object, QEvent* event);

 private slots:
  void LoginClicked();
  void LogoutClicked();
  void Connected();

 private:
  Ui_SkydriveSettingsPage* ui_;

  SkydriveService* service_;
};

#endif  // INTERNET_SKYDRIVESETTINGSPAGE_H_
