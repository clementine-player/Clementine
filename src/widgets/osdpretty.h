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

#ifndef OSDPRETTY_H
#define OSDPRETTY_H

#include <QWidget>

class Ui_OSDPretty;

class QTimeLine;

class OSDPretty : public QWidget {
  Q_OBJECT

 public:
  enum Mode { Mode_Popup, Mode_Draggable, };

  OSDPretty(Mode mode, QWidget* parent = nullptr);
  ~OSDPretty();

  static const char* kSettingsGroup;

  static const int kDropShadowSize;
  static const int kBorderRadius;
  static const int kMaxIconSize;

  static const int kSnapProximity;

  static const QRgb kPresetBlue;
  static const QRgb kPresetOrange;

  static bool IsTransparencyAvailable();

  void SetMessage(const QString& summary, const QString& message,
                  const QImage& image);
  void ShowMessage(const QString& summary, const QString& message,
                   const QImage& image);

  // Controls the fader.  This is enabled by default on Windows.
  void set_fading_enabled(bool enabled) { fading_enabled_ = enabled; }

  // Popup duration in seconds.  Only used in Mode_Popup.
  void set_popup_duration(int msec);

  // These will get overwritten when ReloadSettings() is called
  void set_foreground_color(QRgb color);
  void set_background_color(QRgb color);
  void set_background_opacity(qreal opacity);
  void set_font(QFont font);

  QRgb foreground_color() const { return foreground_color_.rgb(); }
  QRgb background_color() const { return background_color_.rgb(); }
  qreal background_opacity() const { return background_opacity_; }
  int popup_display() const { return popup_display_; }
  QPoint popup_pos() const { return popup_pos_; }
  QFont font() const { return font_; }
  bool disable_duration() const { return disable_duration_; }

  // When the user has been moving the popup, use these to get its current
  // position and screen.  Note that these return invalid values if the popup
  // is hidden.
  int current_display() const;
  QPoint current_pos() const;

  // QWidget
  void setVisible(bool visible);

  bool toggle_mode() const { return toggle_mode_; }
  void set_toggle_mode(bool toggle_mode) { toggle_mode_ = toggle_mode; }

 public slots:
  void ReloadSettings();

 protected:
  void paintEvent(QPaintEvent*);
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void mousePressEvent(QMouseEvent*);
  void showEvent(QShowEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);

 private:
  void Reposition();
  void Load();

  QRect BoxBorder() const;

 private slots:
  void FaderValueChanged(qreal value);
  void FaderFinished();

 private:
  Ui_OSDPretty* ui_;

  Mode mode_;

  // Settings loaded from QSettings
  QColor foreground_color_;
  QColor background_color_;
  float background_opacity_;
  int popup_display_;  // -1 for default
  QPoint popup_pos_;
  QFont font_;
  // The OSD is kept always on top until you click (no timer)
  bool disable_duration_;

  // Cached pixmaps
  QPixmap shadow_edge_[4];
  QPixmap shadow_corner_[4];
  QPixmap background_;

  // For dragging the OSD
  QPoint original_window_pos_;
  QPoint drag_start_pos_;

  // For timeout of notification
  QTimer* timeout_;

  // For fading
  bool fading_enabled_;
  QTimeLine* fader_;

  // Toggling requested, we have to show or hide the OSD
  bool toggle_mode_;
};

#endif  // OSDPRETTY_H
