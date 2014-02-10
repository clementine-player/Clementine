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

#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include "playlist/playlistitem.h"
#include "smartplaylists/generator_fwd.h"

#include <QIcon>
#include <QWidget>

class QMenu;
class QPropertyAnimation;

class TagWidgetTag : public QWidget {
  Q_OBJECT
  Q_PROPERTY(float background_opacity READ background_opacity WRITE
                 set_background_opacity);

 public:
  TagWidgetTag(const QIcon& icon, const QString& text, QWidget* parent);

  static const int kIconSize;
  static const int kIconTextSpacing;
  static const int kHPadding;
  static const int kVPadding;

  float background_opacity() const { return opacity_; }
  void set_background_opacity(float opacity);

  QSize sizeHint() const;
  QString text() const { return text_; }

signals:
  void Clicked();

 protected:
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void paintEvent(QPaintEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void contextMenuEvent(QContextMenuEvent*);

 private:
  QString text_;
  QIcon icon_;
  float opacity_;

  QPropertyAnimation* animation_;
};

class TagWidget : public QWidget {
  Q_OBJECT

 public:
  enum Type { Type_Tags, Type_Artists, };

  TagWidget(Type type, QWidget* parent = nullptr);

  void SetIcon(const QIcon& icon) { icon_ = icon; }
  void AddTag(const QString& tag);

  int count() const { return tags_.count(); }

signals:
  void AddToPlaylist(QMimeData* data);
  void DoGlobalSearch(const QString& query);

 private slots:
  void TagClicked();

 private:
  void PlayLastFm(const QString& url_pattern);

 private:
  Type type_;
  QIcon icon_;
  QList<TagWidgetTag*> tags_;
};

#endif  // TAGWIDGET_H
