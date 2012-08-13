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

#ifndef GOOGLEDRIVEFOLDERMODEL_H
#define GOOGLEDRIVEFOLDERMODEL_H

#include "googledriveclient.h"

#include <QStandardItemModel>

namespace google_drive {

class Client;

class FolderModel : public QStandardItemModel {
  Q_OBJECT

public:
  FolderModel(Client* client, QObject* parent = 0);

  enum Role {
    Role_Etag = Qt::UserRole,
    Role_Id
  };

  QIcon folder_icon() const { return folder_icon_; }
  void set_folder_icon(const QIcon& icon) { folder_icon_ = icon; }

  QVariant data(const QModelIndex& index, int role) const;

  QStandardItem* ItemById(const QString& id) const;

public slots:
  void Refresh();

private slots:
  void FilesFound(const QList<google_drive::File>& files);
  void FindFilesFinished(ListFilesResponse* reply);
  
private:
  Client* client_;
  QIcon folder_icon_;

  QStandardItem* root_;

  QMap<QString, QStandardItem*> item_by_id_;
  QMap<QString, QList<google_drive::File> > orphans_;
};

} // namespace

#endif // GOOGLEDRIVEFOLDERMODEL_H
