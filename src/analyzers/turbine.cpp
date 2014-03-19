//
// Amarok BarAnalyzer 3 - Jet Turbine: Symmetric version of analyzer 1
//
// Author: Stanislav Karchebny <berkus@users.sf.net>, (C) 2003
//         Max Howell (I modified it to use boom analyzer code)
//
// Copyright: like rest of Amarok
//

#include <cmath>
#include <QPainter>

#include "turbine.h"

const char* TurbineAnalyzer::kName =
    QT_TRANSLATE_NOOP("AnalyzerContainer", "Turbine");

void TurbineAnalyzer::analyze(QPainter& p, const Scope& scope, bool new_frame) {
  float h;
  const uint hd2 = height() / 2;
  const uint MAX_HEIGHT = hd2 - 1;

  for (uint i = 0, x = 0, y; i < BAND_COUNT; ++i, x += COLUMN_WIDTH + 1) {
    h = log10(scope[i] * 256.0) * F * 0.5;

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

    y = hd2 - uint(bar_height[i]);
    p.drawPixmap(x + 1, y, barPixmap, 0, y, -1, -1);
    p.drawPixmap(x + 1, hd2, barPixmap, 0, int(bar_height[i]), -1, -1);

    p.setPen(palette().color(QPalette::Highlight));
    if (bar_height[i] > 0)
      p.drawRect(x, y, COLUMN_WIDTH - 1, (int)bar_height[i] * 2 - 1);

    const uint x2 = x + COLUMN_WIDTH - 1;
    p.setPen(palette().color(QPalette::Base));
    y = hd2 - uint(peak_height[i]);
    p.drawLine(x, y, x2, y);
    y = hd2 + uint(peak_height[i]);
    p.drawLine(x, y, x2, y);
  }
}
