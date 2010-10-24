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

#ifndef SMARTPLAYLISTMODEL_H
#define SMARTPLAYLISTMODEL_H

#include "playlistgenerator_fwd.h"

#include <QStandardItemModel>

class LibraryBackend;

class QSettings;

class SmartPlaylistModel : public QStandardItemModel {
  Q_OBJECT

public:
  SmartPlaylistModel(QObject* parent = 0);

  enum Role {
    Role_Type = Qt::UserRole + 1,
    Role_ContainerGroup,
    Role_GeneratorClass,

    RoleCount
  };

  enum Type {
    Type_Invalid,

    Type_Container,
    Type_Generator,
  };

  static const char* kSettingsGroup;

  PlaylistGeneratorPtr CreateGenerator(const QModelIndex& index,
                                       LibraryBackend* library) const;

private:
  void Load(const char* name, QStandardItem* parent);
  void SaveDefaults();
  void SaveDefaultQuery(QSettings* settings, int i, const QString& name,
                        const QString& where, const QString& order,
                        int limit = -1);

  QStandardItem* CreateContainer(const QString& name, const QString& group);

private:
  QIcon container_icon_;
  QIcon playlist_icon_;

  QStandardItem* smart_item_;
  QStandardItem* dynamic_item_;
};

#endif // SMARTPLAYLISTMODEL_H
