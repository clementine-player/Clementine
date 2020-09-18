/* This file is part of Clementine.
   Copyright 2013-2014, John Maguire <john.maguire@gmail.com>
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

#ifndef INTERNET_BOX_BOXSETTINGSPAGE_H_
#define INTERNET_BOX_BOXSETTINGSPAGE_H_

#include <QModelIndex>
#include <QWidget>

#include "ui/settingspage.h"

class BoxService;
class Ui_BoxSettingsPage;

class BoxSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit BoxSettingsPage(SettingsDialog* parent = nullptr);
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

#endif  // INTERNET_BOX_BOXSETTINGSPAGE_H_
