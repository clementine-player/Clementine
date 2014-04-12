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

#include "collapsibleinfoheader.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOption>

const int CollapsibleInfoHeader::kHeight = 20;
const int CollapsibleInfoHeader::kIconSize = 16;

CollapsibleInfoHeader::CollapsibleInfoHeader(QWidget* parent)
    : QWidget(parent),
      expanded_(false),
      hovering_(false),
      animation_(new QPropertyAnimation(this, "opacity", this)),
      opacity_(0.0) {
  setMinimumHeight(kHeight);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setCursor(QCursor(Qt::PointingHandCursor));
}

void CollapsibleInfoHeader::SetTitle(const QString& title) {
  title_ = title;
  update();
}

void CollapsibleInfoHeader::SetIcon(const QIcon& icon) {
  icon_ = icon;
  update();
}

void CollapsibleInfoHeader::SetExpanded(bool expanded) {
  expanded_ = expanded;

  emit ExpandedToggled(expanded);
  if (expanded)
    emit Expanded();
  else
    emit Collapsed();
}

void CollapsibleInfoHeader::enterEvent(QEvent*) {
  hovering_ = true;
  if (!expanded_) {
    animation_->stop();
    animation_->setEndValue(1.0);
    animation_->setDuration(80);
    animation_->start();
  }
}

void CollapsibleInfoHeader::leaveEvent(QEvent*) {
  hovering_ = false;
  if (!expanded_) {
    animation_->stop();
    animation_->setEndValue(0.0);
    animation_->setDuration(160);
    animation_->start();
  }
}

void CollapsibleInfoHeader::set_opacity(float opacity) {
  opacity_ = opacity;
  update();
}

void CollapsibleInfoHeader::paintEvent(QPaintEvent* e) {
  QPainter p(this);

  QColor active_text_color(
      palette().color(QPalette::Active, QPalette::HighlightedText));
  QColor inactive_text_color(palette().color(QPalette::Active, QPalette::Text));
  QColor text_color;
  if (expanded_) {
    text_color = active_text_color;
  } else {
    p.setOpacity(0.4 + opacity_ * 0.6);
    text_color = QColor(active_text_color.red() * opacity_ +
                            inactive_text_color.red() * (1.0 - opacity_),
                        active_text_color.green() * opacity_ +
                            inactive_text_color.green() * (1.0 - opacity_),
                        active_text_color.blue() * opacity_ +
                            inactive_text_color.blue() * (1.0 - opacity_));
  }

  QRect indicator_rect(0, 0, height(), height());
  QRect icon_rect(height() + 2, (kHeight - kIconSize) / 2, kIconSize,
                  kIconSize);
  QRect text_rect(rect());
  text_rect.setLeft(icon_rect.right() + 4);

  // Draw the background
  QColor highlight(palette().color(QPalette::Active, QPalette::Highlight));
  const QColor bg_color_1(highlight.lighter(120));
  const QColor bg_color_2(highlight.darker(120));
  const QColor bg_border(palette().color(QPalette::Dark));
  QLinearGradient bg_brush(rect().topLeft(), rect().bottomLeft());
  bg_brush.setColorAt(0.0, bg_color_1);
  bg_brush.setColorAt(0.5, bg_color_1);
  bg_brush.setColorAt(0.5, bg_color_2);
  bg_brush.setColorAt(1.0, bg_color_2);

  p.setPen(Qt::NoPen);
  p.fillRect(rect(), bg_brush);

  p.setPen(bg_border);
  p.drawLine(rect().topLeft(), rect().topRight());
  p.drawLine(rect().bottomLeft(), rect().bottomRight());

  // Draw the expand/collapse indicator
  QStyleOption opt;
  opt.initFrom(this);
  opt.rect = indicator_rect;
  opt.state |= QStyle::State_Children;
  if (expanded_) opt.state |= QStyle::State_Open;
  if (hovering_) opt.state |= QStyle::State_Active;

  // Have to use the application's style here because using the widget's style
  // will trigger QStyleSheetStyle's recursion guard (I don't know why).
  QApplication::style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, &p,
                                       this);

  // Draw the icon
  p.drawPixmap(icon_rect, icon_.pixmap(kIconSize));

  // Draw the title text
  QFont bold_font(font());
  bold_font.setBold(true);
  p.setFont(bold_font);

  p.setPen(text_color);
  p.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, title_);
}

void CollapsibleInfoHeader::mouseReleaseEvent(QMouseEvent* e) {
  SetExpanded(!expanded_);
}
