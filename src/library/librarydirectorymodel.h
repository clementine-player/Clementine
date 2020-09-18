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

#ifndef LIBRARYDIRECTORYMODEL_H
#define LIBRARYDIRECTORYMODEL_H

#include <QIcon>
#include <QStandardItemModel>
#include <memory>

#include "directory.h"

class LibraryBackend;
class MusicStorage;

class LibraryDirectoryModel : public QStandardItemModel {
  Q_OBJECT

 public:
  LibraryDirectoryModel(std::shared_ptr<LibraryBackend> backend,
                        QObject* parent = nullptr);
  ~LibraryDirectoryModel();

  // To be called by GUIs
  void AddDirectory(const QString& path);
  void RemoveDirectory(const QModelIndex& index);

  QVariant data(const QModelIndex& index, int role) const;

 private slots:
  // To be called by the backend
  void DirectoryDiscovered(const Directory& directories);
  void DirectoryDeleted(int dir_id);

 private:
  static const int kIdRole = Qt::UserRole + 1;

  QIcon dir_icon_;
  std::shared_ptr<LibraryBackend> backend_;
  QList<std::shared_ptr<MusicStorage>> storage_;
};

#endif  // LIBRARYDIRECTORYMODEL_H
