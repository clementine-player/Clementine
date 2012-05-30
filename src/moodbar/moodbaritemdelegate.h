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

#ifndef MOODBARITEMDELEGATE_H
#define MOODBARITEMDELEGATE_H

#include "moodbarrenderer.h"

#include <QCache>
#include <QItemDelegate>
#include <QFutureWatcher>
#include <QUrl>

class Application;
class MoodbarPipeline;
class PlaylistView;

class QModelIndex;

class MoodbarItemDelegate : public QItemDelegate {
  Q_OBJECT

public:
  MoodbarItemDelegate(Application* app, PlaylistView* view, QObject* parent = 0);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const;

private slots:
  void ReloadSettings();

  void DataLoaded(const QUrl& url, MoodbarPipeline* pipeline);
  void ColorsLoaded(const QUrl& url, QFutureWatcher<ColorVector>* watcher);
  void ImageLoaded(const QUrl& url, QFutureWatcher<QImage>* watcher);

private:
  struct Data {
    Data();

    enum State {
      State_None,
      State_CannotLoad,
      State_LoadingData,
      State_LoadingColors,
      State_LoadingImage,
      State_Loaded
    };

    QSet<QPersistentModelIndex> indexes_;

    State state_;
    ColorVector colors_;
    QSize desired_size_;
    QPixmap pixmap_;
  };

private:
  QPixmap PixmapForIndex(const QModelIndex& index, const QSize& size);
  void StartLoadingData(const QUrl& url, Data* data);
  void StartLoadingColors(const QUrl& url, const QByteArray& bytes, Data* data);
  void StartLoadingImage(const QUrl& url, Data* data);

  bool RemoveFromCacheIfIndexesInvalid(const QUrl& url, Data* data);

  void ReloadAllColors();

private:
  Application* app_;
  PlaylistView* view_;
  QCache<QUrl, Data> data_;

  MoodbarRenderer::MoodbarStyle style_;
};

#endif // MOODBARITEMDELEGATE_H
