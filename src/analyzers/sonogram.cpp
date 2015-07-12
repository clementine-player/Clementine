/* This file is part of Clementine.
   Copyright 2004, Melchior FRANZ <mfranz@kde.org>
   Copyright 2009-2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014-2015, Mark Furneaux <mark@furneaux.ca>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

/* Original Author:  Melchior FRANZ  <mfranz@kde.org>  2004
 */

#include "sonogram.h"

#include <QPainter>

using Analyzer::Scope;

const char* Sonogram::kName =
    QT_TRANSLATE_NOOP("AnalyzerContainer", "Sonogram");

Sonogram::Sonogram(QWidget* parent)
    : Analyzer::Base(parent, 9), scope_size_(128) {}

Sonogram::~Sonogram() {}

void Sonogram::resizeEvent(QResizeEvent* e) {
  QWidget::resizeEvent(e);

// only for gcc < 4.0
#if !(__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 0))
  resizeForBands(height() < 128 ? 128 : height());
#endif

  canvas_ = QPixmap(size());
  canvas_.fill(palette().color(QPalette::Background));
  updateBandSize(scope_size_);
}

void Sonogram::psychedelicModeChanged(bool enabled) {
  psychedelic_enabled_ = enabled;
  updateBandSize(scope_size_);
}

void Sonogram::analyze(QPainter& p, const Scope& s, bool new_frame) {
  if (!new_frame || engine_->state() == Engine::Paused) {
    p.drawPixmap(0, 0, canvas_);
    return;
  }

  int x = width() - 1;
  QColor c;

  QPainter canvas_painter(&canvas_);
  canvas_painter.drawPixmap(0, 0, canvas_, 1, 0, x, -1);

  Scope::const_iterator it = s.begin(), end = s.end();
  if (scope_size_ != s.size()) {
    scope_size_ = s.size();
    updateBandSize(scope_size_);
  }

  if (psychedelic_enabled_) {
    c = getPsychedelicColor(s, 20, 100);
    for (int y = height() - 1; y;) {
      if (it >= end || *it < .005) {
        c = palette().color(QPalette::Background);
      } else if (*it < .05) {
        c.setHsv(c.hue(), c.saturation(), 255 - static_cast<int>(*it * 4000.0));
      } else if (*it < 1.0) {
        c.setHsv((c.hue() + static_cast<int>(*it * 90.0)) % 255, 255, 255);
      } else {
        c = getPsychedelicColor(s, 10, 50);
      }

      canvas_painter.setPen(c);
      canvas_painter.drawPoint(x, y--);

      if (it < end) ++it;
    }
  } else {
    for (int y = height() - 1; y;) {
      if (it >= end || *it < .005)
        c = palette().color(QPalette::Background);
      else if (*it < .05)
        c.setHsv(95, 255, 255 - static_cast<int>(*it * 4000.0));
      else if (*it < 1.0)
        c.setHsv(95 - static_cast<int>(*it * 90.0), 255, 255);
      else
        c = Qt::red;

      canvas_painter.setPen(c);
      canvas_painter.drawPoint(x, y--);

      if (it < end) ++it;
    }
  }

  canvas_painter.end();

  p.drawPixmap(0, 0, canvas_);
}

void Sonogram::transform(Scope& scope) {
  float* front = static_cast<float*>(&scope.front());
  fht_->power2(front);
  fht_->scale(front, 1.0 / 256);
  scope.resize(fht_->size() / 2);
}

void Sonogram::demo(QPainter& p) {
  analyze(p, Scope(fht_->size(), 0), new_frame_);
}
