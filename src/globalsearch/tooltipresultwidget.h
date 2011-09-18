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

#ifndef TOOLTIPRESULTWIDGET_H
#define TOOLTIPRESULTWIDGET_H

#include "searchprovider.h"

#include <QAbstractButton>

class TooltipResultWidget : public QAbstractButton {
  Q_OBJECT

public:
  TooltipResultWidget(const SearchProvider::Result& result, QWidget* parent = 0);

  static const int kBorder;
  static const int kSpacing;
  static const int kTrackNumSpacing;
  static const int kLineHeight;
  static const int kIconSize;

  QSize sizeHint() const;

  QString TitleText() const;

protected:
  void paintEvent(QPaintEvent*);

private:
  QSize CalculateSizeHint() const;

private:
  SearchProvider::Result result_;
  const int kTextHeight;
  const int kTrackNoWidth;

  QSize size_hint_;

  QFont bold_font_;
  QFontMetrics bold_metrics_;
};

#endif // TOOLTIPRESULTWIDGET_H
