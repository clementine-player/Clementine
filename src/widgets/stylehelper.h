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

#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QtGui/QColor>
#include <QtGui/QStyle>

#include "ui/qt_blurimage.h"

QT_BEGIN_NAMESPACE
class QPalette;
class QPainter;
class QRect;
QT_END_NAMESPACE

// Helper class holding all custom color values

namespace Utils {
class StyleHelper {
 public:
  static const unsigned int DEFAULT_BASE_COLOR = 0x666666;

  // Height of the project explorer navigation bar
  static qreal sidebarFontSize();

  // This is our color table, all colors derive from baseColor
  static QColor requestedBaseColor() { return m_requestedBaseColor; }
  static QColor baseColor(bool lightColored = false);
  static QColor panelTextColor(bool lightColored = false);
  static QColor highlightColor(bool lightColored = false);
  static QColor shadowColor(bool lightColored = false);
  static QColor borderColor(bool lightColored = false);

  static QColor sidebarHighlight() { return QColor(255, 255, 255, 40); }
  static QColor sidebarShadow() { return QColor(0, 0, 0, 40); }

  // Sets the base color and makes sure all top level widgets are updated
  static void setBaseColor(const QColor& color);

  // Gradients used for panels
  static void verticalGradient(QPainter* painter, const QRect& spanRect,
                               const QRect& clipRect,
                               bool lightColored = false);
  static bool usePixmapCache() { return true; }

  static void drawIconWithShadow(const QIcon& icon, const QRect& rect,
                                 QPainter* p, QIcon::Mode iconMode,
                                 int radius = 3,
                                 const QColor& color = QColor(0, 0, 0, 130),
                                 const QPoint& offset = QPoint(1, -2));

 private:
  static QColor m_baseColor;
  static QColor m_requestedBaseColor;
};

}  // namespace Utils

using Utils::StyleHelper;
#endif  // STYLEHELPER_H
