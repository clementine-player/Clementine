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

#include "tooltipresultwidget.h"
#include "core/logging.h"

#include <QPainter>

const int TooltipResultWidget::kBorder = 6;
const int TooltipResultWidget::kSpacing = 3;
const int TooltipResultWidget::kTrackNoSpacing = 6;
const int TooltipResultWidget::kLineHeight = 1;
const int TooltipResultWidget::kIconSize = 16;

TooltipResultWidget::TooltipResultWidget(const SearchProvider::Result& result,
                                         QWidget* parent)
  : QWidget(parent),
    result_(result),
    kTextHeight(fontMetrics().height()),
    kTrackNoWidth(fontMetrics().width("0000")),
    bold_metrics_(fontMetrics())
{
  bold_font_ = font();
  bold_font_.setBold(true);
  bold_metrics_ = QFontMetrics(bold_font_);

  size_hint_ = CalculateSizeHint();
}

QSize TooltipResultWidget::sizeHint() const {
  return size_hint_;
}

QSize TooltipResultWidget::CalculateSizeHint() const {
  int w = 0;
  int h = 0;

  // Title text
  h += kBorder + kIconSize + kBorder + kLineHeight;
  w = qMax(w, kBorder + kIconSize + kBorder + bold_metrics_.width(TitleText()) + kBorder);

  switch (result_.type_) {
  case SearchProvider::Result::Type_Track:
    break;

  case SearchProvider::Result::Type_Album:
    if (result_.album_songs_.isEmpty())
      break;

    // Song list
    h += kBorder + kSpacing * (result_.album_songs_.count() - 1) +
         kTextHeight * result_.album_songs_.count();
    foreach (const Song& song, result_.album_songs_) {
      w = qMax(w, kTrackNoWidth + kTrackNoSpacing +
                  fontMetrics().width(song.TitleWithCompilationArtist()) +
                  kBorder);
    }

    h += kBorder + kLineHeight;

    break;
  }

  return QSize(w, h);
}

QString TooltipResultWidget::TitleText() const {
  return result_.provider_->name();
}

void TooltipResultWidget::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setPen(palette().color(QPalette::Text));

  int y = kBorder;

  // Title icon
  QRect icon_rect(kBorder, y, kIconSize, kIconSize);
  p.drawPixmap(icon_rect, result_.provider_->icon().pixmap(kIconSize));

  // Title text
  QRect text_rect(icon_rect.right() + kBorder, y,
                  width() - kBorder*2 - icon_rect.right(), kIconSize);
  p.setFont(bold_font_);
  p.drawText(text_rect, Qt::AlignVCenter, TitleText());

  // Line
  y += kIconSize + kBorder;
  p.drawLine(0, y, width(), y);
  y += kLineHeight;

  switch (result_.type_) {
  case SearchProvider::Result::Type_Track:
    break;

  case SearchProvider::Result::Type_Album:
    if (result_.album_songs_.isEmpty())
      break;

    // Song list
    y += kBorder;

    p.setFont(font());

    foreach (const Song& song, result_.album_songs_) {
      QRect number_rect(0, y, kTrackNoWidth, kTextHeight);
      if (song.track() > 0) {
        // Track number
        p.drawText(number_rect, Qt::AlignRight | Qt::AlignHCenter,
                   QString::number(song.track()));
      }

      // Song title
      QRect title_rect(number_rect.right() + kTrackNoSpacing, y,
                       width() - number_rect.right() - kTrackNoSpacing - kBorder,
                       kTextHeight);
      p.drawText(title_rect, song.TitleWithCompilationArtist());

      y += kTextHeight + kSpacing;
    }

    y -= kSpacing;
    y += kBorder;

    // Line
    p.drawLine(0, y, width(), y);
    y += kLineHeight;

    break;
  }
}
