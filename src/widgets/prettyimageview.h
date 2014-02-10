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

#ifndef PRETTYIMAGEVIEW_H
#define PRETTYIMAGEVIEW_H

#include <QMap>
#include <QScrollArea>
#include <QUrl>

class QHBoxLayout;
class QMenu;
class QNetworkAccessManager;
class QNetworkReply;
class QPropertyAnimation;
class QTimeLine;

class PrettyImageView : public QScrollArea {
  Q_OBJECT

 public:
  PrettyImageView(QNetworkAccessManager* network, QWidget* parent = nullptr);

  static const char* kSettingsGroup;

 public slots:
  void AddImage(const QUrl& url);

 protected:
  void mouseReleaseEvent(QMouseEvent*);
  void resizeEvent(QResizeEvent* e);
  void wheelEvent(QWheelEvent* e);

 private slots:
  void ScrollBarReleased();
  void ScrollBarAction(int action);
  void ScrollTo(int index, bool smooth = true);
  void ScrollToCurrent();

 private:
  bool eventFilter(QObject*, QEvent*);

  QNetworkAccessManager* network_;

  QWidget* container_;
  QHBoxLayout* layout_;

  int current_index_;
  QPropertyAnimation* scroll_animation_;

  bool recursion_filter_;
};

#endif  // PRETTYIMAGEVIEW_H
