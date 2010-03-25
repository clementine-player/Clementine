/* This file is part of Clementine.

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

#ifndef OSDPRETTY_H
#define OSDPRETTY_H

#include <QWidget>

#include "ui_osdpretty.h"

class OSDPretty : public QWidget {
  Q_OBJECT

 public:
  OSDPretty(QWidget *parent = 0);

  static const char* kSettingsGroup;

  static const int kDropShadowSize;
  static const int kBorderRadius;
  static const int kMaxIconSize;

  static const QRgb kPresetBlue;
  static const QRgb kPresetOrange;

  enum Mode {
    Mode_Popup,
    Mode_Draggable,
  };

  static bool IsTransparencyAvailable();

  void SetMode(Mode mode);
  void SetMessage(const QString& summary,
                  const QString& message,
                  const QImage& image);

  // Popup duration in seconds.  Only used in Mode_Popup.
  void set_popup_duration(int msec);

  // These will get overwritten when ReloadSettings() is called
  void set_foreground_color(QRgb color);
  void set_background_color(QRgb color);
  void set_background_opacity(qreal opacity);

  QRgb foreground_color() const { return foreground_color_.rgb(); }
  QRgb background_color() const { return background_color_.rgb(); }
  qreal background_opacity() const { return background_opacity_; }

  // When the user has been moving the popup, use these to get its current
  // position and screen
  int current_display() const;
  QPoint current_pos() const;

 public slots:
  void ReloadSettings();

 protected:
  void paintEvent(QPaintEvent *);
  void enterEvent(QEvent *);
  void leaveEvent(QEvent *);
  void mousePressEvent(QMouseEvent *);
  void showEvent(QShowEvent *);
  void mouseMoveEvent(QMouseEvent *);

 private:
  void Reposition();
  void Load();

  QRect BoxBorder() const;

 private:
  Ui::OSDPretty ui_;

  Mode mode_;

  // Settings loaded from QSettings
  QColor foreground_color_;
  QColor background_color_;
  float background_opacity_;
  int popup_display_; // -1 for default
  QPoint popup_pos_;

  // Cached pixmaps
  QPixmap shadow_edge_[4];
  QPixmap shadow_corner_[4];

  // For dragging the OSD
  QPoint original_window_pos_;
  QPoint drag_start_pos_;

  // For timeout of notification
  QTimer* timeout_;
};

#endif // OSDPRETTY_H
