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

#ifndef INTERNETMODEL_H
#define INTERNETMODEL_H

#include "core/song.h"
#include "library/librarymodel.h"
#include "playlist/playlistitem.h"
#include "ui/settingsdialog.h"
#include "widgets/multiloadingindicator.h"

class Application;
class CoverProviders;
class Database;
class GlobalSearch;
class MergedProxyModel;
class PlayerInterface;
class InternetService;
class SettingsDialog;
class TaskManager;

#ifdef HAVE_LIBLASTFM
class LastFMService;
#endif

class InternetModel : public QStandardItemModel {
  Q_OBJECT

 public:
  InternetModel(Application* app, QObject* parent = nullptr);

  enum Role {
    // Services can use this role to distinguish between different types of
    // items that they add.  The root item's type is automatically set to
    // Type_Service, but apart from that Services are free to define their own
    // values for this field (starting from TypeCount).
    Role_Type = Qt::UserRole + 1000,

    // If this is not set the item is not playable (ie. it can't be dragged to
    // the playlist).  Otherwise it describes how this item is converted to
    // playlist items.  See the PlayBehaviour enum for more details.
    Role_PlayBehaviour,

    // The URL of the media for this item.  This is required if the
    // PlayBehaviour is set to PlayBehaviour_UseSongLoader.
    Role_Url,

    // The metadata used in the item that is added to the playlist if the
    // PlayBehaviour is set to PlayBehaviour_SingleItem.  Ignored otherwise.
    Role_SongMetadata,

    // If this is set to true then the model will call the service's
    // LazyPopulate method when this item is expanded.  Use this if your item's
    // children have to be downloaded or fetched remotely.
    Role_CanLazyLoad,

    // This is automatically set on the root item for a service.  It contains
    // a pointer to an InternetService.  Services should not set this field
    // themselves.
    Role_Service,

    // Setting this to true means that the item can be changed by user action
    // (e.g. changing remote playlists)
    Role_CanBeModified,
    RoleCount,
    Role_IsDivider = LibraryModel::Role_IsDivider,
  };

  enum Type {
    Type_Service = 1,
    Type_Track,
    Type_UserPlaylist,
    Type_SmartPlaylist,
    TypeCount
  };

  enum PlayBehaviour {
    // The item can't be played.  This is the default.
    PlayBehaviour_None = 0,

    // This item's URL is passed through the normal song loader.  This supports
    // loading remote playlists, remote files and local files.  This is probably
    // the most sensible behaviour to use if you're just returning normal radio
    // stations.
    PlayBehaviour_UseSongLoader,

    // This item's URL, Title and Artist are used in the playlist.  No special
    // behaviour occurs - the URL is just passed straight to gstreamer when
    // the user starts playing.
    PlayBehaviour_SingleItem,

    // This item's children have PlayBehaviour_SingleItem set.
    // This is used when dragging a playlist item for instance, to have all the
    // playlit's items info loaded in the mime data.
    PlayBehaviour_MultipleItems,

    // This item might not represent a song - the service's ItemDoubleClicked()
    // slot will get called instead to do some custom action.
    PlayBehaviour_DoubleClickAction,
  };

  struct ServiceItem {
    QStandardItem* item;
    bool shown;
  };

  // Needs to be static for InternetPlaylistItem::restore
  static InternetService* ServiceByName(const QString& name);
  static const char* kSettingsGroup;

  template <typename T>
  static T* Service() {
    return static_cast<T*>(ServiceByName(T::kServiceName));
  }

  // Add and remove services.  Ownership is not transferred and the service
  // is not reparented.  If the service is deleted it will be automatically
  // removed from the model.
  void AddService(InternetService* service);
  void RemoveService(InternetService* service);
  void HideService(InternetService* service);
  void ShowService(InternetService* service);
  // Add or remove the services according to the setting file
  void UpdateServices();

  // Returns the service that is a parent of this item.  Works by walking up
  // the tree until it finds an item with Role_Service set.
  InternetService* ServiceForItem(const QStandardItem* item) const;
  InternetService* ServiceForIndex(const QModelIndex& index) const;

  // Returns true if the given item has a PlayBehaviour other than None.
  bool IsPlayable(const QModelIndex& index) const;

  // QAbstractItemModel
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                    int column, const QModelIndex& parent);
  bool hasChildren(const QModelIndex& parent) const;
  int rowCount(const QModelIndex& parent) const;

  void ShowContextMenu(const QModelIndexList& selected_merged_model_indexes,
                       const QModelIndex& current_merged_model_index,
                       const QPoint& global_pos);
  void ReloadSettings();

  Application* app() const { return app_; }
  MergedProxyModel* merged_model() const { return merged_model_; }

  const QModelIndex& current_index() const { return current_index_; }
  const QModelIndexList& selected_indexes() const { return selected_indexes_; }
  const QMap<InternetService*, ServiceItem> shown_services() const {
    return shown_services_;
  }

signals:
  void StreamError(const QString& message);
  void StreamMetadataFound(const QUrl& original_url, const Song& song);

  void AddToPlaylist(QMimeData* data);
  void ScrollToIndex(const QModelIndex& index);

 private slots:
  void ServiceDeleted();

 private:
  QMap<InternetService*, ServiceItem> shown_services_;

  static QMap<QString, InternetService*>* sServices;

  Application* app_;
  MergedProxyModel* merged_model_;

  // Set when a context menu is requested, can be accessed by context menu
  // actions to do things to the current item.
  QModelIndexList selected_indexes_;
  QModelIndex current_index_;
};

#endif  // INTERNETMODEL_H
