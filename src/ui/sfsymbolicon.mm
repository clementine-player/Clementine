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

#include "sfsymbolicon.h"

#include <QApplication>
#include <QHash>
#include <QIconEngine>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPixmapCache>

#include <AppKit/AppKit.h>

#include <algorithm>

QString SFSymbolForIconName(const QString& icon_name) {
  // Only actions and places with a clear match. Clementine's own artwork
  // (the iPod, Hypnotoad, the kittens...) keeps its icons.
  static const QHash<QString, QString> kSymbols = {
      // Playback
      {"media-playback-start", "play.fill"},
      {"media-playback-pause", "pause.fill"},
      {"media-playback-stop", "stop.fill"},
      {"media-skip-forward", "forward.fill"},
      {"media-skip-backward", "backward.fill"},
      {"media-eject", "eject.fill"},
      {"media-playlist-shuffle", "shuffle"},
      {"x-clementine-shuffle", "shuffle"},
      {"media-playlist-repeat", "repeat"},
      {"audio-volume-muted", "speaker.slash.fill"},
      {"audio-volume-medium", "speaker.wave.2.fill"},
      {"rate-enabled", "star.fill"},
      // Library and media
      {"folder-sound", "music.note.house"},
      {"audio-x-generic", "music.note"},
      {"view-media-playlist", "music.note.list"},
      {"x-clementine-artist", "music.mic"},
      {"x-clementine-album", "square.stack"},
      {"x-clementine-albums", "music.note.list"},
      {"media-optical", "opticaldisc"},
      {"view-media-lyrics", "quote.bubble"},
      {"view-media-equalizer", "slider.vertical.3"},
      {"view-media-visualization", "paintpalette"},
      {"weather-showers-scattered", "cloud.rain"},
      // Devices
      {"multimedia-player-ipod-mini-blue", "externaldrive"},
      {"ipodtouchicon", "iphone.radiowaves.left.and.right"},
      {"wiimotedev", "gamecontroller"},
      // Files and places
      {"folder", "folder"},
      {"folder-new", "folder.badge.plus"},
      {"document-open-folder", "folder"},
      {"document-open", "doc"},
      {"document-new", "doc.badge.plus"},
      {"document-save", "square.and.arrow.down"},
      {"document-open-remote", "link"},
      {"download", "arrow.down.circle"},
      {"applications-internet", "globe"},
      {"internet-services", "network"},
      {"go-home", "house"},
      // Editing
      {"list-add", "plus"},
      {"list-remove", "minus"},
      {"edit-delete", "trash"},
      {"edit-rename", "pencil"},
      {"edit-copy", "doc.on.doc"},
      {"edit-undo", "arrow.uturn.backward"},
      {"edit-redo", "arrow.uturn.forward"},
      {"edit-clear-list", "clear"},
      {"edit-clear-locationbar-ltr", "delete.left"},
      {"edit-find", "magnifyingglass"},
      {"system-search", "magnifyingglass"},
      {"search", "magnifyingglass"},
      {"zoom-in", "plus.magnifyingglass"},
      // Navigation
      {"go-next", "chevron.forward"},
      {"go-previous", "chevron.backward"},
      {"go-up", "chevron.up"},
      {"go-down", "chevron.down"},
      {"go-jump", "arrow.turn.down.right"},
      {"view-refresh", "arrow.clockwise"},
      {"view-fullscreen", "arrow.up.left.and.arrow.down.right"},
      {"view-choose", "rectangle.grid.2x2"},
      // Settings and dialogs
      {"configure", "gearshape"},
      {"tools-wizard", "wand.and.stars"},
      {"input-keyboard", "keyboard"},
      {"help-hint", "lightbulb"},
      {"help-about", "info.circle"},
      {"about-info", "info.circle"},
      {"dialog-warning", "exclamationmark.triangle"},
      {"dialog-ok-apply", "checkmark"},
      {"cancel", "xmark"},
      {"application-exit", "power"},
  };
  return kSymbols.value(icon_name);
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

class SFSymbolIconEngine : public QIconEngine {
 public:
  explicit SFSymbolIconEngine(const QString& symbol_name) : symbol_name_(symbol_name) {}

  QIconEngine* clone() const override { return new SFSymbolIconEngine(symbol_name_); }
  QString key() const override { return "SFSymbol"; }

  QSize actualSize(const QSize& size, QIcon::Mode, QIcon::State) override { return size; }

  QList<QSize> availableSizes(QIcon::Mode, QIcon::State) override {
    return {QSize(16, 16), QSize(22, 22), QSize(32, 32), QSize(48, 48)};
  }

  void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override {
    // Code that paints an icon itself (the sidebar, item views) has set the
    // painter's pen to its text colour, often on a background of its own:
    // draw the symbol in the same colour. Selected and disabled icons take
    // the palette's colours for them, which the pen doesn't reflect.
    const QColor color = mode == QIcon::Disabled || mode == QIcon::Selected
                             ? ColorFor(mode)
                             : painter->pen().color();
    const qreal scale = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
    painter->drawPixmap(rect, Pixmap(rect.size(), color, scale));
  }

  QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override {
    return scaledPixmap(size, mode, state, 1.0);
  }

  QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State, qreal scale) override {
    return Pixmap(size, ColorFor(mode), scale);
  }

 private:
  QPixmap Pixmap(const QSize& size, const QColor& color, qreal scale) const {
    const QSize pixels = size * scale;
    const QString cache_key = QString("sfsymbol:%1:%2x%3:%4")
                                  .arg(symbol_name_)
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

    NSImage* symbol = [NSImage imageWithSystemSymbolName:symbol_name_.toNSString()
                                accessibilityDescription:nil];
    if (!symbol) return image;
    NSColor* ns_color = [NSColor colorWithSRGBRed:color.redF()
                                            green:color.greenF()
                                             blue:color.blueF()
                                            alpha:color.alphaF()];
    // In pixels: the context below is the image's pixels, not points.
    NSImageSymbolConfiguration* config =
        [[NSImageSymbolConfiguration configurationWithPointSize:pixels.height() * 0.75
                                                         weight:NSFontWeightRegular]
            configurationByApplyingConfiguration:[NSImageSymbolConfiguration
                                                     configurationWithPaletteColors:@[ ns_color ]]];
    NSImage* glyph = [symbol imageWithSymbolConfiguration:config];

    // Fit the symbol into the square with a little room, keeping its shape.
    const CGFloat box = 0.9;
    const CGFloat fit = std::min(pixels.width() * box / glyph.size.width,
                                 pixels.height() * box / glyph.size.height);
    const CGFloat width = glyph.size.width * fit;
    const CGFloat height = glyph.size.height * fit;

    CGColorSpaceRef color_space = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    CGContextRef context = CGBitmapContextCreate(
        image.bits(), image.width(), image.height(), 8, image.bytesPerLine(), color_space,
        kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
    CGColorSpaceRelease(color_space);
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithCGContext:context
                                                                                 flipped:NO]];
    [glyph drawInRect:NSMakeRect((pixels.width() - width) / 2, (pixels.height() - height) / 2,
                                 width, height)];
    [NSGraphicsContext restoreGraphicsState];
    CGContextRelease(context);
    return image;
  }

  QString symbol_name_;
};

}  // namespace

QIcon SFSymbolIcon(const QString& symbol_name) {
  if (![NSImage imageWithSystemSymbolName:symbol_name.toNSString() accessibilityDescription:nil]) {
    return QIcon();
  }
  return QIcon(new SFSymbolIconEngine(symbol_name));
}
