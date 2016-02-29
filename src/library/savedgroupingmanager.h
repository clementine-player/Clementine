/* This file is part of Clementine.
   Copyright 2015, Nick Lanham <nick@afternight.org>

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

#ifndef SAVEDGROUPINGMANAGER_H
#define SAVEDGROUPINGMANAGER_H

#include <QDialog>
#include <QStandardItemModel>

#include "librarymodel.h"

class Ui_SavedGroupingManager;
class LibraryFilterWidget;

class SavedGroupingManager : public QDialog {
  Q_OBJECT

 public:
  SavedGroupingManager(QWidget* parent = nullptr);
  ~SavedGroupingManager();

  void UpdateModel();
  void SetFilter(LibraryFilterWidget* filter) { filter_ = filter; }

  static QString GroupByToString(const LibraryModel::GroupBy& g);

 private slots:
  void UpdateButtonState();
  void Remove();

 private:
  Ui_SavedGroupingManager* ui_;
  QStandardItemModel* model_;
  LibraryFilterWidget* filter_;
};

#endif  // SAVEDGROUPINGMANAGER_H
