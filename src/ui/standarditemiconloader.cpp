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

#include "standarditemiconloader.h"
#include "covers/albumcoverloader.h"

#include <QAbstractItemModel>
#include <QSet>
#include <QStandardItem>

StandardItemIconLoader::StandardItemIconLoader(AlbumCoverLoader* cover_loader,
                                               QObject* parent)
    : QObject(parent), cover_loader_(cover_loader), model_(nullptr) {
  cover_options_.desired_height_ = 16;

  connect(cover_loader_, SIGNAL(ImageLoaded(quint64, QImage)),
          SLOT(ImageLoaded(quint64, QImage)));
}

void StandardItemIconLoader::SetModel(QAbstractItemModel* model) {
  if (model_) {
    disconnect(model_, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
               this, SLOT(RowsAboutToBeRemoved(QModelIndex, int, int)));
  }

  model_ = model;

  connect(model_, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
          SLOT(RowsAboutToBeRemoved(QModelIndex, int, int)));
  connect(model_, SIGNAL(modelAboutToBeReset()), SLOT(ModelReset()));
}

void StandardItemIconLoader::LoadIcon(const QString& art_automatic,
                                      const QString& art_manual,
                                      QStandardItem* for_item) {
  const quint64 id =
      cover_loader_->LoadImageAsync(cover_options_, art_automatic, art_manual);
  pending_covers_[id] = for_item;
}

void StandardItemIconLoader::LoadIcon(const Song& song,
                                      QStandardItem* for_item) {
  const quint64 id = cover_loader_->LoadImageAsync(cover_options_, song);
  pending_covers_[id] = for_item;
}

void StandardItemIconLoader::RowsAboutToBeRemoved(const QModelIndex& parent,
                                                  int begin, int end) {
  for (QMap<quint64, QStandardItem*>::iterator it = pending_covers_.begin();
       it != pending_covers_.end();) {
    const QStandardItem* item = it.value();
    const QStandardItem* item_parent = item->parent();

    if (item_parent && item_parent->index() == parent &&
        item->index().row() >= begin && item->index().row() <= end) {
      cover_loader_->CancelTask(it.key());
      it = pending_covers_.erase(it);
    } else {
      ++it;
    }
  }
}

void StandardItemIconLoader::ModelReset() {
  cover_loader_->CancelTasks(QSet<quint64>::fromList(pending_covers_.keys()));
  pending_covers_.clear();
}

void StandardItemIconLoader::ImageLoaded(quint64 id, const QImage& image) {
  QStandardItem* item = pending_covers_.take(id);
  if (!item) return;

  if (!image.isNull()) {
    item->setIcon(QIcon(QPixmap::fromImage(image)));
  }
}
