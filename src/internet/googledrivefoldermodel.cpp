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

#include "googledriveclient.h"
#include "googledrivefoldermodel.h"
#include "core/closure.h"
#include "ui/iconloader.h"

using namespace google_drive;

FolderModel::FolderModel(Client* client, QObject* parent)
  : QStandardItemModel(parent),
    client_(client)
{
  folder_icon_ = IconLoader::Load("folder");

  root_ = new QStandardItem(tr("My Drive"));
  item_by_id_[QString()] = root_;
  invisibleRootItem()->appendRow(root_);

  connect(client, SIGNAL(Authenticated()), SLOT(Refresh()));
  if (client->is_authenticated()) {
    Refresh();
  }
}

void FolderModel::Refresh() {
  ListFilesResponse* reply =
      client_->ListFiles(QString("mimeType = '%1'").arg(File::kFolderMimeType));
  connect(reply, SIGNAL(FilesFound(QList<google_drive::File>)),
          this, SLOT(FilesFound(QList<google_drive::File>)));
  NewClosure(reply, SIGNAL(Finished()),
             this, SLOT(FindFilesFinished(ListFilesResponse*)),
             reply);
}

void FolderModel::FindFilesFinished(ListFilesResponse* reply) {
  reply->deleteLater();
}

void FolderModel::FilesFound(const QList<google_drive::File>& files) {
  foreach (const File& file, files) {
    if (file.is_hidden() || file.is_trashed()) {
      continue;
    }

    const QString id(file.id());

    // Does this file exist in the model already?
    if (item_by_id_.contains(id)) {
      // If it has the same etag ignore it, otherwise remove and recreate it.
      QStandardItem* old_item = item_by_id_[id];
      if (old_item->data(Role_Etag).toString() == file.etag()) {
        continue;
      } else {
        item_by_id_.remove(id);
        old_item->parent()->removeRow(old_item->row());
      }
    }

    // Get the first parent's ID
    const QStringList parent_ids = file.parent_ids();
    if (parent_ids.isEmpty()) {
      continue;
    }
    const QString parent_id = parent_ids.first();

    // If the parent doesn't exist yet, remember this file for later.
    if (!item_by_id_.contains(parent_id)) {
      orphans_[parent_id] << file;
      continue;
    }

    // Find the item for the parent
    QStandardItem* parent = item_by_id_[parent_id];

    // Create the item
    QStandardItem* item = new QStandardItem(file.title());
    item->setData(file.etag(), Role_Etag);
    item->setData(id, Role_Id);
    item_by_id_[id] = item;
    parent->appendRow(item);

    // Add any children for this item that we saw before.
    if (orphans_.contains(id)) {
      FilesFound(orphans_.take(id));
    }
  }
}

QVariant FolderModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::DecorationRole) {
    return folder_icon_;
  }

  return QStandardItemModel::data(index, role);
}

QStandardItem* FolderModel::ItemById(const QString& id) const {
  return item_by_id_[id];
}
