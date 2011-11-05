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

class QAbstractButton;
class QDesktopWidget;

class TooltipActionWidget;

class GlobalSearchTooltip : public QWidget {
  Q_OBJECT

public:
  GlobalSearchTooltip(QWidget* event_target);

  static const qreal kBorderRadius;
  static const qreal kTotalBorderWidth;
  static const qreal kOuterBorderWidth;
  static const qreal kArrowWidth;
  static const qreal kArrowHeight;

  void SetActions(const QList<QAction*>& actions) { common_actions_ = actions; }
  void SetResults(const SearchProvider::ResultList& results);
  void ShowAt(const QPoint& pointing_to);

  int ActiveResultIndex() const { return active_result_; }
  qreal ArrowOffset() const;

  bool event(QEvent* e);

public slots:
  void ReloadSettings();

protected:
  void paintEvent(QPaintEvent*);

private slots:
  void SwitchProvider();

private:
  void AddWidget(QWidget* widget, int* w, int* y);

private:
  QDesktopWidget* desktop_;
  TooltipActionWidget* action_widget_;
  QList<QAction*> common_actions_;

  QAction* switch_action_;

  SearchProvider::ResultList results_;
  qreal arrow_offset_;
  QRect inner_rect_;

  QWidget* event_target_;

  QWidgetList widgets_;
  QList<QAbstractButton*> result_buttons_;
  int active_result_;

  bool show_tooltip_help_;
};

#endif // GLOBALSEARCHTOOLTIP_H
