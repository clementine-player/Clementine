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

#ifndef WIDGETFADEHELPER_H
#define WIDGETFADEHELPER_H

#include <QWidget>

class QTimeLine;

class WidgetFadeHelper : public QWidget {
  Q_OBJECT

 public:
  WidgetFadeHelper(QWidget* parent, int msec = 500);

 public slots:
  void StartBlur();
  void StartFade();

 protected:
  void paintEvent(QPaintEvent*);
  bool eventFilter(QObject* obj, QEvent* event);

 private slots:
  void FadeFinished();

 private:
  void CaptureParent();

 private:
  static const int kLoadingPadding;
  static const int kLoadingBorderRadius;

  QWidget* parent_;
  QTimeLine* blur_timeline_;
  QTimeLine* fade_timeline_;

  QPixmap original_pixmap_;
  QPixmap blurred_pixmap_;
};

#endif  // WIDGETFADEHELPER_H
