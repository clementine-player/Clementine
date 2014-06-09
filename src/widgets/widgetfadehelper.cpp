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

#include "widgetfadehelper.h"
#include "ui/qt_blurimage.h"

#include <QResizeEvent>
#include <QPainter>
#include <QTimeLine>
#include <QtDebug>

const int WidgetFadeHelper::kLoadingPadding = 9;
const int WidgetFadeHelper::kLoadingBorderRadius = 10;

WidgetFadeHelper::WidgetFadeHelper(QWidget* parent, int msec)
    : QWidget(parent),
      parent_(parent),
      blur_timeline_(new QTimeLine(msec, this)),
      fade_timeline_(new QTimeLine(msec, this)) {
  parent->installEventFilter(this);

  connect(blur_timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));
  connect(fade_timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));
  connect(fade_timeline_, SIGNAL(finished()), SLOT(FadeFinished()));

  hide();
}

bool WidgetFadeHelper::eventFilter(QObject* obj, QEvent* event) {
  // We're only interested in our parent's resize events
  if (obj != parent_ || event->type() != QEvent::Resize) return false;

  // Don't care if we're hidden
  if (!isVisible()) return false;

  QResizeEvent* re = static_cast<QResizeEvent*>(event);
  if (re->oldSize() == re->size()) {
    // Ignore phoney resize events
    return false;
  }

  // Get a new capture of the parent
  hide();
  CaptureParent();
  show();
  return false;
}

void WidgetFadeHelper::StartBlur() {
  CaptureParent();

  // Cover the parent
  raise();
  show();

  // Start the timeline
  blur_timeline_->stop();
  blur_timeline_->start();

  setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void WidgetFadeHelper::CaptureParent() {
  // Take a "screenshot" of the window
  original_pixmap_ = QPixmap::grabWidget(parent_);
  QImage original_image = original_pixmap_.toImage();

  // Blur it
  QImage blurred(original_image.size(), QImage::Format_ARGB32_Premultiplied);
  blurred.fill(Qt::transparent);

  QPainter blur_painter(&blurred);
  blur_painter.save();
  qt_blurImage(&blur_painter, original_image, 10.0, true, false);
  blur_painter.restore();

  // Draw some loading text over the top
  QFont loading_font(font());
  loading_font.setBold(true);
  QFontMetrics loading_font_metrics(loading_font);

  const QString loading_text = tr("Loading...");
  const QSize loading_size(
      kLoadingPadding * 2 + loading_font_metrics.width(loading_text),
      kLoadingPadding * 2 + loading_font_metrics.height());
  const QRect loading_rect((blurred.width() - loading_size.width()) / 2, 100,
                           loading_size.width(), loading_size.height());

  blur_painter.setRenderHint(QPainter::Antialiasing);
  blur_painter.setRenderHint(QPainter::HighQualityAntialiasing);

  blur_painter.translate(0.5, 0.5);
  blur_painter.setPen(QColor(200, 200, 200, 255));
  blur_painter.setBrush(QColor(200, 200, 200, 192));
  blur_painter.drawRoundedRect(loading_rect, kLoadingBorderRadius,
                               kLoadingBorderRadius);

  blur_painter.setPen(palette().brush(QPalette::Text).color());
  blur_painter.setFont(loading_font);
  blur_painter.drawText(loading_rect.translated(-1, -1), Qt::AlignCenter,
                        loading_text);
  blur_painter.translate(-0.5, -0.5);

  blur_painter.end();

  blurred_pixmap_ = QPixmap::fromImage(blurred);

  resize(parent_->size());
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

  setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void WidgetFadeHelper::paintEvent(QPaintEvent*) {
  QPainter p(this);

  if (fade_timeline_->state() != QTimeLine::Running) {
    // We're fading in the blur
    p.drawPixmap(0, 0, original_pixmap_);
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
