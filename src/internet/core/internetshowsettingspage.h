/* This file is part of Clementine.
   Copyright 2014, Chocobozzz <florian.bigard@gmail.com>
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

#ifndef INTERNET_CORE_INTERNETSHOWSETTINGSPAGE_H_
#define INTERNET_CORE_INTERNETSHOWSETTINGSPAGE_H_

#include <QIcon>
#include <memory>

#include "ui/settingspage.h"
#include "ui_internetshowsettingspage.h"

class QTreeWidgetItem;
class Ui_InternetShowSettingsPage;

class InternetShowSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit InternetShowSettingsPage(SettingsDialog* dialog);

  void Load();
  void Save();

 private:
  std::unique_ptr<Ui_InternetShowSettingsPage> ui_;
};

#endif  // INTERNET_CORE_INTERNETSHOWSETTINGSPAGE_H_
