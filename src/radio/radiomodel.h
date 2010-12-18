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

#ifndef RADIOMODEL_H
#define RADIOMODEL_H

#include "radioitem.h"
#include "core/backgroundthread.h"
#include "core/simpletreemodel.h"
#include "core/song.h"
#include "playlist/playlistitem.h"
#include "ui/settingsdialog.h"
#include "widgets/multiloadingindicator.h"

class Database;
class MergedProxyModel;
class RadioService;
class SettingsDialog;
class TaskManager;

#ifdef HAVE_LIBLASTFM
  class LastFMService;
#endif

class RadioModel : public SimpleTreeModel<RadioItem> {
  Q_OBJECT

 public:
  RadioModel(BackgroundThread<Database>* db_thread, TaskManager* task_manager,
             QObject* parent = 0);

  enum {
    Role_Type = Qt::UserRole + 1,
    Role_SortText,
    Role_Key,
  };

  // Needs to be static for RadioPlaylistItem::restore
  static RadioService* ServiceByName(const QString& name);

  template<typename T>
  static T* Service() {
    if (sServices.contains(T::kServiceName)) {
      return static_cast<T*>(sServices[T::kServiceName]);
    }
    return NULL;
  }

  // This is special because Player needs it for scrobbling
#ifdef HAVE_LIBLASTFM
  LastFMService* GetLastFMService() const;
#endif

  // QAbstractItemModel
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;

  void ShowContextMenu(RadioItem* item, const QModelIndex& index,
                       const QPoint& global_pos);
  void ReloadSettings();

  BackgroundThread<Database>* db_thread() const { return db_thread_; }
  MergedProxyModel* merged_model() const { return merged_model_; }
  TaskManager* task_manager() const { return task_manager_; }

 signals:
  void AsyncLoadFinished(const PlaylistItem::SpecialLoadResult& result);
  void StreamError(const QString& message);
  void StreamMetadataFound(const QUrl& original_url, const Song& song);
  void OpenSettingsAtPage(SettingsDialog::Page);

  void AddItemToPlaylist(RadioItem* item, bool clear_first);
  void AddItemsToPlaylist(const PlaylistItemList& items, bool clear_first);

 protected:
  void LazyPopulate(RadioItem* parent);

 private:
  QVariant data(const RadioItem* item, int role) const;
  void AddService(RadioService* service);

 private:
  static QMap<QString, RadioService*> sServices;
  BackgroundThread<Database>* db_thread_;
  MergedProxyModel* merged_model_;
  TaskManager* task_manager_;
};

#endif // RADIOMODEL_H
