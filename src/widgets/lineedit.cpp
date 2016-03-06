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

#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>

ExtendedEditor::ExtendedEditor(QWidget* widget, int extra_right_padding,
                               bool draw_hint)
    : LineEditInterface(widget),
      has_clear_button_(true),
      clear_button_(new QToolButton(widget)),
      reset_button_(new QToolButton(widget)),
      extra_right_padding_(extra_right_padding),
      draw_hint_(draw_hint),
      is_rtl_(false) {
  clear_button_->setIcon(IconLoader::Load("edit-clear-locationbar-ltr", IconLoader::Base));
  clear_button_->setIconSize(QSize(16, 16));
  clear_button_->setCursor(Qt::ArrowCursor);
  clear_button_->setStyleSheet("QToolButton { border: none; padding: 0px; }");
  clear_button_->setToolTip(widget->tr("Clear"));
  clear_button_->setFocusPolicy(Qt::NoFocus);

  QStyleOption opt;
  opt.initFrom(widget);

  reset_button_->setIcon(widget->style()->standardIcon(
      QStyle::SP_DialogResetButton, &opt, widget));
  reset_button_->setIconSize(QSize(16, 16));
  reset_button_->setCursor(Qt::ArrowCursor);
  reset_button_->setStyleSheet("QToolButton { border: none; padding: 0px; }");
  reset_button_->setToolTip(widget->tr("Reset"));
  reset_button_->setFocusPolicy(Qt::NoFocus);
  reset_button_->hide();

  widget->connect(clear_button_, SIGNAL(clicked()), widget, SLOT(clear()));
  widget->connect(clear_button_, SIGNAL(clicked()), widget, SLOT(setFocus()));

  UpdateButtonGeometry();
}

void ExtendedEditor::set_clear_button(bool visible) {
  has_clear_button_ = visible;
  clear_button_->setVisible(visible);
  UpdateButtonGeometry();
}

bool ExtendedEditor::has_reset_button() const {
  return reset_button_->isVisible();
}

void ExtendedEditor::set_reset_button(bool visible) {
  reset_button_->setVisible(visible);
  UpdateButtonGeometry();
}

void ExtendedEditor::UpdateButtonGeometry() {
  const int frame_width =
      widget_->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  const int left = frame_width + 1 +
                   (has_clear_button() ? clear_button_->sizeHint().width() : 0);
  const int right =
      frame_width + 1 +
      (has_reset_button() ? reset_button_->sizeHint().width() : 0);

  widget_->setStyleSheet(
      QString("QLineEdit { padding-left: %1px; padding-right: %2px; }")
          .arg(left)
          .arg(right));

  QSize msz = widget_->minimumSizeHint();
  widget_->setMinimumSize(
      msz.width() + (clear_button_->sizeHint().width() + frame_width + 1) * 2 +
          extra_right_padding_,
      qMax(msz.height(),
           clear_button_->sizeHint().height() + frame_width * 2 + 2));
}

void ExtendedEditor::Paint(QPaintDevice* device) {
  if (!widget_->hasFocus() && is_empty() && !hint_.isEmpty()) {
    clear_button_->hide();

    if (!draw_hint_) {
      clear_button_->setVisible(has_clear_button_);
    }
  }
}

void ExtendedEditor::Resize() {
  const QSize sz = clear_button_->sizeHint();
  const int frame_width =
      widget_->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
  const int y = (widget_->rect().height() - sz.height()) / 2;

  clear_button_->move(frame_width, y);

  if (!is_rtl_) {
    reset_button_->move(
        widget_->width() - frame_width - sz.width() - extra_right_padding_, y);
  } else {
    reset_button_->move((has_clear_button() ? sz.width() + 4 : 0) + frame_width,
                        y);
  }
}

LineEdit::LineEdit(QWidget* parent) : QLineEdit(parent), ExtendedEditor(this) {
  connect(reset_button_, SIGNAL(clicked()), SIGNAL(Reset()));
  connect(this, SIGNAL(textChanged(QString)), SLOT(text_changed(QString)));
}

void LineEdit::text_changed(const QString& text) {
  if (text.isEmpty()) {
    // Consider empty string as LTR
    set_rtl(false);
  } else {
    // For some reason Qt will detect any text with LTR at the end as LTR, so
    // instead
    // compare only the first character
    set_rtl(QString(text.at(0)).isRightToLeft());
  }
  Resize();
  clear_hint();
}

void LineEdit::set_hint(const QString& text) {
  hint_ = text;
  setPlaceholderText(text);
}

void LineEdit::paintEvent(QPaintEvent* e) {
  QLineEdit::paintEvent(e);
  Paint(this);
}

void LineEdit::resizeEvent(QResizeEvent* e) {
  QLineEdit::resizeEvent(e);
  Resize();
}

TextEdit::TextEdit(QWidget* parent)
    : QPlainTextEdit(parent), ExtendedEditor(this) {
  connect(reset_button_, SIGNAL(clicked()), SIGNAL(Reset()));
  connect(this, SIGNAL(textChanged()), this,
          SLOT(text_changed()));  // To clear the hint*/
}

void TextEdit::set_hint(const QString& hint) {
  hint_ = hint;
}

//this can be replaced in qt5 with setPlaceHolderText of qtextedit
void TextEdit::paintEvent(QPaintEvent* e) {
  QPlainTextEdit::paintEvent(e);
  Paint(viewport());

  if (!widget_->hasFocus() && is_empty() && !hint_.isEmpty() && draw_hint_) {
    QPainter p(viewport());

    QFont font;
    font.setBold(false);
    font.setPointSizeF(widget_->font().pointSizeF());

    QFontMetrics m(font);
    const int kBorder = (viewport()->height() - m.height()) / 2;

    //the color of PlaceHolderText is hardcoded in Qt to be text color at 128 alpha
    QColor col = widget_->palette().text().color();
    col.setAlpha(128);
    p.setPen(col);
    p.setFont(font);

    QRect r(5, kBorder, viewport()->width() - 10, viewport()->height() - kBorder * 2);
    p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter,
               m.elidedText(hint_, Qt::ElideRight, r.width()));
  }
}

void TextEdit::resizeEvent(QResizeEvent* e) {
  QPlainTextEdit::resizeEvent(e);
  Resize();
}

const char* SpinBox::abbrev_hint = "-";

SpinBox::SpinBox(QWidget* parent)
    : QSpinBox(parent), ExtendedEditor(this, 14, true) {
  connect(reset_button_, SIGNAL(clicked()), SIGNAL(Reset()));
  connect(this, SIGNAL(valueChanged(QString)), this, SLOT(value_changed()));
}

void SpinBox::set_hint(const QString& hint) {
  if (hint.isEmpty()) {
    hint_ = "";
    lineEdit()->setPlaceholderText("");
  } else {
    hint_ = abbrev_hint;
    lineEdit()->clear();
    lineEdit()->setPlaceholderText(abbrev_hint);
  }
}

void SpinBox::paintEvent(QPaintEvent* e) {
  QSpinBox::paintEvent(e);
  Paint(this);
}

void SpinBox::resizeEvent(QResizeEvent* e) {
  QSpinBox::resizeEvent(e);
  Resize();
}

void SpinBox::focusOutEvent(QFocusEvent* event) {
  QSpinBox::focusOutEvent(event);
  if (!hint_.isEmpty() && value() <=0) {
    lineEdit()->clear();
    lineEdit()->setPlaceholderText(hint_);
  }
}

QString SpinBox::textFromValue(int val) const {
  if (val <= 0 && !hint_.isEmpty()) return "-";
  return QSpinBox::textFromValue(val);
}
