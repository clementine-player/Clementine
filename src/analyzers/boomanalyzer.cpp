// Author: Max Howell <max.howell@methylblue.com>, (C) 2004
// Copyright: See COPYING file that comes with this distribution

#include "boomanalyzer.h"
#include <cmath>
#include <QPainter>

const char* BoomAnalyzer::kName =
    QT_TRANSLATE_NOOP("AnalyzerContainer", "Boom analyzer");

BoomAnalyzer::BoomAnalyzer(QWidget* parent)
    : Analyzer::Base(parent, 9),
      K_barHeight(1.271)  // 1.471
      ,
      F_peakSpeed(1.103)  // 1.122
      ,
      F(1.0),
      bar_height(BAND_COUNT, 0),
      peak_height(BAND_COUNT, 0),
      peak_speed(BAND_COUNT, 0.01),
      barPixmap(COLUMN_WIDTH, 50) {}

void BoomAnalyzer::changeK_barHeight(int newValue) {
  K_barHeight = (double)newValue / 1000;
}

void BoomAnalyzer::changeF_peakSpeed(int newValue) {
  F_peakSpeed = (double)newValue / 1000;
}

void BoomAnalyzer::resizeEvent(QResizeEvent*) { init(); }

void BoomAnalyzer::init() {
  const uint HEIGHT = height() - 2;
  const double h = 1.2 / HEIGHT;

  F = double(HEIGHT) / (log10(256) * 1.1 /*<- max. amplitude*/);

  barPixmap = QPixmap(COLUMN_WIDTH - 2, HEIGHT);
  canvas_ = QPixmap(size());
  canvas_.fill(palette().color(QPalette::Background));

  QPainter p(&barPixmap);
  for (uint y = 0; y < HEIGHT; ++y) {
    const double F = (double)y * h;

    p.setPen(QColor(qMax(0, 255 - int(229.0 * F)),
                    qMax(0, 255 - int(229.0 * F)),
                    qMax(0, 255 - int(191.0 * F))));
    p.drawLine(0, y, COLUMN_WIDTH - 2, y);
  }
}

void BoomAnalyzer::transform(Scope& s) {
  float* front = static_cast<float*>(&s.front());

  m_fht->spectrum(front);
  m_fht->scale(front, 1.0 / 60);

  Scope scope(32, 0);

  const uint xscale[] = {0,  1,  2,  3,  4,  5,   6,   7,   8,   9,   10,
                         11, 12, 13, 14, 15, 16,  17,  19,  24,  29,  36,
                         43, 52, 63, 76, 91, 108, 129, 153, 182, 216, 255};

  for (uint j, i = 0; i < 32; i++)
    for (j = xscale[i]; j < xscale[i + 1]; j++)
      if (s[j] > scope[i]) scope[i] = s[j];

  s = scope;
}

void BoomAnalyzer::analyze(QPainter& p, const Scope& scope, bool new_frame) {
  if (!new_frame) {
    p.drawPixmap(0, 0, canvas_);
    return;
  }
  float h;
  const uint MAX_HEIGHT = height() - 1;

  QPainter canvas_painter(&canvas_);
  canvas_.fill(palette().color(QPalette::Background));

  for (uint i = 0, x = 0, y; i < BAND_COUNT; ++i, x += COLUMN_WIDTH + 1) {
    h = log10(scope[i] * 256.0) * F;

    if (h > MAX_HEIGHT) h = MAX_HEIGHT;

    if (h > bar_height[i]) {
      bar_height[i] = h;

      if (h > peak_height[i]) {
        peak_height[i] = h;
        peak_speed[i] = 0.01;
      } else
        goto peak_handling;
    } else {
      if (bar_height[i] > 0.0) {
        bar_height[i] -= K_barHeight;  // 1.4
        if (bar_height[i] < 0.0) bar_height[i] = 0.0;
      }

    peak_handling:

      if (peak_height[i] > 0.0) {
        peak_height[i] -= peak_speed[i];
        peak_speed[i] *= F_peakSpeed;  // 1.12

        if (peak_height[i] < bar_height[i]) peak_height[i] = bar_height[i];
        if (peak_height[i] < 0.0) peak_height[i] = 0.0;
      }
    }

    y = height() - uint(bar_height[i]);
    canvas_painter.drawPixmap(x + 1, y, barPixmap, 0, y, -1, -1);
    canvas_painter.setPen(palette().color(QPalette::Highlight));
    if (bar_height[i] > 0)
      canvas_painter.drawRect(x, y, COLUMN_WIDTH - 1, height() - y - 1);

    y = height() - uint(peak_height[i]);
    canvas_painter.setPen(palette().color(QPalette::Base));
    canvas_painter.drawLine(x, y, x + COLUMN_WIDTH - 1, y);
  }

  p.drawPixmap(0, 0, canvas_);
}
