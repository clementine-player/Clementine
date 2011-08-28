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

#include "globalsearchitemdelegate.h"
#include "globalsearchwidget.h"

#include <QApplication>
#include <QPainter>


const int GlobalSearchItemDelegate::kHeight = SearchProvider::kArtHeight;
const int GlobalSearchItemDelegate::kMargin = 1;
const int GlobalSearchItemDelegate::kArtMargin = 6;
const int GlobalSearchItemDelegate::kWordPadding = 6;

GlobalSearchItemDelegate::GlobalSearchItemDelegate(GlobalSearchWidget* widget)
  : QStyledItemDelegate(widget),
    widget_(widget)
{
  no_cover_ = QPixmap::fromImage(QImage(":nocover.png"));
}

QSize GlobalSearchItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const {
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  size.setHeight(kHeight + kMargin*2);
  return size;
}

void GlobalSearchItemDelegate::DrawAndShrink(QPainter* p, QRect* rect,
                                             const QString& text) const {
  QRect br;
  p->drawText(*rect, Qt::TextSingleLine | Qt::AlignVCenter, text, &br);
  rect->setLeft(br.right() + kWordPadding);
}

void GlobalSearchItemDelegate::paint(QPainter* p,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {
  const SearchProvider::Result result =
      index.data(GlobalSearchWidget::Role_Result).value<SearchProvider::Result>();
  const Song& m = result.metadata_;

  widget_->LazyLoadArt(index);

  QFont bold_font = option.font;
  bold_font.setBold(true);

  QColor pen = option.palette.color(QPalette::Text);
  QColor light_pen = pen;
  pen.setAlpha(200);
  light_pen.setAlpha(128);

  // Draw the background
  const QStyleOptionViewItemV3* vopt = qstyleoption_cast<const QStyleOptionViewItemV3*>(&option);
  const QWidget* widget = vopt->widget;
  QStyle* style = widget->style() ? widget->style() : QApplication::style();
  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, widget);

  // Draw the album art.  This will already be the correct size.
  const QRect rect = option.rect;
  const QRect art_rect(rect.left() + kMargin, rect.top() + kMargin, kHeight, kHeight);

  QPixmap art = index.data(Qt::DecorationRole).value<QPixmap>();
  if (art.isNull())
    art = no_cover_;

  p->drawPixmap(art_rect, art);

  // Position text
  QRect text_rect(art_rect.right() + kArtMargin, art_rect.top(),
                  rect.right() - art_rect.right() - kArtMargin, kHeight);
  QRect text_rect_1(text_rect.adjusted(0, 0, 0, -kHeight/2));
  QRect text_rect_2(text_rect.adjusted(0, kHeight/2, 0, 0));

  // The text we draw depends on the type of result.
  switch (result.type_) {
  case SearchProvider::Result::Type_Track: {
    // Line 1 is Title
    p->setFont(bold_font);

    // Title
    p->setPen(pen);
    DrawAndShrink(p, &text_rect_1, m.title());

    // Line 2 is Artist - Album - Track n
    p->setFont(option.font);

    // Artist
    p->setPen(pen);
    if (!m.artist().isEmpty()) {
      DrawAndShrink(p, &text_rect_2, m.artist());
    } else if (!m.albumartist().isEmpty()) {
      DrawAndShrink(p, &text_rect_2, m.albumartist());
    }

    if (!m.album().isEmpty()) {
      // Dash
      p->setPen(light_pen);
      DrawAndShrink(p, &text_rect_2, " - ");

      // Album
      p->setPen(pen);
      DrawAndShrink(p, &text_rect_2, m.album());
    }

    if (m.track() > 0) {
      // Dash
      p->setPen(light_pen);
      DrawAndShrink(p, &text_rect_2, " - ");

      // Album
      DrawAndShrink(p, &text_rect_2, tr("track %1").arg(m.track()));
    }

    break;
  }

  case SearchProvider::Result::Type_Album: {
    // Line 1 is Artist - Album
    p->setFont(bold_font);

    // Artist
    p->setPen(pen);
    if (!m.albumartist().isEmpty())
      DrawAndShrink(p, &text_rect_1, m.albumartist());
    else if (m.is_compilation())
      DrawAndShrink(p, &text_rect_1, tr("Various Artists"));
    else if (!m.artist().isEmpty())
      DrawAndShrink(p, &text_rect_1, m.artist());
    else
      DrawAndShrink(p, &text_rect_1, tr("Unknown"));

    // Dash
    p->setPen(light_pen);
    DrawAndShrink(p, &text_rect_1, " - ");

    // Album
    p->setPen(pen);
    if (m.album().isEmpty())
      DrawAndShrink(p, &text_rect_1, tr("Unknown"));
    else
      DrawAndShrink(p, &text_rect_1, m.album());

    // Line 2 is <n> tracks
    p->setFont(option.font);

    p->setPen(pen);
    DrawAndShrink(p, &text_rect_2, QString::number(result.album_size_));

    p->setPen(light_pen);
    DrawAndShrink(p, &text_rect_2, tr(result.album_size_ == 1 ? "track" : "tracks"));
    break;
  }
  }
}
