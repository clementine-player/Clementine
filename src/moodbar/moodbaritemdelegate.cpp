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

#include "moodbaritemdelegate.h"
#include "moodbarloader.h"
#include "moodbarpipeline.h"
#include "moodbarrenderer.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/qhash_qurl.h"
#include "playlist/playlist.h"

#include <QApplication>
#include <QPainter>
#include <QtConcurrentRun>

MoodbarItemDelegate::Data::Data()
  : state_(State_None)
{
}

MoodbarItemDelegate::MoodbarItemDelegate(Application* app, QObject* parent)
  : QItemDelegate(parent),
    app_(app)
{
}

void MoodbarItemDelegate::paint(
    QPainter* painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const {
  QPixmap pixmap = const_cast<MoodbarItemDelegate*>(this)->PixmapForIndex(
        index, option.rect.size());

  if (!pixmap.isNull()) {
    painter->drawPixmap(option.rect, pixmap);
  } else {
    drawBackground(painter, option, index);
  }
}

QPixmap MoodbarItemDelegate::PixmapForIndex(
    const QModelIndex& index, const QSize& size) {
  // Pixmaps are keyed off URL.
  const QUrl url(index.sibling(index.row(), Playlist::Column_Filename).data().toUrl());

  Data* data = data_[url];
  if (!data) {
    data = new Data;
    data_.insert(url, data);
  }

  data->indexes_.insert(index);
  data->desired_size_ = size;

  switch (data->state_) {
  case Data::State_CannotLoad:
  case Data::State_LoadingData:
  case Data::State_LoadingColors:
  case Data::State_LoadingImage:
    return data->pixmap_;

  case Data::State_Loaded:
    // Is the pixmap the right size?
    if (data->pixmap_.size() != size) {
      StartLoadingImage(url, data);
    }

    return data->pixmap_;

  case Data::State_None:
    break;
  }

  // We have to start loading the data from scratch.
  data->state_ = Data::State_LoadingData;

  // Load a mood file for this song and generate some colors from it
  QByteArray bytes;
  MoodbarPipeline* pipeline = NULL;
  switch (app_->moodbar_loader()->Load(url, &bytes, &pipeline)) {
  case MoodbarLoader::CannotLoad:
    data->state_ = Data::State_CannotLoad;
    break;

  case MoodbarLoader::Loaded:
    // We got the data immediately.
    StartLoadingColors(url, bytes, data);
    break;

  case MoodbarLoader::WillLoadAsync:
    // Maybe in a little while.
    NewClosure(pipeline, SIGNAL(Finished(bool)),
               this, SLOT(DataLoaded(QUrl,MoodbarPipeline*)),
               url, pipeline);
    break;
  }

  return QPixmap();
}

bool MoodbarItemDelegate::RemoveFromCacheIfIndexesInvalid(const QUrl& url, Data* data) {
  foreach (const QPersistentModelIndex& index, data->indexes_) {
    if (index.isValid()) {
      return false;
    }
  }

  data_.remove(url);
  return true;
}

void MoodbarItemDelegate::DataLoaded( const QUrl& url, MoodbarPipeline* pipeline) {
  Data* data = data_[url];
  if (!data) {
    return;
  }

  if (RemoveFromCacheIfIndexesInvalid(url, data)) {
    return;
  }

  if (!pipeline->success()) {
    data->state_ = Data::State_CannotLoad;
    return;
  }

  // Load the colors next.
  StartLoadingColors(url, pipeline->data(), data);
}

void MoodbarItemDelegate::StartLoadingColors(
    const QUrl& url, const QByteArray& bytes, Data* data) {
  data->state_ = Data::State_LoadingColors;

  QFutureWatcher<ColorVector>* watcher = new QFutureWatcher<ColorVector>();
  NewClosure(watcher, SIGNAL(finished()),
             this, SLOT(ColorsLoaded(QUrl,QFutureWatcher<ColorVector>*)),
             url, watcher);

  QFuture<ColorVector> future = QtConcurrent::run(MoodbarRenderer::Colors,
      bytes, MoodbarRenderer::Style_Normal, qApp->palette());
  watcher->setFuture(future);
}

void MoodbarItemDelegate::ColorsLoaded(
    const QUrl& url, QFutureWatcher<ColorVector>* watcher) {
  watcher->deleteLater();

  Data* data = data_[url];
  if (!data) {
    return;
  }

  if (RemoveFromCacheIfIndexesInvalid(url, data)) {
    return;
  }

  data->colors_ = watcher->result();

  // Load the image next.
  StartLoadingImage(url, data);
}

void MoodbarItemDelegate::StartLoadingImage(const QUrl& url, Data* data) {
  data->state_ = Data::State_LoadingImage;

  QFutureWatcher<QImage>* watcher = new QFutureWatcher<QImage>();
  NewClosure(watcher, SIGNAL(finished()),
             this, SLOT(ImageLoaded(QUrl,QFutureWatcher<QImage>*)),
             url, watcher);

  QFuture<QImage> future = QtConcurrent::run(MoodbarRenderer::RenderToImage,
      data->colors_, data->desired_size_);
  watcher->setFuture(future);
}

void MoodbarItemDelegate::ImageLoaded(const QUrl& url, QFutureWatcher<QImage>* watcher) {
  watcher->deleteLater();

  Data* data = data_[url];
  if (!data) {
    return;
  }

  if (RemoveFromCacheIfIndexesInvalid(url, data)) {
    return;
  }

  QImage image(watcher->result());

  // If the desired size changed then don't even bother converting the image
  // to a pixmap, just reload it at the new size.
  if (!image.isNull() && data->desired_size_ != image.size()) {
    StartLoadingImage(url, data);
    return;
  }

  data->pixmap_ = QPixmap::fromImage(image);
  data->state_ = Data::State_Loaded;

  // Update all the indices with the new pixmap.
  foreach (const QPersistentModelIndex& index, data->indexes_) {
    if (index.isValid() && index.sibling(index.row(), Playlist::Column_Filename).data().toUrl() == url) {
      const_cast<Playlist*>(reinterpret_cast<const Playlist*>(index.model()))
          ->MoodbarUpdated(index);
    }
  }
}
