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

#ifndef GLOBALSEARCHTOOLTIP_H
#define GLOBALSEARCHTOOLTIP_H

#include "searchprovider.h"

#include <QWidget>

class QDesktopWidget;

class GlobalSearchTooltip : public QWidget {
  Q_OBJECT

public:
  GlobalSearchTooltip(QObject* event_target);

  static const qreal kBorderRadius;
  static const qreal kBorderWidth;
  static const qreal kArrowWidth;
  static const qreal kArrowHeight;

  void SetResults(const SearchProvider::ResultList& results);
  void ShowAt(const QPoint& pointing_to);

  qreal ArrowOffset() const;

protected:
  void keyPressEvent(QKeyEvent* e);
  void paintEvent(QPaintEvent*);

private:
  QDesktopWidget* desktop_;

  SearchProvider::ResultList results_;
  qreal arrow_offset_;
  QRect inner_rect_;

  QObject* event_target_;

  QWidgetList widgets_;
};

#endif // GLOBALSEARCHTOOLTIP_H
