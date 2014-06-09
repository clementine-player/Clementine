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

#ifndef FREESPACEBAR_H
#define FREESPACEBAR_H

#include <QWidget>

class FreeSpaceBar : public QWidget {
  Q_OBJECT

 public:
  FreeSpaceBar(QWidget* parent = nullptr);

  static const int kBarHeight;
  static const int kBarBorderRadius;
  static const int kMarkerSpacing;
  static const int kLabelBoxSize;
  static const int kLabelBoxPadding;
  static const int kLabelSpacing;

  static const QRgb kColorBg1;
  static const QRgb kColorBg2;
  static const QRgb kColorAdd1;
  static const QRgb kColorAdd2;
  static const QRgb kColorBar1;
  static const QRgb kColorBar2;
  static const QRgb kColorBorder;

  void set_free_bytes(qint64 bytes) {
    free_ = bytes;
    update();
  }
  void set_additional_bytes(qint64 bytes) {
    additional_ = bytes;
    update();
  }
  void set_total_bytes(qint64 bytes) {
    total_ = bytes;
    update();
  }

  void set_free_text(const QString& text) {
    free_text_ = text;
    update();
  }
  void set_additional_text(const QString& text) {
    additional_text_ = text;
    update();
  }
  void set_used_text(const QString& text) {
    used_text_ = text;
    update();
  }

  QSize sizeHint() const;

 protected:
  void paintEvent(QPaintEvent*);

 private:
  struct Label {
    Label(const QString& t, const QColor& c) : text(t), color(c) {}

    QString text;
    QColor color;
  };

  QString TextForSize(const QString& prefix, qint64 size) const;

  void DrawBar(QPainter* p, const QRect& r);
  void DrawText(QPainter* p, const QRect& r);

 private:
  qint64 free_;
  qint64 additional_;
  qint64 total_;

  QString free_text_;
  QString additional_text_;
  QString used_text_;
};

#endif  // FREESPACEBAR_H
