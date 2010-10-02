/* This file is part of Clementine.

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

#include "collapsibleinfopane.h"

#include <QStylePainter>
#include <QStyleOption>
#include <QVBoxLayout>

const int CollapsibleInfoPane::kMargin = 6;
const int CollapsibleInfoPane::kTitleHeight = 20;

CollapsibleInfoPane::CollapsibleInfoPane(QWidget* parent)
  : QWidget(parent),
    widget_(NULL),
    expanded_(true)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(kMargin, kTitleHeight, kMargin, 0);
  setLayout(layout);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  setMinimumHeight(kTitleHeight);
}

void CollapsibleInfoPane::SetTitle(const QString& title) {
  title_ = title;
  update();
}

void CollapsibleInfoPane::SetWidget(QWidget* widget) {
  if (widget_)
    delete widget_;

  widget_ = widget;
  layout()->addWidget(widget);
}

void CollapsibleInfoPane::Collapse() {
  expanded_ = false;
  update();
}

void CollapsibleInfoPane::Expand() {
  expanded_ = true;
  update();
}

void CollapsibleInfoPane::paintEvent(QPaintEvent* e) {
  QStylePainter p(this);

  QRect title_rect(0, 0, width(), kTitleHeight);
  QRect indicator_rect(0, 0, kTitleHeight, kTitleHeight);
  QRect text_rect(title_rect);
  text_rect.setLeft(kTitleHeight + 6);

  // Draw the background
  const QColor bg_color_1(palette().color(QPalette::Highlight));
  const QColor bg_color_2(palette().color(QPalette::Highlight).lighter(125));
  const QColor bg_border(palette().color(QPalette::Dark));
  QLinearGradient bg_brush(title_rect.topLeft(), title_rect.bottomLeft());
  bg_brush.setColorAt(0.0, bg_color_1);
  bg_brush.setColorAt(0.5, bg_color_2);
  bg_brush.setColorAt(1.0, bg_color_1);

  p.setPen(bg_border);
  p.drawLine(title_rect.topLeft(), title_rect.topRight());
  p.drawLine(title_rect.bottomLeft(), title_rect.bottomRight());

  p.setPen(Qt::NoPen);
  p.fillRect(title_rect, bg_brush);

  // Draw the expand/collapse indicator
  QStyleOption opt;
  opt.initFrom(this);
  opt.rect = indicator_rect;
  opt.state |= QStyle::State_Children;
  if (expanded_)
    opt.state |= QStyle::State_Open;

  p.drawPrimitive(QStyle::PE_IndicatorBranch, opt);

  // Draw the title text
  p.setPen(palette().color(QPalette::HighlightedText));
  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, title_);
}
