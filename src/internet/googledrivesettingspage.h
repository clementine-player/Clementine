/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#ifndef GOOGLEDRIVESETTINGSPAGE_H
#define GOOGLEDRIVESETTINGSPAGE_H

#include "ui/settingspage.h"

#include <QModelIndex>
#include <QWidget>

class Ui_GoogleDriveSettingsPage;

class QSortFilterProxyModel;

namespace google_drive {
  class FolderModel;
}

class GoogleDriveSettingsPage : public SettingsPage {
  Q_OBJECT
  
public:
  GoogleDriveSettingsPage(SettingsDialog* parent = 0);
  ~GoogleDriveSettingsPage();

  void Load();
  void Save();

private slots:
  void DirectoryRowsInserted(const QModelIndex& parent);
  
private:
  Ui_GoogleDriveSettingsPage* ui_;

  google_drive::FolderModel* model_;
  QSortFilterProxyModel* proxy_model_;

  QString destination_folder_id_;
  bool item_needs_selecting_;
};

#endif // GOOGLEDRIVESETTINGSPAGE_H
