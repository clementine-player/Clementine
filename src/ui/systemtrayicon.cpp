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

#include "systemtrayicon.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QSystemTrayIcon>
#include <QWheelEvent>
#include <QWidget>
#include <QtDebug>
#include <cmath>

#include "core/appearance.h"
#include "macsystemtrayicon.h"
#include "qtsystemtrayicon.h"

namespace {

// GNOME Shell's top bar is black whatever the colour scheme. GNOME Classic's
// panel is light, though it names GNOME too, as "GNOME-Classic:GNOME".
bool PanelIsAlwaysDark() {
  static const bool gnome = [] {
    const QList<QByteArray> desktops =
        qgetenv("XDG_CURRENT_DESKTOP").split(':');
    return desktops.contains("GNOME") && !desktops.contains("GNOME-Classic");
  }();
  return gnome;
}

}  // namespace

SystemTrayIcon::SystemTrayIcon(QObject* parent)
    : QObject(parent), percentage_(0) {}

QPixmap SystemTrayIcon::CreateProgressIcon(const QPixmap& icon,
                                           const QPixmap& grey_icon) {
  QRect rect(icon.rect());

  // The angle of the line that's used to cover the icon.
  // Centered on rect.topRight()
  double angle = double(100 - song_progress()) / 100.0 * M_PI_2 + M_PI;
  double length = sqrt(pow(rect.width(), 2.0) + pow(rect.height(), 2.0));

  QPolygon mask;
  mask << rect.topRight();
  mask << rect.topRight() + QPoint(length * sin(angle), -length * cos(angle));

  if (song_progress() > 50) mask << rect.bottomLeft();

  mask << rect.topLeft();
  mask << rect.topRight();

  QPixmap ret(icon);
  QPainter p(&ret);

  // Draw the grey bit over the orange icon
  p.setClipRegion(mask);
  p.drawPixmap(0, 0, grey_icon);
  p.end();

  return ret;
}

QPixmap SystemTrayIcon::CreateIcon(const QPixmap& icon,
                                   const QPixmap& grey_icon) {
  QPixmap ret(CreateProgressIcon(icon, grey_icon));
  if (playback_state() == PlaybackState::Stopped) return ret;

  QPainter p(&ret);
  p.setRenderHint(QPainter::Antialiasing);

  // A round badge in the bottom right with a play triangle or pause bars, as
  // on the macOS Dock: light on a dark panel and dark on a light one. The
  // panel's colour can't be asked for, so it's taken to follow the theme,
  // except where it's known always to be dark.
  const bool dark =
      PanelIsAlwaysDark() || Appearance::IsDarkPalette(QApplication::palette());
  const qreal size = ret.width() / ret.devicePixelRatio();
  const qreal diameter = size * 0.55;
  const QRectF badge(size - diameter, size - diameter, diameter, diameter);
  p.setPen(Qt::NoPen);
  p.setBrush(dark ? QColor(250, 250, 250) : QColor(33, 33, 33, 240));
  p.drawEllipse(badge);

  p.setBrush(dark ? QColor(33, 33, 33) : QColor(Qt::white));
  const QPointF centre = badge.center();
  const qreal glyph = diameter * 0.52;
  if (playback_state() == PlaybackState::Playing) {
    // The triangle's weight is left of its centre, so nudge it right.
    const qreal left = centre.x() - glyph * 0.4;
    const QPointF points[] = {
        QPointF(left, centre.y() - glyph / 2),
        QPointF(left, centre.y() + glyph / 2),
        QPointF(left + glyph * 0.9, centre.y()),
    };
    p.drawPolygon(points, 3);
  } else {
    const qreal bar = glyph * 0.32;
    const qreal gap = glyph * 0.28;
    p.drawRect(
        QRectF(centre.x() - gap / 2 - bar, centre.y() - glyph / 2, bar, glyph));
    p.drawRect(
        QRectF(centre.x() + gap / 2, centre.y() - glyph / 2, bar, glyph));
  }

  p.end();
  return ret;
}

void SystemTrayIcon::SetProgress(int percentage) {
  percentage_ = percentage;
  UpdateIcon();
}

void SystemTrayIcon::SetPaused() {
  playback_state_ = PlaybackState::Paused;
  UpdateIcon();
}

void SystemTrayIcon::SetPlaying(bool enable_play_pause, bool enable_love) {
  playback_state_ = PlaybackState::Playing;
  UpdateIcon();
}

void SystemTrayIcon::SetStopped() {
  playback_state_ = PlaybackState::Stopped;
  UpdateIcon();
}

SystemTrayIcon* SystemTrayIcon::CreateSystemTrayIcon(QObject* parent) {
#ifdef Q_OS_DARWIN
  return new MacSystemTrayIcon(parent);
#else
  if (QSystemTrayIcon::isSystemTrayAvailable())
    return new QtSystemTrayIcon(parent);
  else
    return nullptr;
#endif
}
