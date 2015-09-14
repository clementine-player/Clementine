/***************************************************************************
                        amarokslider.cpp  -  description
                           -------------------
  begin                : Dec 15 2003
  copyright            : (C) 2003 by Mark Kretschmann
  email                : markey@web.de
  copyright            : (C) 2005 by Gábor Lehel
  email                : illissius@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sliderwidget.h"

#include <QApplication>
#include <QBitmap>
#include <QBrush>
#include <QImage>
#include <QPainter>
#include <QSize>
#include <QTimer>
#include <QStyle>
#include <QMenu>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QStyleOptionSlider>

Amarok::Slider::Slider(Qt::Orientation orientation, QWidget* parent, uint max)
    : QSlider(orientation, parent),
      m_sliding(false),
      m_outside(false),
      m_prevValue(0) {
  setRange(0, max);
}

void Amarok::Slider::wheelEvent(QWheelEvent* e) {
  if (orientation() == Qt::Vertical) {
    // Will be handled by the parent widget
    e->ignore();
    return;
  }

  // Position Slider (horizontal)
  int step = e->delta() * 1500 / 18;
  int nval = QSlider::value() + step;
  nval = qMax(nval, minimum());
  nval = qMin(nval, maximum());

  QSlider::setValue(nval);

  emit sliderReleased(value());
}

void Amarok::Slider::mouseMoveEvent(QMouseEvent* e) {
  if (m_sliding) {
    // feels better, but using set value of 20 is bad of course
    QRect rect(-20, -20, width() + 40, height() + 40);

    if (orientation() == Qt::Horizontal && !rect.contains(e->pos())) {
      if (!m_outside) QSlider::setValue(m_prevValue);
      m_outside = true;
    } else {
      m_outside = false;
      slideEvent(e);
      emit sliderMoved(value());
    }
  } else
    QSlider::mouseMoveEvent(e);
}

void Amarok::Slider::slideEvent(QMouseEvent* e) {
  QStyleOptionSlider option;
  initStyleOption(&option);
  QRect sliderRect(style()->subControlRect(QStyle::CC_Slider, &option,
                                           QStyle::SC_SliderHandle, this));

  QSlider::setValue(
      orientation() == Qt::Horizontal
          ? ((QApplication::layoutDirection() == Qt::RightToLeft)
                 ? QStyle::sliderValueFromPosition(
                       minimum(), maximum(),
                       width() - (e->pos().x() - sliderRect.width() / 2),
                       width() + sliderRect.width(), true)
                 : QStyle::sliderValueFromPosition(
                       minimum(), maximum(),
                       e->pos().x() - sliderRect.width() / 2,
                       width() - sliderRect.width()))
          : QStyle::sliderValueFromPosition(
                minimum(), maximum(), e->pos().y() - sliderRect.height() / 2,
                height() - sliderRect.height()));
}

void Amarok::Slider::mousePressEvent(QMouseEvent* e) {
  QStyleOptionSlider option;
  initStyleOption(&option);
  QRect sliderRect(style()->subControlRect(QStyle::CC_Slider, &option,
                                           QStyle::SC_SliderHandle, this));

  m_sliding = true;
  m_prevValue = QSlider::value();

  if (!sliderRect.contains(e->pos())) mouseMoveEvent(e);
}

void Amarok::Slider::mouseReleaseEvent(QMouseEvent*) {
  if (!m_outside && QSlider::value() != m_prevValue)
    emit sliderReleased(value());

  m_sliding = false;
  m_outside = false;
}

void Amarok::Slider::setValue(int newValue) {
  // don't adjust the slider while the user is dragging it!

  if (!m_sliding || m_outside)
    QSlider::setValue(adjustValue(newValue));
  else
    m_prevValue = newValue;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// CLASS PrettySlider
//////////////////////////////////////////////////////////////////////////////////////////

#define THICKNESS 7
#define MARGIN 3

Amarok::PrettySlider::PrettySlider(Qt::Orientation orientation, SliderMode mode,
                                   QWidget* parent, uint max)
    : Amarok::Slider(orientation, parent, max), m_mode(mode) {
  if (m_mode == Pretty) {
    setFocusPolicy(Qt::NoFocus);
  }
}

void Amarok::PrettySlider::mousePressEvent(QMouseEvent* e) {
  Amarok::Slider::mousePressEvent(e);

  slideEvent(e);
}

void Amarok::PrettySlider::slideEvent(QMouseEvent* e) {
  if (m_mode == Pretty)
    QSlider::setValue(
        orientation() == Qt::Horizontal
            ? QStyle::sliderValueFromPosition(minimum(), maximum(),
                                              e->pos().x(), width() - 2)
            : QStyle::sliderValueFromPosition(minimum(), maximum(),
                                              e->pos().y(), height() - 2));
  else
    Amarok::Slider::slideEvent(e);
}

namespace Amarok {
namespace ColorScheme {
extern QColor Background;
extern QColor Foreground;
}
}

#if 0
/** these functions aren't required in our fixed size world,
    but they may become useful one day **/

QSize
Amarok::PrettySlider::minimumSizeHint() const
{
    return sizeHint();
}

