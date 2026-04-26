/* This file is part of Clementine.
   Copyright 2026

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

#ifndef INTERNET_MEGA_MEGASETTINGSPAGE_H_
#define INTERNET_MEGA_MEGASETTINGSPAGE_H_

#include <QSet>

#include "ui/settingspage.h"

class MegaService;
class QTreeWidgetItem;
class Ui_MegaSettingsPage;
struct MegaFolderInfo;

class MegaSettingsPage : public SettingsPage {
  Q_OBJECT

 public:
  explicit MegaSettingsPage(SettingsDialog* parent = nullptr);
  ~MegaSettingsPage();

  void Load();
  void Save();

  bool eventFilter(QObject* object, QEvent* event);

 private slots:
  void LoginClicked();
  void LogoutClicked();
  void Connected();
  void RefreshFoldersClicked();
  void ScanClicked();
  void ResetClicked();
  void AutoSelectClicked();

 private:
  void PopulateFolderTree();
  void AddFolderItems(QTreeWidgetItem* parent,
                      const QList<MegaFolderInfo>& folders,
                      const QStringList& selected);
  void CollectSelectedFolders(QTreeWidgetItem* parent,
                              QStringList& handles);
  void AutoSelectItems(QTreeWidgetItem* parent,
                       const QSet<QString>& handles);
  void SaveSelectedFolders();

  Ui_MegaSettingsPage* ui_;
  MegaService* service_;
};

#endif  // INTERNET_MEGA_MEGASETTINGSPAGE_H_
