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

#ifndef RADIOSERVICE_H
#define RADIOSERVICE_H

#include <QObject>
#include <QList>
#include <QUrl>

#include "core/song.h"
#include "playlist/playlistitem.h"
#include "ui/settingsdialog.h"
#include "widgets/multiloadingindicator.h"

class RadioModel;
class LibraryFilterWidget;

class RadioService : public QObject {
  Q_OBJECT

public:
  RadioService(const QString& name, RadioModel* model);
  virtual ~RadioService() {}

  QString name() const { return name_; }
  RadioModel* model() const { return model_; }

  virtual QStandardItem* CreateRootItem() = 0;
  virtual void LazyPopulate(QStandardItem* parent) = 0;

  virtual void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos) {
    Q_UNUSED(index); Q_UNUSED(global_pos); }

  virtual PlaylistItem::SpecialLoadResult StartLoading(const QUrl& url);
  virtual PlaylistItem::SpecialLoadResult LoadNext(const QUrl& url);

  virtual PlaylistItem::Options playlistitem_options() const { return PlaylistItem::Default; }

  virtual QWidget* HeaderWidget() const { return NULL; }

  virtual void ReloadSettings() {}

  virtual QString Icon() { return QString(); }

signals:
  void AsyncLoadFinished(const PlaylistItem::SpecialLoadResult& result);
  void StreamError(const QString& message);
  void StreamMetadataFound(const QUrl& original_url, const Song& song);
  void OpenSettingsAtPage(SettingsDialog::Page page);

  void AddToPlaylistSignal(QMimeData* data);

protected:
  // Describes how songs should be added to playlist.
  enum AddMode {
    // appends songs to the current playlist
    AddMode_Append,
    // clears the current playlist and then appends all songs to it
    AddMode_Replace,
    // creates a new, empty playlist and then adds all songs to it
    AddMode_OpenInNew
  };

  // Adds the 'index' element to playlist using the 'add_mode' mode.
  void AddItemToPlaylist(const QModelIndex& index, AddMode add_mode);
  // Adds the 'indexes' elements to playlist using the 'add_mode' mode.
  void AddItemsToPlaylist(const QModelIndexList& indexes, AddMode add_mode);

private:
  RadioModel* model_;
  QString name_;
};

Q_DECLARE_METATYPE(RadioService*);

#endif // RADIOSERVICE_H
