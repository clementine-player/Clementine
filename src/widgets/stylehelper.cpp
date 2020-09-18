/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "stylehelper.h"

#include <QApplication>
#include <QStyleOption>
#include <QWidget>
#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPixmapCache>

// Clamps float color values within (0, 255)
static int clamp(float x) {
  const int val = x > 255 ? 255 : static_cast<int>(x);
  return val < 0 ? 0 : val;
}

namespace Utils {

qreal StyleHelper::sidebarFontSize() {
#if defined(Q_OS_MAC)
  return 10;
#else
  return 7.5;
#endif
}

QColor StyleHelper::panelTextColor(bool lightColored) {
  if (!lightColored)
    return Qt::white;
  else
    return Qt::black;
}

// Invalid by default, setBaseColor needs to be called at least once
QColor StyleHelper::m_baseColor;
QColor StyleHelper::m_requestedBaseColor;

QColor StyleHelper::baseColor(bool lightColored) {
  if (!lightColored)
    return m_baseColor;
  else
    return m_baseColor.lighter(230);
}

QColor StyleHelper::highlightColor(bool lightColored) {
  QColor result = baseColor(lightColored);
  if (!lightColored)
    result.setHsv(result.hue(), clamp(result.saturation()),
                  clamp(result.value() * 1.16));
  else
    result.setHsv(result.hue(), clamp(result.saturation()),
                  clamp(result.value() * 1.06));
  return result;
}

QColor StyleHelper::shadowColor(bool lightColored) {
  QColor result = baseColor(lightColored);
  result.setHsv(result.hue(), clamp(result.saturation() * 1.1),
                clamp(result.value() * 0.70));
  return result;
}

QColor StyleHelper::borderColor(bool lightColored) {
  QColor result = baseColor(lightColored);
  result.setHsv(result.hue(), result.saturation(), result.value() / 2);
  return result;
}

// We try to ensure that the actual color used are within
// reasonalbe bounds while generating the actual baseColor
// from the users request.
void StyleHelper::setBaseColor(const QColor& newcolor) {
  m_requestedBaseColor = newcolor;

  QColor color;
  color.setHsv(newcolor.hue(), newcolor.saturation() * 0.7,
               64 + newcolor.value() / 3);

  if (color.isValid() && color != m_baseColor) {
    m_baseColor = color;
    for (QWidget* w : QApplication::topLevelWidgets()) {
      w->update();
    }
  }
}

static void verticalGradientHelper(QPainter* p, const QRect& spanRect,
                                   const QRect& rect, bool lightColored) {
  QColor highlight = StyleHelper::highlightColor(lightColored);
  QColor shadow = StyleHelper::shadowColor(lightColored);
  QLinearGradient grad(spanRect.topRight(), spanRect.topLeft());
  grad.setColorAt(0, highlight.lighter(117));
  grad.setColorAt(1, shadow.darker(109));
  p->fillRect(rect, grad);

  QColor light(255, 255, 255, 80);
  p->setPen(light);
  p->drawLine(rect.topRight() - QPoint(1, 0),
              rect.bottomRight() - QPoint(1, 0));
  QColor dark(0, 0, 0, 90);
  p->setPen(dark);
  p->drawLine(rect.topLeft(), rect.bottomLeft());
}

void StyleHelper::verticalGradient(QPainter* painter, const QRect& spanRect,
                                   const QRect& clipRect, bool lightColored) {
  if (StyleHelper::usePixmapCache()) {
    QString key;
    QColor keyColor = baseColor(lightColored);
    key = QString::asprintf("mh_vertical %d %d %d %d %d", spanRect.width(),
                            spanRect.height(), clipRect.width(),
                            clipRect.height(), keyColor.rgb());
    ;

    QPixmap pixmap;
    if (!QPixmapCache::find(key, pixmap)) {
      pixmap = QPixmap(clipRect.size());
      QPainter p(&pixmap);
      QRect rect(0, 0, clipRect.width(), clipRect.height());
      verticalGradientHelper(&p, spanRect, rect, lightColored);
      p.end();
      QPixmapCache::insert(key, pixmap);
    }

    painter->drawPixmap(clipRect.topLeft(), pixmap);
  } else {
    verticalGradientHelper(painter, spanRect, clipRect, lightColored);
  }
}

// Draws a cached pixmap with shadow
void StyleHelper::drawIconWithShadow(const QIcon& icon, const QRect& rect,
                                     QPainter* p, QIcon::Mode iconMode,
                                     int radius, const QColor& color,
                                     const QPoint& offset) {
  QPixmap cache;
  QString pixmapName = QString("icon %0 %1 %2")
                           .arg(icon.cacheKey())
                           .arg(iconMode)
                           .arg(rect.height());

  if (!QPixmapCache::find(pixmapName, cache)) {
    QPixmap px = icon.pixmap(rect.size());
    cache = QPixmap(px.size() + QSize(radius * 2, radius * 2));
    cache.fill(Qt::transparent);

    QPainter cachePainter(&cache);
    if (iconMode == QIcon::Disabled) {
      QImage im = px.toImage().convertToFormat(QImage::Format_ARGB32);
      for (int y = 0; y < im.height(); ++y) {
        QRgb* scanLine = (QRgb*)im.scanLine(y);
        for (int x = 0; x < im.width(); ++x) {
          QRgb pixel = *scanLine;
          char intensity = qGray(pixel);
          *scanLine = qRgba(intensity, intensity, intensity, qAlpha(pixel));
          ++scanLine;
        }
      }
      px = QPixmap::fromImage(im);
    }

    // Draw shadow
    QImage tmp(px.size() + QSize(radius * 2, radius * 2 + 1),
               QImage::Format_ARGB32_Premultiplied);
    tmp.fill(Qt::transparent);

    QPainter tmpPainter(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tmpPainter.drawPixmap(QPoint(radius, radius), px);
    tmpPainter.end();

    // blur the alpha channel
    QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(Qt::transparent);
    QPainter blurPainter(&blurred);
    qt_blurImage(&blurPainter, tmp, radius, false, true);
    blurPainter.end();

    tmp = blurred;

    // blacken the image...
    tmpPainter.begin(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tmpPainter.fillRect(tmp.rect(), color);
    tmpPainter.end();

    tmpPainter.begin(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tmpPainter.fillRect(tmp.rect(), color);
    tmpPainter.end();

    // draw the blurred drop shadow...
    cachePainter.drawImage(
        QRect(0, 0, cache.rect().width(), cache.rect().height()), tmp);

    // Draw the actual pixmap...
    cachePainter.drawPixmap(QPoint(radius, radius) + offset, px);
    QPixmapCache::insert(pixmapName, cache);
  }

  QRect targetRect = cache.rect();
  targetRect.moveCenter(rect.center());
  p->drawPixmap(targetRect.topLeft() - offset, cache);
}

}  // namespace Utils
