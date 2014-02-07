/* This file is part of Clementine.
   Copyright 2013, John Maguire <john.maguire@gmail.com>

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

#ifndef BOXSETTINGSPAGE_H
#define BOXSETTINGSPAGE_H

#include "ui/settingspage.h"

#include <QModelIndex>
#include <QWidget>

class BoxService;
class Ui_BoxSettingsPage;

class BoxSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  BoxSettingsPage(SettingsDialog* parent = 0);
  ~BoxSettingsPage();

  void Load();
  void Save();

  // QObject
  bool eventFilter(QObject* object, QEvent* event);

 private slots:
  void LoginClicked();
  void LogoutClicked();
  void Connected();

 private:
  Ui_BoxSettingsPage* ui_;

  BoxService* service_;
};

#endif  // BOXSETTINGSPAGE_H
