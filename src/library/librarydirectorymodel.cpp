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

#include "librarydirectorymodel.h"
#include "librarybackend.h"
#include "core/application.h"
#include "core/filesystemmusicstorage.h"
#include "core/musicstorage.h"
#include "core/utilities.h"
#include "ui/iconloader.h"

LibraryDirectoryModel::LibraryDirectoryModel(
    std::shared_ptr<LibraryBackend> backend, QObject* parent)
    : QStandardItemModel(parent),
      dir_icon_(IconLoader::Load("document-open-folder", IconLoader::Base)),
      backend_(backend) {
  connect(backend_.get(),
          SIGNAL(DirectoryDiscovered(Directory, SubdirectoryList)),
          SLOT(DirectoryDiscovered(Directory)));
  connect(backend_.get(), SIGNAL(DirectoryDeleted(int)),
          SLOT(DirectoryDeleted(int)));
}

LibraryDirectoryModel::~LibraryDirectoryModel() {}

void LibraryDirectoryModel::DirectoryDiscovered(const Directory& dir) {
  QStandardItem* item;
  if (Application::kIsPortable &&
      Utilities::UrlOnSameDriveAsClementine(QUrl::fromLocalFile(dir.path))) {
    item =
        new QStandardItem(Utilities::GetRelativePathToClementineBin(dir.path));
  } else {
    item = new QStandardItem(dir.path);
  }
  item->setData(dir.id, kIdRole);
  item->setIcon(dir_icon_);
  storage_ << std::shared_ptr<MusicStorage>(
                  new FilesystemMusicStorage(dir.path));
  appendRow(item);
}

void LibraryDirectoryModel::DirectoryDeleted(int dir_id) {
  for (int i = 0; i < rowCount(); ++i) {
    if (item(i, 0)->data(kIdRole).toInt() == dir_id) {
      removeRow(i);
      storage_.removeAt(i);
      break;
    }
  }
}

void LibraryDirectoryModel::AddDirectory(const QString& path) {
  if (!backend_) return;

  backend_->AddDirectory(path);
}

void LibraryDirectoryModel::RemoveDirectory(const QModelIndex& index) {
  if (!backend_ || !index.isValid()) return;

  int dir_id = index.data(kIdRole).toInt();

  backend_->RemoveDirectory(dir_id);
}

QVariant LibraryDirectoryModel::data(const QModelIndex& index, int role) const {
  switch (role) {
    case MusicStorage::Role_Storage:
    case MusicStorage::Role_StorageForceConnect:
      return QVariant::fromValue(storage_[index.row()]);

    case MusicStorage::Role_FreeSpace:
      return Utilities::FileSystemFreeSpace(
          data(index, Qt::DisplayRole).toString());

    case MusicStorage::Role_Capacity:
      return Utilities::FileSystemCapacity(
          data(index, Qt::DisplayRole).toString());

    default:
      return QStandardItemModel::data(index, role);
  }
}