QSize
Amarok::PrettySlider::sizeHint() const
{
    constPolish();

    return (orientation() == Horizontal
             ? QSize( maxValue(), THICKNESS + MARGIN )
             : QSize( THICKNESS + MARGIN, maxValue() )).expandedTo( QApplit ication::globalStrut() );
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
/// CLASS VolumeSlider
//////////////////////////////////////////////////////////////////////////////////////////

Amarok::VolumeSlider::VolumeSlider(QWidget* parent, uint max)
    : Amarok::Slider(Qt::Horizontal, parent, max),
      m_animCount(0),
      m_animTimer(new QTimer(this)),
      m_pixmapInset(QPixmap(volumePixmapDraw ())) {
  setFocusPolicy(Qt::NoFocus);

  // BEGIN Calculate handle animation pixmaps for mouse-over effect
  QImage pixmapHandle(":volumeslider-handle.png");
  QImage pixmapHandleGlow(":volumeslider-handle_glow.png");

  float opacity = 0.0;
  const float step = 1.0 / ANIM_MAX;
  QImage dst;
  for (int i = 0; i < ANIM_MAX; ++i) {
    dst = pixmapHandle.copy();

    QPainter p(&dst);
    p.setOpacity(opacity);
    p.drawImage(0, 0, pixmapHandleGlow);
    p.end();

    m_handlePixmaps.append(QPixmap::fromImage(dst));
    opacity += step;
  }
  // END

  generateGradient();

  setMinimumWidth(m_pixmapInset.width());
  setMinimumHeight(m_pixmapInset.height());

  connect(m_animTimer, SIGNAL(timeout()), this, SLOT(slotAnimTimer()));
}

void Amarok::VolumeSlider::generateGradient() {
  const QImage mask(":volumeslider-gradient.png");

  QImage gradient_image(mask.size(), QImage::Format_ARGB32_Premultiplied);
  QPainter p(&gradient_image);

  QLinearGradient gradient(gradient_image.rect().topLeft(),
                           gradient_image.rect().topRight());
  gradient.setColorAt(0, palette().color(QPalette::Background));
  gradient.setColorAt(1, palette().color(QPalette::Highlight));
  p.fillRect(gradient_image.rect(), QBrush(gradient));

  p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
  p.drawImage(0, 0, mask);
  p.end();

  m_pixmapGradient = QPixmap::fromImage(gradient_image);
}

void Amarok::VolumeSlider::slotAnimTimer()  // SLOT
{
  if (m_animEnter) {
    m_animCount++;
    update();
    if (m_animCount == ANIM_MAX - 1) m_animTimer->stop();
  } else {
    m_animCount--;
    update();
    if (m_animCount == 0) m_animTimer->stop();
  }
}

void Amarok::VolumeSlider::mousePressEvent(QMouseEvent* e) {
  if (e->button() != Qt::RightButton) {
    Amarok::Slider::mousePressEvent(e);
    slideEvent(e);
  }
}

void Amarok::VolumeSlider::contextMenuEvent(QContextMenuEvent* e) {
  QMap<QAction*, int> values;
  QMenu menu;
  menu.setTitle("Volume");
  values[menu.addAction("100%")] = 100;
  values[menu.addAction("80%")] = 80;
  values[menu.addAction("60%")] = 60;
  values[menu.addAction("40%")] = 40;
  values[menu.addAction("20%")] = 20;
  values[menu.addAction("0%")] = 0;

  QAction* ret = menu.exec(mapToGlobal(e->pos()));
  if (ret) {
    QSlider::setValue(values[ret]);
    emit sliderReleased(values[ret]);
  }
}

void Amarok::VolumeSlider::slideEvent(QMouseEvent* e) {
  QSlider::setValue(QStyle::sliderValueFromPosition(minimum(), maximum(),
                                                    e->pos().x(), width() - 2));
}

void Amarok::VolumeSlider::wheelEvent(QWheelEvent* e) {
  const uint step = e->delta() / 30;
  QSlider::setValue(QSlider::value() + step);

  emit sliderReleased(value());
}

void Amarok::VolumeSlider::paintEvent(QPaintEvent*) {
  QPainter p(this);

  const int padding = 7;
  const int offset = int(double((width() - 2 * padding) * value()) / maximum());

  m_pixmapInset = volumePixmapDraw();

  p.drawPixmap(0, 0, m_pixmapGradient, 0, 0, offset + padding, 0);
  p.drawPixmap(0, 0, m_pixmapInset);
  p.drawPixmap(offset - m_handlePixmaps[0].width() / 2 + padding, 0,
               m_handlePixmaps[m_animCount]);

  // Draw percentage number
  QStyleOptionViewItem opt;
  p.setPen(opt.palette.color(QPalette::Disabled, QPalette::Text));
  QFont vol_font(opt.font);
  vol_font.setPixelSize(9);
  p.setFont(vol_font);
  const QRect rect(0, 0, 34, 15);
  p.drawText(rect, Qt::AlignRight | Qt::AlignVCenter,
             QString::number(value()) + '%');
}

void Amarok::VolumeSlider::enterEvent(QEvent*) {
  m_animEnter = true;
  m_animCount = 0;

  m_animTimer->start(ANIM_INTERVAL);
}

void Amarok::VolumeSlider::leaveEvent(QEvent*) {
  // This can happen if you enter and leave the widget quickly
  if (m_animCount == 0) m_animCount = 1;

  m_animEnter = false;
  m_animTimer->start(ANIM_INTERVAL);
}

void Amarok::VolumeSlider::paletteChange(const QPalette&) {
  generateGradient();
}

QPixmap Amarok::VolumeSlider::volumePixmapDraw () const {
  QPixmap pixmap(112, 36);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  QPen pen(palette().color(QPalette::WindowText), 0.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
  painter.setPen(pen);
  
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  // Draw volume control pixmap
  QPolygon poly;
  poly << QPoint(6, 21) << QPoint(104, 21)
       << QPoint(104, 7) << QPoint(6, 16)
       << QPoint(6, 21);
  QPainterPath path;
  path.addPolygon(poly);
  painter.drawPolygon(poly);
  painter.drawLine(6, 29, 104, 29);
  // Return QPixmap
  return pixmap;
}
