/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#ifndef MOODBARRENDERER_H
#define MOODBARRENDERER_H

#include <QColor>
#include <QPixmap>
#include <QMetaType>
#include <QVector>

class QPalette;
typedef QVector<QColor> ColorVector;

class MoodbarRenderer {
public:
  // These values are persisted.  Remember to change appearancesettingspage.ui
  // when changing them.
  enum MoodbarStyle {
    Style_Normal = 0,
    Style_Angry,
    Style_Frozen,
    Style_Happy,
    Style_SystemPalette
  };

  static const int kNumHues;

  static ColorVector Colors(const QByteArray& data, MoodbarStyle style,
                            const QPalette& palette);
  static void Render(const ColorVector& colors, QPainter* p, const QRect& rect);
  static QImage RenderToImage(const ColorVector& colors, const QSize& size);

private:
  MoodbarRenderer();

  struct StyleProperties {
    StyleProperties(int threshold = 0, int range_start = 0, int range_delta = 0,
                    int sat = 0, int val = 0)
      : threshold_(threshold), range_start_(range_start), range_delta_(range_delta),
        sat_(sat), val_(val) {}

    int threshold_;
    int range_start_;
    int range_delta_;
    int sat_;
    int val_;
  };
};

Q_DECLARE_METATYPE(QVector<QColor>)

#endif // MOODBARRENDERER_H
