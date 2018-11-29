/* This file is part of Clementine.
   Copyright 2009-2014, David Sansome <me@davidsansome.com>
   Copyright 2011-2012, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2012-2013, Alan Briolat <alan.briolat@gmail.com>
   Copyright 2012-2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Maltsev Vlad <shedwardx@gmail.com>
   Copyright 2014, Chocobozzz <florian.bigard@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "internet/core/internetmodel.h"

#include <QMimeData>
#include <QtDebug>

#include "core/closure.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "internet/core/internetmimedata.h"
#include "internet/core/internetservice.h"
#include "internet/digitally/digitallyimportedservicebase.h"
#include "internet/icecast/icecastservice.h"
#include "internet/intergalacticfm/intergalacticfmservice.h"
#include "internet/internetradio/savedradio.h"
#include "internet/jamendo/jamendoservice.h"
#include "internet/magnatune/magnatuneservice.h"
#include "internet/podcasts/podcastservice.h"
#include "internet/somafm/somafmservice.h"
#include "internet/subsonic/subsonicservice.h"
#include "smartplaylists/generatormimedata.h"

#ifdef HAVE_GOOGLE_DRIVE
#include "internet/googledrive/googledriveservice.h"
#endif
#ifdef HAVE_DROPBOX
#include "internet/dropbox/dropboxservice.h"
#endif
#ifdef HAVE_SKYDRIVE
#include "internet/skydrive/skydriveservice.h"
#endif
#ifdef HAVE_BOX
#include "internet/box/boxservice.h"
#endif
#ifdef HAVE_SEAFILE
#include "internet/seafile/seafileservice.h"
#endif
#ifdef HAVE_SPOTIFY
#include "internet/spotify/spotifyservice.h"
#endif

using smart_playlists::Generator;
using smart_playlists::GeneratorMimeData;
using smart_playlists::GeneratorPtr;

QMap<QString, InternetService*>* InternetModel::sServices = nullptr;

const char* InternetModel::kSettingsGroup = "InternetModel";

InternetModel::InternetModel(Application* app, QObject* parent)
    : QStandardItemModel(parent),
      app_(app),
      merged_model_(new MergedProxyModel(this)) {
  if (!sServices) {
    sServices = new QMap<QString, InternetService*>;
  }
  Q_ASSERT(sServices->isEmpty());

  merged_model_->setSourceModel(this);

  AddService(new ClassicalRadioService(app, this));
  AddService(new DigitallyImportedService(app, this));
  AddService(new IcecastService(app, this));
  AddService(new JamendoService(app, this));
  AddService(new JazzRadioService(app, this));
  AddService(new MagnatuneService(app, this));
  AddService(new PodcastService(app, this));
  AddService(new RockRadioService(app, this));
  AddService(new SavedRadio(app, this));
  AddService(new RadioTunesService(app, this));
  AddService(new SomaFMService(app, this));
  AddService(new IntergalacticFMService(app, this));
#ifdef HAVE_SPOTIFY
  AddService(new SpotifyService(app, this));
#endif
  AddService(new SubsonicService(app, this));
#ifdef HAVE_BOX
  AddService(new BoxService(app, this));
#endif
#ifdef HAVE_DROPBOX
  AddService(new DropboxService(app, this));
#endif
#ifdef HAVE_GOOGLE_DRIVE
  AddService(new GoogleDriveService(app, this));
#endif
#ifdef HAVE_SEAFILE
  AddService(new SeafileService(app, this));
#endif
#ifdef HAVE_SKYDRIVE
  AddService(new SkydriveService(app, this));
#endif

  invisibleRootItem()->sortChildren(0, Qt::AscendingOrder);
  UpdateServices();
}

void InternetModel::AddService(InternetService* service) {
  QStandardItem* root = service->CreateRootItem();
  if (!root) {
    qLog(Warning) << "Internet service" << service->name()
                  << "did not return a root item";
    return;
  }

  root->setData(Type_Service, Role_Type);
  root->setData(QVariant::fromValue(service), Role_Service);

  invisibleRootItem()->appendRow(root);
  qLog(Debug) << "Adding internet service:" << service->name();
  sServices->insert(service->name(), service);

  ServiceItem service_item;
  service_item.item = root;
  service_item.shown = true;

  shown_services_.insert(service, service_item);

  connect(service, SIGNAL(StreamError(QString)), SIGNAL(StreamError(QString)));
  connect(service, SIGNAL(StreamMetadataFound(QUrl, Song)),
          SIGNAL(StreamMetadataFound(QUrl, Song)));
  connect(service, SIGNAL(AddToPlaylistSignal(QMimeData*)),
          SIGNAL(AddToPlaylist(QMimeData*)));
  connect(service, SIGNAL(ScrollToIndex(QModelIndex)),
          SIGNAL(ScrollToIndex(QModelIndex)));
  connect(service, SIGNAL(destroyed()), SLOT(ServiceDeleted()));

  if (service->has_initial_load_settings()) {
    service->InitialLoadSettings();
  } else {
    service->ReloadSettings();
  }
}

void InternetModel::RemoveService(InternetService* service) {
  if (!sServices->contains(service->name())) return;

  // Find and remove the root item that this service created
  for (int i = 0; i < invisibleRootItem()->rowCount(); ++i) {
    QStandardItem* item = invisibleRootItem()->child(i);
    if (!item ||
        item->data(Role_Service).value<InternetService*>() == service) {
      invisibleRootItem()->removeRow(i);
      break;
    }
  }

  // Remove the service from the list
  sServices->remove(service->name());

  // Don't forget to delete from shown_services too
  shown_services_.remove(service);

  // Disconnect the service
  disconnect(service, 0, this, 0);
}

void InternetModel::ServiceDeleted() {
  InternetService* service = qobject_cast<InternetService*>(sender());
  if (service) RemoveService(service);
}

InternetService* InternetModel::ServiceByName(const QString& name) {
  if (sServices->contains(name)) return sServices->value(name);
  return nullptr;
}

InternetService* InternetModel::ServiceForItem(
    const QStandardItem* item) const {
  return ServiceForIndex(indexFromItem(item));
}

InternetService* InternetModel::ServiceForIndex(
    const QModelIndex& index) const {
  QModelIndex current_index = index;
  while (current_index.isValid()) {
    InternetService* service =
        current_index.data(Role_Service).value<InternetService*>();
    if (service) {
      return service;
    }
    current_index = current_index.parent();
  }
  return nullptr;
}

Qt::ItemFlags InternetModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags flags =
      Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
  if (IsPlayable(index)) {
    flags |= Qt::ItemIsDragEnabled;
  }
  return flags;
}

bool InternetModel::hasChildren(const QModelIndex& parent) const {
  if (parent.data(Role_CanLazyLoad).toBool()) return true;
  return QStandardItemModel::hasChildren(parent);
}

int InternetModel::rowCount(const QModelIndex& parent) const {
  if (parent.data(Role_CanLazyLoad).toBool()) {
    QStandardItem* item = itemFromIndex(parent);
    InternetService* service = ServiceForItem(item);
    if (service) {
      item->setData(false, Role_CanLazyLoad);
      service->LazyPopulate(item);
    }
  }

  return QStandardItemModel::rowCount(parent);
}

bool InternetModel::IsPlayable(const QModelIndex& index) const {
  QVariant behaviour = index.data(Role_PlayBehaviour);
  if (!behaviour.isValid()) return false;

  PlayBehaviour pb = PlayBehaviour(behaviour.toInt());
  return (pb == PlayBehaviour_MultipleItems || pb == PlayBehaviour_SingleItem ||
          pb == PlayBehaviour_UseSongLoader);
}

QStringList InternetModel::mimeTypes() const {
  return QStringList() << "text/uri-list";
}

QMimeData* InternetModel::mimeData(const QModelIndexList& indexes) const {
  // Special case for when the user double clicked on a special item.
  if (indexes.count() == 1 &&
      indexes[0].data(Role_PlayBehaviour).toInt() ==
          PlayBehaviour_DoubleClickAction) {
    InternetModel::ServiceForIndex(indexes[0])
        ->ItemDoubleClicked(itemFromIndex(indexes[0]));
    return nullptr;
  }

  if (indexes.count() == 1 &&
      indexes[0].data(Role_Type).toInt() == Type_SmartPlaylist) {
    GeneratorPtr generator = InternetModel::ServiceForIndex(indexes[0])
                                 ->CreateGenerator(itemFromIndex(indexes[0]));
    if (!generator) return nullptr;
    GeneratorMimeData* data = new GeneratorMimeData(generator);
    data->setData(LibraryModel::kSmartPlaylistsMimeType, QByteArray());
    data->name_for_new_playlist_ = this->data(indexes.first()).toString();
    return data;
  }

  QList<QUrl> urls;
  QModelIndexList new_indexes;

  QModelIndex last_valid_index;
  for (const QModelIndex& index : indexes) {
    if (!IsPlayable(index)) continue;

    last_valid_index = index;
    if (index.data(Role_PlayBehaviour).toInt() == PlayBehaviour_MultipleItems) {
      // Get children
      int row = 0;
      int column = 0;
      QModelIndex child = index.child(row, column);
      while (child.isValid()) {
        new_indexes << child;
        urls << child.data(Role_Url).toUrl();
        child = index.child(++row, column);
      }
    } else {
      new_indexes = indexes;
      urls << index.data(Role_Url).toUrl();
    }
  }

  if (urls.isEmpty()) return nullptr;

  InternetMimeData* data = new InternetMimeData(this);
  data->setUrls(urls);
  data->indexes = new_indexes;
  data->name_for_new_playlist_ =
      InternetModel::ServiceForIndex(last_valid_index)->name();

  return data;
}

bool InternetModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                 int row, int column,
                                 const QModelIndex& parent) {
  if (action == Qt::IgnoreAction) {
    return false;
  }
  if (parent.data(Role_CanBeModified).toBool() ||
      parent.parent().data(Role_CanBeModified).toBool()) {
    InternetModel::ServiceForIndex(parent)->DropMimeData(data, parent);
  }

  return true;
}

void InternetModel::ShowContextMenu(
    const QModelIndexList& selected_merged_model_indexes,
    const QModelIndex& current_merged_model_index, const QPoint& global_pos) {
  current_index_ = merged_model_->mapToSource(current_merged_model_index);

  selected_indexes_.clear();
  for (const QModelIndex& index : selected_merged_model_indexes) {
    selected_indexes_ << merged_model_->mapToSource(index);
  }

  InternetService* service = ServiceForIndex(current_merged_model_index);
  if (service) service->ShowContextMenu(global_pos);
}

void InternetModel::ReloadSettings() {
  for (InternetService* service : sServices->values()) {
    service->ReloadSettings();
  }
}

void InternetModel::UpdateServices() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  QStringList keys = s.childKeys();

  for (const QString& service_name : keys) {
    InternetService* internet_service = ServiceByName(service_name);
    if (internet_service == nullptr) {
      continue;
    }
    bool setting_val = s.value(service_name).toBool();

    // Only update if values are different
    if (setting_val == true &&
        shown_services_[internet_service].shown == false) {
      ShowService(internet_service);
    } else if (setting_val == false &&
               shown_services_[internet_service].shown == true) {
      HideService(internet_service);
    }
  }

  s.endGroup();
}

int InternetModel::FindItemPosition(const QString& text) {
  int a = 0;
  int b = invisibleRootItem()->rowCount() - 1;
  while (a <= b) {
    int mid = a + (b - a) / 2;
    if (invisibleRootItem()->child(mid, 0)->text() < text) {
      a = mid + 1;
    } else {
      b = mid - 1;
    }
  }
  return a;
}

void InternetModel::ShowService(InternetService* service) {
  if (shown_services_[service].shown != true) {
    QStandardItem* item = shown_services_[service].item;
    int pos = FindItemPosition(item->text());
    invisibleRootItem()->insertRow(pos, item);
    shown_services_[service].shown = true;
  }
}

void InternetModel::HideService(InternetService* service) {
  // Find and remove the root item that this service created
  for (int i = 0; i < invisibleRootItem()->rowCount(); ++i) {
    QStandardItem* item = invisibleRootItem()->child(i);
    if (!item ||
        item->data(Role_Service).value<InternetService*>() == service) {
      // Don't remove the standarditem behind the row
      invisibleRootItem()->takeRow(i);
      break;
    }
  }

  shown_services_[service].shown = false;
}
