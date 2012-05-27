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

#include <QItemDelegate>

class Application;
class MoodbarPipeline;

class QModelIndex;

class MoodbarItemDelegate : public QItemDelegate {
  Q_OBJECT

public:
  MoodbarItemDelegate(Application* app, QObject* parent = 0);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const;

private slots:
  void RequestFinished(MoodbarPipeline* pipeline, const QModelIndex& index,
                       const QUrl& url);

private:
  QPixmap PixmapForIndex(const QModelIndex& index, const QSize& size,
                         const QPalette& palette);

private:
  Application* app_;
};

#endif // MOODBARITEMDELEGATE_H
