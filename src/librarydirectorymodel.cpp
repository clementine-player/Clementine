/* This file is part of Clementine.

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

LibraryDirectoryModel::LibraryDirectoryModel(QObject* parent)
  : QStandardItemModel(parent),
    dir_icon_(":folder.png")
{
}

void LibraryDirectoryModel::SetBackend(boost::shared_ptr<LibraryBackendInterface> backend) {
  if (backend_)
    backend_->disconnect(this);

  backend_ = backend;

  connect(backend_.get(), SIGNAL(DirectoryDiscovered(Directory, SubdirectoryList)), SLOT(DirectoryDiscovered(Directory)));
  connect(backend_.get(), SIGNAL(DirectoryDeleted(Directory)), SLOT(DirectoryDeleted(Directory)));

  emit BackendReady();
}

void LibraryDirectoryModel::DirectoryDiscovered(const Directory &dir) {
  QStandardItem* item = new QStandardItem(dir.path);
  item->setData(dir.id, kIdRole);
  item->setIcon(dir_icon_);
  appendRow(item);
}

void LibraryDirectoryModel::DirectoryDeleted(const Directory &dir) {
  for (int i=0 ; i<rowCount() ; ++i) {
    if (item(i, 0)->data(kIdRole).toInt() == dir.id) {
      removeRow(i);
      break;
    }
  }
}

void LibraryDirectoryModel::AddDirectory(const QString& path) {
  if (!backend_)
    return;

  backend_->AddDirectory(path);
}

void LibraryDirectoryModel::RemoveDirectory(const QModelIndex& index) {
  if (!backend_ || !index.isValid())
    return;

  Directory dir;
  dir.path = index.data().toString();
  dir.id = index.data(kIdRole).toInt();

  backend_->RemoveDirectory(dir);
}
