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

#include "core/mergedproxymodel.h"
#include "icecastservice.h"
#include "jamendoservice.h"
#include "magnatuneservice.h"
#include "radiomimedata.h"
#include "radiomodel.h"
#include "radioservice.h"
#include "savedradio.h"
#include "somafmservice.h"

#ifdef HAVE_LIBLASTFM
  #include "lastfmservice.h"
#endif

#include <QMimeData>
#include <QtDebug>

QMap<QString, RadioService*>* RadioModel::sServices = NULL;

RadioModel::RadioModel(BackgroundThread<Database>* db_thread,
                       TaskManager* task_manager, QObject* parent)
  : QStandardItemModel(parent),
    db_thread_(db_thread),
    merged_model_(new MergedProxyModel(this)),
    task_manager_(task_manager)
{
  if (!sServices) {
    sServices = new QMap<QString, RadioService*>;
  }
  Q_ASSERT(sServices->isEmpty());

  merged_model_->setSourceModel(this);

#ifdef HAVE_LIBLASTFM
  AddService(new LastFMService(this));
#endif
  AddService(new SomaFMService(this));
  AddService(new MagnatuneService(this));
  AddService(new JamendoService(this));
  AddService(new IcecastService(this));
  AddService(new SavedRadio(this));
}

void RadioModel::AddService(RadioService *service) {
  QStandardItem* root = service->CreateRootItem();
  if (!root) {
    qWarning() << "Radio service" << service->name() << "did not return a root item";
    return;
  }

  root->setData(Type_Service, Role_Type);
  root->setData(QVariant::fromValue(service), Role_Service);

  invisibleRootItem()->appendRow(root);
  sServices->insert(service->name(), service);

  connect(service, SIGNAL(AsyncLoadFinished(PlaylistItem::SpecialLoadResult)), SIGNAL(AsyncLoadFinished(PlaylistItem::SpecialLoadResult)));
  connect(service, SIGNAL(StreamError(QString)), SIGNAL(StreamError(QString)));
  connect(service, SIGNAL(StreamMetadataFound(QUrl,Song)), SIGNAL(StreamMetadataFound(QUrl,Song)));
  connect(service, SIGNAL(OpenSettingsAtPage(SettingsDialog::Page)), SIGNAL(OpenSettingsAtPage(SettingsDialog::Page)));
  connect(service, SIGNAL(AddToPlaylistSignal(QMimeData*)), SIGNAL(AddToPlaylist(QMimeData*)));
  connect(service, SIGNAL(destroyed()), SLOT(ServiceDeleted()));
}

void RadioModel::RemoveService(RadioService* service) {
  if (!sServices->contains(service->name()))
    return;

  // Find and remove the root item that this service created
  for (int i=0 ; i<invisibleRootItem()->rowCount() ; ++i) {
    if (invisibleRootItem()->child(i)->data(Role_Service).value<RadioService*>()
        == service) {
      invisibleRootItem()->removeRow(i);
      break;
    }
  }

  // Remove the service from the list
  sServices->remove(service->name());

  // Disconnect the service
  disconnect(service, 0, this, 0);
}

void RadioModel::ServiceDeleted() {
  // qobject_cast doesn't work here with services created by python
  RadioService* service = static_cast<RadioService*>(sender());
  if (service)
    RemoveService(service);
}

RadioService* RadioModel::ServiceByName(const QString& name) {
  if (sServices->contains(name))
    return sServices->value(name);
  return NULL;
}

RadioService* RadioModel::ServiceForItem(const QStandardItem* item) const {
  return ServiceForIndex(indexFromItem(item));
}

RadioService* RadioModel::ServiceForIndex(const QModelIndex& index) const {
  QModelIndex current_index = index;
  while (current_index.isValid()) {
    RadioService* service = current_index.data(Role_Service).value<RadioService*>();
    if (service) {
      return service;
    }
    current_index = current_index.parent();
  }
  return NULL;
}

Qt::ItemFlags RadioModel::flags(const QModelIndex& index) const {
  if (IsPlayable(index))
    return Qt::ItemIsSelectable |
           Qt::ItemIsEnabled |
           Qt::ItemIsDragEnabled;

  return Qt::ItemIsSelectable |
         Qt::ItemIsEnabled;
}

bool RadioModel::hasChildren(const QModelIndex& parent) const {
  if (parent.data(Role_CanLazyLoad).toBool())
    return true;
  return QStandardItemModel::hasChildren(parent);
}

int RadioModel::rowCount(const QModelIndex& parent) const {
  if (parent.data(Role_CanLazyLoad).toBool()) {
    QStandardItem* item = itemFromIndex(parent);
    RadioService* service = ServiceForItem(item);
    if (service) {
      item->setData(false, Role_CanLazyLoad);
      service->LazyPopulate(item);
    }
  }

  return QStandardItemModel::rowCount(parent);
}

bool RadioModel::IsPlayable(const QModelIndex& index) const {
  QVariant behaviour = index.data(Role_PlayBehaviour);
  if (!behaviour.isValid() || behaviour.toInt() == PlayBehaviour_None)
    return false;
  return true;
}

QStringList RadioModel::mimeTypes() const {
  return QStringList() << "text/uri-list";
}

QMimeData* RadioModel::mimeData(const QModelIndexList& indexes) const {
  QList<QUrl> urls;

  foreach (const QModelIndex& index, indexes) {
    if (!IsPlayable(index))
      continue;

    urls << index.data(Role_Url).toUrl();
  }

  if (urls.isEmpty())
    return NULL;

  RadioMimeData* data = new RadioMimeData(this);
  data->setUrls(urls);
  data->indexes = indexes;

  return data;
}

void RadioModel::ShowContextMenu(const QModelIndex& merged_model_index,
                                 const QPoint& global_pos) {
  RadioService* service = ServiceForIndex(merged_model_index);
  if (service)
    service->ShowContextMenu(merged_model_->mapToSource(merged_model_index), global_pos);
}

void RadioModel::ReloadSettings() {
  foreach (RadioService* service, sServices->values()) {
    service->ReloadSettings();
  }
}
