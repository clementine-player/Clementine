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
#include "playlist/playlist.h"

#include <QApplication>
#include <QPainter>

MoodbarItemDelegate::MoodbarItemDelegate(Application* app, QObject* parent)
  : QItemDelegate(parent),
    app_(app)
{
}

void MoodbarItemDelegate::paint(
    QPainter* painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const {
  QPixmap pixmap = const_cast<MoodbarItemDelegate*>(this)->PixmapForIndex(
        index, option.rect.size(), option.palette);
  if (!pixmap.isNull()) {
    painter->drawPixmap(option.rect, pixmap);
  }
}

QPixmap MoodbarItemDelegate::PixmapForIndex(
    const QModelIndex& index, const QSize& size, const QPalette& palette) {
  // Do we have a pixmap already that's the right size?
  QPixmap pixmap = index.data(Playlist::Role_MoodbarPixmap).value<QPixmap>();
  if (!pixmap.isNull() && pixmap.size() == size) {
    return pixmap;
  }

  // Do we have colors?
  ColorVector colors = index.data(Playlist::Role_MoodbarColors).value<ColorVector>();
  if (colors.isEmpty()) {
    // Nope - we need to load a mood file for this song and generate some colors
    // from it.
    const QUrl url(index.sibling(index.row(), Playlist::Column_Filename).data().toUrl());

    QByteArray data;
    MoodbarPipeline* pipeline = NULL;
    switch (app_->moodbar_loader()->Load(url, &data, &pipeline)) {
    case MoodbarLoader::CannotLoad:
      return QPixmap();

    case MoodbarLoader::Loaded:
      // Aww yeah
      colors = MoodbarRenderer::Colors(data, MoodbarRenderer::Style_Normal, palette);
      break;

    case MoodbarLoader::WillLoadAsync:
      // Maybe in a little while.
      qLog(Debug) << "Loading" << pipeline;
      NewClosure(pipeline, SIGNAL(Finished(bool)),
                 this, SLOT(RequestFinished(MoodbarPipeline*,QModelIndex,QUrl)),
                 pipeline, index, url);
      return QPixmap();
    }
  }

  // We've got colors, let's make a pixmap.
  pixmap = QPixmap(size);
  QPainter p(&pixmap);
  MoodbarRenderer::Render(colors, &p, QRect(QPoint(0, 0), size));
  p.end();

  // Set these on the item so we don't have to look them up again.
  QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
  model->setData(index, QVariant::fromValue(colors), Playlist::Role_MoodbarColors);
  model->setData(index, pixmap, Playlist::Role_MoodbarPixmap);

  return pixmap;
}

void MoodbarItemDelegate::RequestFinished(
    MoodbarPipeline* pipeline, const QModelIndex& index, const QUrl& url) {
  qLog(Debug) << "Finished" << pipeline;
  // Is this index still valid, and does it still point to the same URL?
  if (!index.isValid() || index.sibling(index.row(), Playlist::Column_Filename).data().toUrl() != url) {
    return;
  }

  // It's good.  Create the color list and set them on the item.
  ColorVector colors = MoodbarRenderer::Colors(
        pipeline->data(), MoodbarRenderer::Style_Normal, qApp->palette());
  QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
  model->setData(index, QVariant::fromValue(colors), Playlist::Role_MoodbarColors);
}
