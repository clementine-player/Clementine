/* This file is part of Clementine.
   Copyright 2014, Alibek Omarov <a1ba.omarov@gmail.com>
   Copyright 2014, Mark Furneaux <mark@romaco.ca>
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

#ifndef ANALYZERS_RAINBOWDASHANALYZER_H_
#define ANALYZERS_RAINBOWDASHANALYZER_H_

#include "analyzerbase.h"

#include <QDateTime>

class RainbowDashAnalyzer : public Analyzer::Base {
  Q_OBJECT

 public:
  Q_INVOKABLE RainbowDashAnalyzer(QWidget* parent);

  static const char* kName;

 protected:
  void transform(Analyzer::Scope&);
  void analyze(QPainter& p, const Analyzer::Scope&, bool new_frame);

  void timerEvent(QTimerEvent* e);
  void resizeEvent(QResizeEvent* e);

 private:
  static const int kDashHeight = 33;
  static const int kDashWidth = 53;
  static const int kRainbowHeight = 16;
  static const int kDashFrameCount = 16;
  static const int kRainbowOverlap = 15;
  static const int kSleepingDashHeight = 33;

  static const int kHistorySize = 128;
  static const int kRainbowBands = 6;
  static const float kPixelScale;

  static const int kFrameIntervalMs = 150;

 private:
  inline QRect DashSourceRect() const {
    return QRect(0, kDashHeight * frame_, kDashWidth, kDashHeight);
  }

  inline QRect SleepingDashSourceRect() const {
    return QRect(0, kDashHeight * kDashFrameCount, kDashWidth,
                 kSleepingDashHeight);
  }

  inline QRect DashDestRect() const {
    return QRect(width() - kDashWidth, (height() - kDashHeight) / 2, kDashWidth,
                 kDashHeight);
  }

  inline QRect SleepingDashDestRect() const {
    return QRect(width() - kDashWidth, (height() - kSleepingDashHeight) / 2,
                 kDashWidth, kSleepingDashHeight);
  }

 private:
  // "constants" that get initialised in the constructor
  float band_scale_[kRainbowBands];
  QPen colors_[kRainbowBands];

  QPixmap dash_;

  // For the animation
  int timer_id_;
  int frame_;

  // The y positions of each point on the rainbow.
  float history_[kHistorySize * kRainbowBands];

  // A cache of the last frame's rainbow, so it can be used in the next frame.
  QPixmap buffer_[2];
  int current_buffer_;

  // Geometry information that's updated on resize:
  // The width of the widget minus the space for the pony
  int available_rainbow_width_;

  // X spacing between each point in the polyline.
  int px_per_frame_;

  // Amount the buffer_ is shifted to the left (off the edge of the widget) to
  // make the rainbow extend from 0 to available_rainbow_width_.
  int x_offset_;

  QBrush background_brush_;
};

#endif  // ANALYZERS_RAINBOWDASHANALYZER_H_
