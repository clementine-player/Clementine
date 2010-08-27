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

#include "radiomodel.h"
#include "radioservice.h"
#include "lastfmservice.h"
#include "somafmservice.h"
#include "radiomimedata.h"
#include "savedradio.h"
#include "magnatuneservice.h"
#include "core/mergedproxymodel.h"

#include <QMimeData>
#include <QTime>
#include <QtDebug>

QMap<QString, RadioService*> RadioModel::sServices;

RadioModel::RadioModel(BackgroundThread<Database>* db_thread,
                       NetworkAccessManager* network, TaskManager* task_manager,
                       QObject* parent)
  : SimpleTreeModel<RadioItem>(new RadioItem(this), parent),
    db_thread_(db_thread),
    merged_model_(new MergedProxyModel(this)),
    network_(network),
    task_manager_(task_manager)
{
  Q_ASSERT(sServices.isEmpty());

  root_->lazy_loaded = true;
  merged_model_->setSourceModel(this);

  QTime t;
  t.start();

  AddService(new LastFMService(this));
  qDebug() << t.restart() << "lastfm";
  AddService(new SomaFMService(this));
  qDebug() << t.restart() << "somafm";
  AddService(new MagnatuneService(this));
  qDebug() << t.restart() << "magnatune";
  AddService(new SavedRadio(this));
  qDebug() << t.restart() << "saved";
}

void RadioModel::AddService(RadioService *service) {
  sServices[service->name()] = service;
  service->CreateRootItem(root_);

  connect(service, SIGNAL(AsyncLoadFinished(PlaylistItem::SpecialLoadResult)), SIGNAL(AsyncLoadFinished(PlaylistItem::SpecialLoadResult)));
  connect(service, SIGNAL(StreamError(QString)), SIGNAL(StreamError(QString)));
  connect(service, SIGNAL(StreamMetadataFound(QUrl,Song)), SIGNAL(StreamMetadataFound(QUrl,Song)));
  connect(service, SIGNAL(OpenSettingsAtPage(SettingsDialog::Page)), SIGNAL(OpenSettingsAtPage(SettingsDialog::Page)));
  connect(service, SIGNAL(AddItemToPlaylist(RadioItem*)), SIGNAL(AddItemToPlaylist(RadioItem*)));
  connect(service, SIGNAL(AddItemsToPlaylist(PlaylistItemList)), SIGNAL(AddItemsToPlaylist(PlaylistItemList)));
}

RadioService* RadioModel::ServiceByName(const QString& name) {
  if (sServices.contains(name))
    return sServices[name];
  return NULL;
}

QVariant RadioModel::data(const QModelIndex& index, int role) const {
  const RadioItem* item = IndexToItem(index);

  return data(item, role);
}

QVariant RadioModel::data(const RadioItem* item, int role) const {
  switch (role) {
    case Qt::DisplayRole:
      return item->DisplayText();

    case Qt::DecorationRole:
      return item->icon;
      break;

    case Role_Type:
      return item->type;

    case Role_Key:
      return item->key;

    case Role_SortText:
      return item->SortText();
  }
  return QVariant();
}

void RadioModel::LazyPopulate(RadioItem* parent) {
  if (parent->service)
    parent->service->LazyPopulate(parent);
}

Qt::ItemFlags RadioModel::flags(const QModelIndex& index) const {
  RadioItem* item = IndexToItem(index);
  if (item->playable)
    return Qt::ItemIsSelectable |
           Qt::ItemIsEnabled |
           Qt::ItemIsDragEnabled;

  return Qt::ItemIsSelectable |
         Qt::ItemIsEnabled;
}

QStringList RadioModel::mimeTypes() const {
  return QStringList() << "text/uri-list";
}

QMimeData* RadioModel::mimeData(const QModelIndexList& indexes) const {
  QList<QUrl> urls;
  QList<RadioItem*> items;

  foreach (const QModelIndex& index, indexes) {
    RadioItem* item = IndexToItem(index);
    if (!item || !item->service || !item->playable)
      continue;

    items << item;
    urls << item->service->UrlForItem(item);
  }

  if (urls.isEmpty())
    return NULL;

  RadioMimeData* data = new RadioMimeData;
  data->setUrls(urls);
  data->items = items;

  return data;
}

LastFMService* RadioModel::GetLastFMService() const {
  if (sServices.contains(LastFMService::kServiceName))
    return static_cast<LastFMService*>(sServices[LastFMService::kServiceName]);
  return NULL;
}

void RadioModel::ShowContextMenu(RadioItem* item, const QModelIndex& index,
                                 const QPoint& global_pos) {
  if (item->service)
    item->service->ShowContextMenu(item, index, global_pos);
}

void RadioModel::ReloadSettings() {
  foreach (RadioService* service, sServices.values()) {
    service->ReloadSettings();
  }
}
