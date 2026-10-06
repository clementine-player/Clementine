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

#include "symbolicicon.h"

#include <QApplication>
#include <QHash>
#include <QIconEngine>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPixmapCache>
#include <QStringList>

bool UseSymbolicIcons() {
  const QByteArray forced = qgetenv("CLEMENTINE_SYMBOLIC_ICONS");
  if (forced == "1") return true;
  if (forced == "0") return false;

  // A colon-separated list, e.g. "GNOME" or "ubuntu:GNOME".
  const QStringList desktops =
      QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP")).split(':');
  for (const QString& desktop : desktops) {
    if (desktop.compare("GNOME", Qt::CaseInsensitive) == 0) return true;
  }
  return false;
}

QString SymbolicNameForIconName(const QString& icon_name) {
  // Only actions and places with a clear match. Clementine's own artwork
  // (the iPod, Hypnotoad, the kittens...) keeps its icons.
  static const QHash<QString, QString> kNames = {
      // Playback
      {"media-playback-start", "media-playback-start"},
      {"media-playback-pause", "media-playback-pause"},
      {"media-playback-stop", "media-playback-stop"},
      {"media-skip-forward", "media-skip-forward"},
      {"media-skip-backward", "media-skip-backward"},
      {"media-eject", "media-eject"},
      {"media-playlist-shuffle", "media-playlist-shuffle"},
      {"x-clementine-shuffle", "media-playlist-shuffle"},
      {"media-playlist-repeat", "media-playlist-repeat"},
      {"audio-volume-muted", "audio-volume-muted"},
      {"audio-volume-medium", "audio-volume-medium"},
      {"rate-enabled", "starred"},
      // Library and media
      {"folder-sound", "folder-music"},
      {"audio-x-generic", "audio-x-generic"},
      {"view-media-playlist", "view-list"},
      {"x-clementine-artist", "audio-input-microphone"},
      {"x-clementine-album", "media-optical-cd-audio"},
      {"media-optical", "media-optical"},
      {"view-media-lyrics", "media-view-subtitles"},
      {"weather-showers-scattered", "weather-showers-scattered"},
      // Devices
      {"multimedia-player-ipod-mini-blue", "multimedia-player"},
      {"ipodtouchicon", "phone"},
      {"phone", "phone"},
      {"wiimotedev", "input-gaming"},
      {"audio-card", "audio-card"},
      {"audio-headphones", "audio-headphones"},
      {"audio-headset", "audio-headset"},
      {"network-server", "network-server"},
      // Files and places
      {"folder", "folder"},
      {"folder-new", "folder-new"},
      {"document-open-folder", "folder-open"},
      {"document-open", "document-open"},
      {"document-new", "document-new"},
      {"document-save", "document-save"},
      {"document-open-remote", "insert-link"},
      {"download", "folder-download"},
      {"applications-internet", "web-browser"},
      {"internet-services", "network-workgroup"},
      {"go-home", "go-home"},
      // Editing
      {"list-add", "list-add"},
      {"list-remove", "list-remove"},
      {"edit-delete", "edit-delete"},
      {"edit-rename", "document-edit"},
      {"edit-copy", "edit-copy"},
      {"edit-undo", "edit-undo"},
      {"edit-redo", "edit-redo"},
      {"edit-clear-list", "edit-clear-all"},
      {"edit-clear-locationbar-ltr", "edit-clear"},
      {"edit-find", "edit-find"},
      {"system-search", "system-search"},
      {"search", "system-search"},
      {"zoom-in", "zoom-in"},
      // Navigation
      {"go-next", "go-next"},
      {"go-previous", "go-previous"},
      {"go-up", "go-up"},
      {"go-down", "go-down"},
      {"go-jump", "go-jump"},
      {"view-refresh", "view-refresh"},
      {"view-fullscreen", "view-fullscreen"},
      {"view-choose", "view-grid"},
      // Settings and dialogs
      {"configure", "emblem-system"},
      {"input-keyboard", "input-keyboard"},
      {"help-hint", "dialog-information"},
      {"help-about", "help-about"},
      {"about-info", "help-about"},
      {"dialog-warning", "dialog-warning"},
      {"dialog-ok-apply", "object-select"},
      {"cancel", "process-stop"},
      {"application-exit", "application-exit"},
      {"user-away", "user-away"},
      {"group", "system-users"},
  };
  return kNames.value(icon_name);
}

