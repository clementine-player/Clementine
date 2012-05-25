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

#ifndef MOODBARPROXYSTYLE_H
#define MOODBARPROXYSTYLE_H

#include <QProxyStyle>

class QSlider;
class QStyleOptionSlider;
class QTimeLine;

class MoodbarProxyStyle : public QProxyStyle {
  Q_OBJECT

public:
  MoodbarProxyStyle(QSlider* slider);

  enum MoodbarStyle {
    Style_Angry,
    Style_Frozen,
    Style_Happy,
    Style_SystemDefault
  };

  // QProxyStyle
  void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                          QPainter* painter, const QWidget* widget) const;
  QRect subControlRect(ComplexControl cc, const QStyleOptionComplex* opt,
                       SubControl sc, const QWidget* widget) const;

  // QObject
  bool eventFilter(QObject* object, QEvent* event);

public slots:
  // An empty byte array means there's no moodbar, so just show a normal slider.
  void SetMoodbarData(const QByteArray& data);

  // If the moodbar is disabled then a normal slider will always be shown.
  void SetMoodbarEnabled(bool enabled);

private:
  static const int kNumHues;
  static const int kMarginSize;
  static const int kBorderSize;
  static const int kArrowWidth;
  static const int kArrowHeight;

  enum State {
    MoodbarOn,
    MoodbarOff,
    FadingToOn,
    FadingToOff
  };

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

  typedef QVector<QColor> ColorList;

private:
  void NextState();

  void Render(ComplexControl control, const QStyleOptionSlider* option,
              QPainter* painter, const QWidget* widget);
  void EnsureMoodbarRendered();
  void DrawArrow(const QStyleOptionSlider* option, QPainter* painter) const;

  static ColorList MoodbarColors(const QByteArray& data, MoodbarStyle style,
                                 const QPalette& palette);
  static QPixmap MoodbarPixmap(const ColorList& colors, const QSize& size,
                               const QPalette& palette);

private slots:
  void FaderValueChanged(qreal value);

private:
  QSlider* slider_;

  bool enabled_;
  QByteArray data_;
  MoodbarStyle moodbar_style_;

  State state_;
  QTimeLine* fade_timeline_;

  QPixmap fade_source_;
  QPixmap fade_target_;

  bool moodbar_colors_dirty_;
  bool moodbar_pixmap_dirty_;
  ColorList moodbar_colors_;
  QPixmap moodbar_pixmap_;
};

#endif // MOODBARPROXYSTYLE_H
