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

#ifndef NYANCATANALYZER_H
#define NYANCATANALYZER_H

#include "analyzerbase.h"

#include <QDateTime>

class NyanCatAnalyzer : public Analyzer::Base {
  Q_OBJECT

public:
  Q_INVOKABLE NyanCatAnalyzer(QWidget* parent);

  static const char* kName;

  void timerEvent(QTimerEvent* e);

protected:
  void transform( Scope& );
  void analyze( QPainter& p, const Analyzer::Scope& );

private:
  static const int kCatHeight = 21;
  static const int kCatWidth = 34;
  static const int kCatFrameCount = 5;
  static const int kRainbowOverlap = 13;

  static const int kHistorySize = 128;
  static const int kRainbowBands = 6;
  static const float kPixelScale;

  static const int kFrameIntervalMs = 150;

private:
  inline QRect CatSourceRect() const {
    return QRect(0, kCatHeight * frame_, kCatWidth, kCatHeight);
  }

private:
  QPixmap cat_;

  int timer_id_;
  int frame_;

  float history_[kHistorySize * kRainbowBands];
  float mean_history_[kHistorySize * kRainbowBands];
  QPen colors_[kRainbowBands];

  QBrush background_brush_;
};

#endif // NYANCATANALYZER_H
