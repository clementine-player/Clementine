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

const int TooltipResultWidget::kBorder = 15;
const int TooltipResultWidget::kSpacing = 3;
const int TooltipResultWidget::kTrackNumSpacing = 6;
const int TooltipResultWidget::kLineHeight = 1;
const int TooltipResultWidget::kIconSize = 16;

TooltipResultWidget::TooltipResultWidget(const SearchProvider::Result& result,
                                         QWidget* parent)
  : QAbstractButton(parent),
    result_(result),
    kTextHeight(fontMetrics().height()),
    kTrackNoWidth(fontMetrics().width("0000")),
    bold_metrics_(fontMetrics())
{
  bold_font_ = font();
  bold_font_.setBold(true);
  bold_metrics_ = QFontMetrics(bold_font_);

  size_hint_ = CalculateSizeHint();

  setCheckable(true);
  setAutoExclusive(true);
}

QSize TooltipResultWidget::sizeHint() const {
  return size_hint_;
}

QSize TooltipResultWidget::CalculateSizeHint() const {
  int w = 0;
  int h = 0;

  // Title text
  h += kSpacing + kIconSize + kSpacing + kLineHeight;
  w = qMax(w, kBorder + kTrackNoWidth + kTrackNumSpacing +
              bold_metrics_.width(TitleText()) + kBorder);

  switch (result_.type_) {
  case SearchProvider::Result::Type_Track:
  case SearchProvider::Result::Type_Stream:
    break;

  case SearchProvider::Result::Type_Album:
    if (result_.album_songs_.isEmpty())
      break;

    // Song list
    h += kSpacing + kSpacing * (result_.album_songs_.count() - 1) +
         kTextHeight * result_.album_songs_.count();
    foreach (const Song& song, result_.album_songs_) {
      w = qMax(w, kBorder + kTrackNoWidth + kTrackNumSpacing +
                  fontMetrics().width(song.TitleWithCompilationArtist()) +
                  kBorder);
    }

    h += kSpacing + kLineHeight;

    break;
  }

  return QSize(w, h);
}

QString TooltipResultWidget::TitleText() const {
  return result_.provider_->name();
}

void TooltipResultWidget::paintEvent(QPaintEvent*) {
  QPainter p(this);

  const QColor text_color = palette().color(QPalette::Text);

  const qreal line_opacity  = 0.1 + (isChecked() ? 0.2 : 0.0);
  const qreal track_opacity = 0.1 + (isChecked() ? 0.5 : 0.0);
  const qreal text_opacity  = 0.4 + (isChecked() ? 0.5 : 0.0);

  int y = kSpacing;

  // Title text
  QRect text_rect(kBorder + kTrackNoWidth + kTrackNumSpacing, y,
                  width() - kBorder*2 - kTrackNoWidth - kTrackNumSpacing, kIconSize);
  p.setFont(bold_font_);
  p.setPen(text_color);
  p.setOpacity(text_opacity);
  p.drawText(text_rect, Qt::AlignVCenter, TitleText());

  // Title icon
  QRect icon_rect(text_rect.left() - kTrackNumSpacing - kIconSize, y,
                  kIconSize, kIconSize);
  p.drawPixmap(icon_rect, result_.provider_->icon().pixmap(kIconSize));

  // Line
  y += kIconSize + kSpacing;
  p.setOpacity(line_opacity);
  p.setPen(text_color);
  p.drawLine(0, y, width(), y);
  y += kLineHeight;

  switch (result_.type_) {
  case SearchProvider::Result::Type_Track:
  case SearchProvider::Result::Type_Stream:
    break;

  case SearchProvider::Result::Type_Album:
    if (result_.album_songs_.isEmpty())
      break;

    // Song list
    y += kSpacing;

    p.setFont(font());

    foreach (const Song& song, result_.album_songs_) {
      QRect number_rect(kBorder, y, kTrackNoWidth, kTextHeight);
      if (song.track() > 0) {
        // Track number
        p.setOpacity(track_opacity);
        p.drawText(number_rect, Qt::AlignRight | Qt::AlignHCenter,
                   QString::number(song.track()));
      }

      // Song title
      QRect title_rect(number_rect.right() + kTrackNumSpacing, y,
                       width() - number_rect.right() - kTrackNumSpacing - kBorder,
                       kTextHeight);
      p.setOpacity(text_opacity);
      p.drawText(title_rect, song.TitleWithCompilationArtist());

      y += kTextHeight + kSpacing;
    }

    // Line
    p.setOpacity(line_opacity);
    p.drawLine(0, y, width(), y);
    y += kLineHeight;

    break;
  }

  y += kSpacing;
}
