//
//
// C++ Implementation: Sonogram
//
// Description:
//
//
// Author: Melchior FRANZ <mfranz@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "sonogram.h"

#include <QPainter>

using Analyzer::Scope;

const char* Sonogram::kName =
    QT_TRANSLATE_NOOP("AnalyzerContainer", "Sonogram");

Sonogram::Sonogram(QWidget* parent) : Analyzer::Base(parent, 9) {}

Sonogram::~Sonogram() {}

void Sonogram::resizeEvent(QResizeEvent* e) {
  QWidget::resizeEvent(e);

// only for gcc < 4.0
#if !(__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 0))
  resizeForBands(height() < 128 ? 128 : height());
#endif

  canvas_ = QPixmap(size());
  canvas_.fill(palette().color(QPalette::Background));
}

void Sonogram::analyze(QPainter& p, const Scope& s, bool new_frame) {
  if (!new_frame) {
    p.drawPixmap(0, 0, canvas_);
    return;
  }

  int x = width() - 1;
  QColor c;

  QPainter canvas_painter(&canvas_);
  canvas_painter.drawPixmap(0, 0, canvas_, 1, 0, x, -1);

  Scope::const_iterator it = s.begin(), end = s.end();
  for (int y = height() - 1; y;) {
    if (it >= end || *it < .005)
      c = palette().color(QPalette::Background);
    else if (*it < .05)
      c.setHsv(95, 255, 255 - int(*it * 4000.0));
    else if (*it < 1.0)
      c.setHsv(95 - int(*it * 90.0), 255, 255);
    else
      c = Qt::red;

    canvas_painter.setPen(c);
    canvas_painter.drawPoint(x, y--);

    if (it < end) ++it;
  }

  canvas_painter.end();

  p.drawPixmap(0, 0, canvas_);
}

void Sonogram::transform(Scope& scope) {
  float* front = static_cast<float*>(&scope.front());
  m_fht->power2(front);
  m_fht->scale(front, 1.0 / 256);
  scope.resize(m_fht->size() / 2);
}

void Sonogram::demo(QPainter& p) {
  analyze(p, Scope(m_fht->size(), 0), new_frame_);
}
