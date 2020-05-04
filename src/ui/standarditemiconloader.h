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

#ifndef STANDARDITEMICONLOADER_H
#define STANDARDITEMICONLOADER_H

#include "covers/albumcoverloaderoptions.h"

#include <QMap>
#include <QObject>

class AlbumCoverLoader;
class Song;

class QStandardItemModel;
class QModelIndex;
class QStandardItem;

// Uses an AlbumCoverLoader to asynchronously load and set an icon on a
// QStandardItem.
class StandardItemIconLoader : public QObject {
  Q_OBJECT

 public:
  StandardItemIconLoader(AlbumCoverLoader* cover_loader, QObject* parent = nullptr);

  AlbumCoverLoaderOptions* options() { return &cover_options_; }

  void SetModel(QStandardItemModel* model);

  void LoadIcon(const QString& art_automatic, const QString& art_manual,
                QStandardItem* for_item);
  void LoadIcon(const Song& song, QStandardItem* for_item);

 private slots:
  void ImageLoaded(quint64 id, const QImage& image);
  void RowsAboutToBeRemoved(const QModelIndex& parent, int begin, int end);
  void ModelReset();

 private:
  AlbumCoverLoader* cover_loader_;
  AlbumCoverLoaderOptions cover_options_;

  QStandardItemModel* model_;

  QMap<quint64, QStandardItem*> pending_covers_;
};

#endif  // STANDARDITEMICONLOADER_H
