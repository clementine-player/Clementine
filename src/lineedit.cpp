#include "lineedit.h"

#include <QPainter>
#include <QPaintEvent>

LineEdit::LineEdit(QWidget* parent)
  : QLineEdit(parent)
{
}

void LineEdit::SetHint(const QString& hint) {
  hint_ = hint;
  update();
}

void LineEdit::paintEvent(QPaintEvent* e) {
  QLineEdit::paintEvent(e);

  if (!hasFocus() && displayText().isEmpty() && !hint_.isEmpty()) {
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
  }
}