namespace {

QColor ColorFor(QIcon::Mode mode) {
  const QPalette palette = QApplication::palette();
  switch (mode) {
    case QIcon::Disabled:
      return palette.color(QPalette::Disabled, QPalette::WindowText);
    case QIcon::Selected:
      return palette.color(QPalette::Active, QPalette::HighlightedText);
    default:
      return palette.color(QPalette::Active, QPalette::WindowText);
  }
}

class SymbolicIconEngine : public QIconEngine {
 public:
  SymbolicIconEngine(const QString& name, const QIcon& source)
      : name_(name), source_(source) {}

  QIconEngine* clone() const override {
    return new SymbolicIconEngine(name_, source_);
  }
  QString key() const override { return "Symbolic"; }
  QString iconName() override { return name_ + "-symbolic"; }

  QSize actualSize(const QSize& size, QIcon::Mode, QIcon::State) override {
    return size;
  }

  QList<QSize> availableSizes(QIcon::Mode, QIcon::State) override {
    return {QSize(16, 16), QSize(22, 22), QSize(32, 32), QSize(48, 48)};
  }

  void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode,
             QIcon::State) override {
    // Code that paints an icon itself (the sidebar, item views) has set the
    // painter's pen to its text colour, often on a background of its own:
    // draw the icon in the same colour. Selected and disabled icons take
    // the palette's colours for them, which the pen doesn't reflect.
    const QColor color = mode == QIcon::Disabled || mode == QIcon::Selected
                             ? ColorFor(mode)
                             : painter->pen().color();
    const qreal scale =
        painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
    painter->drawPixmap(rect, Pixmap(rect.size(), color, scale));
  }

  QPixmap pixmap(const QSize& size, QIcon::Mode mode,
                 QIcon::State state) override {
    return scaledPixmap(size, mode, state, 1.0);
  }

  QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State,
                       qreal scale) override {
    return Pixmap(size, ColorFor(mode), scale);
  }

 private:
  QPixmap Pixmap(const QSize& size, const QColor& color, qreal scale) const {
    const QSize pixels = size * scale;
    const QString cache_key = QString("symbolic:%1:%2x%3:%4")
                                  .arg(name_)
                                  .arg(pixels.width())
                                  .arg(pixels.height())
                                  .arg(color.rgba());
    QPixmap cached;
    if (QPixmapCache::find(cache_key, &cached)) return cached;

    QPixmap pixmap = QPixmap::fromImage(Render(pixels, color));
    pixmap.setDevicePixelRatio(scale);
    QPixmapCache::insert(cache_key, pixmap);
    return pixmap;
  }

  QImage Render(const QSize& pixels, const QColor& color) const {
    QImage image(pixels, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    if (pixels.isEmpty()) return image;

    // The theme draws symbolic icons in one fixed colour; keep their shape
    // (the alpha) and fill it with ours.
    QPainter p(&image);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.drawPixmap(image.rect(), source_.pixmap(pixels, 1.0));
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(image.rect(), color);
    p.end();
    return image;
  }

  QString name_;
  QIcon source_;
};

}  // namespace

QIcon SymbolicIcon(const QString& name) {
  const QString symbolic_name = name + "-symbolic";
  if (!QIcon::hasThemeIcon(symbolic_name)) return QIcon();
  return QIcon(new SymbolicIconEngine(name, QIcon::fromTheme(symbolic_name)));
}
