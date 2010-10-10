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

#include "widgetfadehelper.h"

#include <QPainter>
#include <QTimeLine>
#include <QtDebug>

// Exported by QtGui
void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);

WidgetFadeHelper::WidgetFadeHelper(QWidget* parent, int msec)
  : QWidget(parent),
    parent_(parent),
    blur_timeline_(new QTimeLine(msec, this)),
    fade_timeline_(new QTimeLine(msec, this))
{
  setAttribute(Qt::WA_TransparentForMouseEvents);

  connect(blur_timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));
  connect(fade_timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));
  connect(fade_timeline_, SIGNAL(finished()), SLOT(FadeFinished()));

  hide();
}

void WidgetFadeHelper::StartBlur() {
  // Take a "screenshot" of the window
  original_pixmap_ = QPixmap::grabWidget(parent_);
  QImage original_image = original_pixmap_.toImage();

  // Blur it
  QImage blurred(original_image.size(), QImage::Format_ARGB32_Premultiplied);
  blurred.fill(Qt::transparent);

  QPainter blur_painter(&blurred);
  qt_blurImage(&blur_painter, original_image, 10.0, true, false);
  blur_painter.end();

  blurred_pixmap_ = QPixmap::fromImage(blurred);

  // Cover the parent
  resize(parent_->size());
  raise();
  show();

  // Start the timeline
  blur_timeline_->stop();
  blur_timeline_->start();
}

void WidgetFadeHelper::StartFade() {
  if (blur_timeline_->state() == QTimeLine::Running) {
    // Blur timeline is still running, so we need render the current state
    // into a new pixmap.
    QPixmap pixmap(original_pixmap_);
    QPainter painter(&pixmap);
    painter.setOpacity(blur_timeline_->currentValue());
    painter.drawPixmap(0, 0, blurred_pixmap_);
    painter.end();
    blurred_pixmap_ = pixmap;
  }
  blur_timeline_->stop();
  original_pixmap_ = QPixmap();

  // Start the timeline
  fade_timeline_->stop();
  fade_timeline_->start();
}

void WidgetFadeHelper::paintEvent(QPaintEvent* ) {
  QPainter p(this);

  if (fade_timeline_->state() != QTimeLine::Running) {
    // We're fading in the blur
    p.setOpacity(blur_timeline_->currentValue());
  } else {
    // Fading out the blur into the new image
    p.setOpacity(1.0 - fade_timeline_->currentValue());
  }

  p.drawPixmap(0, 0, blurred_pixmap_);
}

void WidgetFadeHelper::FadeFinished() {
  hide();
  original_pixmap_ = QPixmap();
  blurred_pixmap_ = QPixmap();
}
