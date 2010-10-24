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

#include "playlistgenerator.h"
#include "smartplaylistmodel.h"

#include <QSettings>

const char* SmartPlaylistModel::kSettingsGroup = "SmartPlaylists";

SmartPlaylistModel::SmartPlaylistModel(QObject* parent)
  : QStandardItemModel(parent),
    smart_item_(CreateContainer(tr("Smart playlists"), "smart")),
    dynamic_item_(CreateContainer(tr("Dynamic playlists"), "dynamic"))
{
  setColumnCount(1);

  invisibleRootItem()->appendRow(smart_item_);
  invisibleRootItem()->appendRow(dynamic_item_);

  QSettings s;
  if (!s.childGroups().contains(kSettingsGroup)) {
    SaveDefaults();
  }

  Load("smart", smart_item_);
  Load("dynamic", dynamic_item_);
}

QStandardItem* SmartPlaylistModel::CreateContainer(
    const QString& name, const QString& group) {
  QStandardItem* ret = new QStandardItem(name);
  ret->setData(Type_Container, Role_Type);
  ret->setData(group, Role_ContainerGroup);
  return ret;
}

void SmartPlaylistModel::Load(const char* name, QStandardItem* parent) {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  const int count = s.beginReadArray(name);
  for (int i=0 ; i<count ; ++i) {
    s.setArrayIndex(i);
    QStandardItem* item = new QStandardItem;
    item->setText(s.value("name").toString());
    item->setData(s.value("type").toString(), Role_GeneratorClass);
    item->setData(Type_Generator, Role_Type);
    parent->appendRow(item);
  }
}

void SmartPlaylistModel::SaveDefaults() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  int i = 0;
  s.beginWriteArray("smart");
  SaveDefaultQuery(&s, i++, tr("50 random tracks"), QString(), "random()", 50);
  SaveDefaultQuery(&s, i++, tr("Ever played"), "playcount > 0", QString());
  SaveDefaultQuery(&s, i++, tr("Last played"), QString(), "lastplayed DESC");
  SaveDefaultQuery(&s, i++, tr("Most played"), QString(), "playcount DESC");
  SaveDefaultQuery(&s, i++, tr("Never played"), "playcount = 0", QString());
  SaveDefaultQuery(&s, i++, tr("Favourite tracks"), QString(), "rating DESC"); // TODO: use score
  SaveDefaultQuery(&s, i++, tr("Newest tracks"), QString(), "ctime DESC");
  s.endArray();
}

void SmartPlaylistModel::SaveDefaultQuery(QSettings* s, int i,
    const QString& name, const QString& where, const QString& order, int limit) {
  s->setArrayIndex(i);
  s->setValue("name", name);
  s->setValue("type", "Query");
  if (limit != -1)      s->setValue("limit", limit);
  if (!where.isEmpty()) s->setValue("where", where);
  if (!order.isEmpty()) s->setValue("order", order);
}

PlaylistGeneratorPtr SmartPlaylistModel::CreateGenerator(
    const QModelIndex& index, LibraryBackend* library) const {
  PlaylistGeneratorPtr ret;

  // Get the item
  const QStandardItem* item = itemFromIndex(index);
  if (!item || item->data(Role_Type).toInt() != Type_Generator)
    return ret;

  // Get the container that the item is in
  const QStandardItem* container = itemFromIndex(index.parent());
  if (!container || container->data(Role_Type).toInt() != Type_Container)
    return ret;

  // Open the settings in the right place
  QSettings s;
  s.beginGroup(kSettingsGroup);
  const int count = s.beginReadArray(container->data(Role_ContainerGroup).toString());

  // Check this row really exists
  if (index.row() >= count)
    return ret;
  s.setArrayIndex(index.row());

  // Create a generator of the right type
  ret = PlaylistGenerator::Create(item->data(Role_GeneratorClass).toString());
  if (!ret)
    return ret;

  // Initialise the generator
  ret->set_library(library);
  ret->Load(s);
  return ret;
}
