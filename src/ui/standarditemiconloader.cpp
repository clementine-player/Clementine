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

#include <QSet>
#include <QStandardItem>
#include <QStandardItemModel>

#include "covers/albumcoverloader.h"

StandardItemIconLoader::StandardItemIconLoader(AlbumCoverLoader* cover_loader,
                                               QObject* parent)
    : QObject(parent), cover_loader_(cover_loader), model_(nullptr) {
  cover_options_.desired_height_ = 16;

  connect(cover_loader_, SIGNAL(ImageLoaded(quint64, QImage)),
          SLOT(ImageLoaded(quint64, QImage)));
}

void StandardItemIconLoader::SetModel(QStandardItemModel* model) {
  // Cancel pending tasks and disconnect from the previous model so stale
  // QStandardItem* pointers cannot fire ImageLoaded after switching models.
  ModelReset();
  if (model_) {
    disconnect(model_, nullptr, this, nullptr);
  }

  model_ = model;

  if (model_) {
    connect(model_, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
            SLOT(RowsAboutToBeRemoved(QModelIndex, int, int)));
    connect(model_, SIGNAL(modelAboutToBeReset()), SLOT(ModelReset()));
  }
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
  // For QStandardItemModel, the invisible root item does not have a valid
  // index.
  bool is_top = !parent.isValid();

  for (QMap<quint64, QStandardItem*>::iterator it = pending_covers_.begin();
       it != pending_covers_.end();) {
    // rowsAboutToBeRemoved only notifies for the removed parent row range,
    // not recursively for descendants. Walk up each pending item's ancestor
    // chain so deleting a parent node cancels covers for its entire subtree.
    bool will_be_removed = false;
    for (const QStandardItem* cur = it.value(); cur != nullptr;
         cur = cur->parent()) {
      const QStandardItem* cur_parent = cur->parent();
      const bool matches_parent =
          (is_top && cur_parent == nullptr) ||
          (cur_parent != nullptr && cur_parent->index() == parent);
      if (matches_parent && cur->row() >= begin && cur->row() <= end) {
        will_be_removed = true;
        break;
      }
    }

    if (will_be_removed) {
      cover_loader_->CancelTask(it.key());
      it = pending_covers_.erase(it);
    } else {
      ++it;
    }
  }
}

void StandardItemIconLoader::ModelReset() {
  const QList<quint64> keys = pending_covers_.keys();
  cover_loader_->CancelTasks(QSet<quint64>(keys.begin(), keys.end()));
  pending_covers_.clear();
}

void StandardItemIconLoader::ImageLoaded(quint64 id, const QImage& image) {
  QStandardItem* item = pending_covers_.take(id);
  if (!item) return;

  if (!image.isNull()) {
    item->setIcon(QIcon(QPixmap::fromImage(image)));
  }
}
