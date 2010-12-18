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

#include "lineedit.h"
#include "ui/iconloader.h"

#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QToolButton>

LineEdit::LineEdit(QWidget* parent)
  : QLineEdit(parent),
    LineEditInterface(this),
    has_clear_button_(true),
    clear_button_(new QToolButton(this))
{
  clear_button_->setIcon(IconLoader::Load("edit-clear-locationbar-ltr"));
  clear_button_->setIconSize(QSize(16, 16));
  clear_button_->setCursor(Qt::ArrowCursor);
  clear_button_->setStyleSheet("QToolButton { border: none; padding: 0px; }");
  set_clear_button(true);

  connect(clear_button_, SIGNAL(clicked()), SLOT(clear()));
  connect(clear_button_, SIGNAL(clicked()), SLOT(setFocus()));
}

void LineEdit::set_hint(const QString& hint) {
  hint_ = hint;
  update();
}

void LineEdit::set_clear_button(bool visible) {
  has_clear_button_ = visible;
  clear_button_->setVisible(visible);

  if (visible) {
    const int frame_width = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-left: %1px; } ").arg(
                    clear_button_->sizeHint().width() + frame_width + 1));

    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clear_button_->sizeHint().height() + frame_width * 2 + 2),
                   qMax(msz.height(), clear_button_->sizeHint().height() + frame_width * 2 + 2));
  } else {
    setStyleSheet(QString());
  }
}

void LineEdit::paintEvent(QPaintEvent* e) {
  QLineEdit::paintEvent(e);

  if (!hasFocus() && displayText().isEmpty() && !hint_.isEmpty()) {
    clear_button_->hide();

    QPainter p(this);

    QFont font;
    font.setItalic(true);
    font.setPointSize(font.pointSize()-1);

    QFontMetrics m(font);
    const int kBorder = (height() - m.height()) / 2;

    p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
    p.setFont(font);

    QRect r(rect().topLeft() + QPoint(kBorder + 5, kBorder),
            rect().bottomRight() - QPoint(kBorder, kBorder));
    p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, hint_);
  } else {
    clear_button_->setVisible(has_clear_button_);
  }
}

void LineEdit::resizeEvent(QResizeEvent*) {
  const QSize sz = clear_button_->sizeHint();
  const int frame_width = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  clear_button_->move(frame_width, (rect().height() - sz.height()) / 2);
}
