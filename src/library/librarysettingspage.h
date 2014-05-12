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

#ifndef LIBRARYSETTINGSPAGE_H
#define LIBRARYSETTINGSPAGE_H

#include "ui/settingspage.h"

class Ui_LibrarySettingsPage;
class LibraryDirectoryModel;

class QModelIndex;

class LibrarySettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  LibrarySettingsPage(SettingsDialog* dialog);
  ~LibrarySettingsPage();

  static const char* kSettingsGroup;

  void Load();
  void Save();

 private slots:
  void Add();
  void Remove();
  void WriteAllSongsStatisticsToFiles();

  void CurrentRowChanged(const QModelIndex& index);

 private:
  Ui_LibrarySettingsPage* ui_;
  bool initialised_model_;
};

#endif  // LIBRARYSETTINGSPAGE_H
