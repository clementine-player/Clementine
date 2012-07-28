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

#include "moodbarrenderer.h"

#include <QProxyStyle>

class Application;

class QActionGroup;
class QMenu;
class QSlider;
class QStyleOptionSlider;
class QTimeLine;

class MoodbarProxyStyle : public QProxyStyle {
  Q_OBJECT

public:
  MoodbarProxyStyle(Application* app, QSlider* slider);

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

private:
  void NextState();

  void Render(ComplexControl control, const QStyleOptionSlider* option,
              QPainter* painter, const QWidget* widget);
  void EnsureMoodbarRendered();
  void DrawArrow(const QStyleOptionSlider* option, QPainter* painter) const;
  void ShowContextMenu(const QPoint& pos);

  static QPixmap MoodbarPixmap(const ColorVector& colors,
                               const QSize& size, const QPalette& palette);

private slots:
  void ReloadSettings();
  void FaderValueChanged(qreal value);
  void ChangeStyle(QAction* action);

private:
  Application* app_;
  QSlider* slider_;

  bool enabled_;
  QByteArray data_;
  MoodbarRenderer::MoodbarStyle moodbar_style_;

  State state_;
  QTimeLine* fade_timeline_;

  QPixmap fade_source_;
  QPixmap fade_target_;

  bool moodbar_colors_dirty_;
  bool moodbar_pixmap_dirty_;
  ColorVector moodbar_colors_;
  QPixmap moodbar_pixmap_;

  QMenu* context_menu_;
  QAction* show_moodbar_action_;
  QActionGroup* style_action_group_;
};

#endif // MOODBARPROXYSTYLE_H
