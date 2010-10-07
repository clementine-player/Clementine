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

#include "prettyimageview.h"
#include "core/networkaccessmanager.h"

#include <QApplication>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTimeLine>
#include <QtDebug>

const int PrettyImageView::kArrowWidth = 35;
const int PrettyImageView::kImageHeight = 160;
const int PrettyImageView::kTotalHeight = 200;
const int PrettyImageView::kBorderHeight = 10;

const int PrettyImageView::kBaseAnimationDuration = 500; // msec
const int PrettyImageView::kArrowAnimationDuration = 250; // msec

PrettyImageView::PrettyImageView(NetworkAccessManager* network, QWidget* parent)
  : QWidget(parent),
    network_(network),
    next_image_request_id_(1),
    current_index_(0),
    base_timeline_(new QTimeLine(kBaseAnimationDuration, this)),
    left_timeline_(new QTimeLine(kArrowAnimationDuration, this)),
    right_timeline_(new QTimeLine(kArrowAnimationDuration, this))
{
  setMouseTracking(true);
  setMinimumHeight(kTotalHeight);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  connect(base_timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));
  connect(left_timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));
  connect(right_timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));
}

QRect PrettyImageView::left() const {
  return QRect(0, 0, kArrowWidth, height());
}

QRect PrettyImageView::right() const {
  return QRect(width() - kArrowWidth, 0, kArrowWidth, height());
}

QPolygon PrettyImageView::arrow(const QRect& rect, int direction) const {
  QPoint point(direction > 0 ? rect.right() : rect.left(),
               (rect.bottom() - rect.top()) / 2 + rect.top());

  return QPolygon()
      << point
      << QPoint(point.x() - direction * kArrowWidth, point.y() - kArrowWidth)
      << QPoint(point.x() - direction * kArrowWidth, point.y() + kArrowWidth);
}

void PrettyImageView::Clear() {
  images_.clear();
  image_requests_.clear();
  current_index_ = 0;
  update();
}

void PrettyImageView::AddImage(const QUrl& url) {
  const int index = images_.count();
  const int id = next_image_request_id_ ++;

  // Add the image to the list
  images_ << Image(url);

  // Start fetching the image
  network_->Get(url, this, "ImageFetched", id);
  image_requests_[id] = index;
}

void PrettyImageView::ImageFetched(quint64 id, QNetworkReply* reply) {
  reply->deleteLater();

  if (!image_requests_.contains(id))
    return;

  Image& data = images_[image_requests_.take(id)];

  QImage image = QImage::fromData(reply->readAll());
  if (image.isNull())
    return;

  data.SetImage(image);
  update();
}

void PrettyImageView::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);

  p.setBrush(palette().color(QPalette::Highlight));
  p.setPen(QPen(palette().color(QPalette::Text), 0.5));

  // Draw left arrow
  p.setOpacity(base_timeline_->currentValue() * (0.5 + left_timeline_->currentValue() * 0.5));
  p.drawConvexPolygon(arrow(left(),  -1));

  // Draw right arrow
  p.setOpacity(base_timeline_->currentValue() * (0.5 + right_timeline_->currentValue() * 0.5));
  p.drawConvexPolygon(arrow(right(), +1));

  // Draw the current image
  if (current_index_ >= 0 && current_index_ < images_.count()) {
    QRect rect(0, 0, width(), kImageHeight - kBorderHeight);

    p.setOpacity(1.0);
    DrawImage(&p, rect, images_[current_index_]);
  }
}

void PrettyImageView::DrawImage(QPainter* p, const QRect& rect, const Image& image) {
  const int width = image.image_.isNull() ? rect.height() * 1.6 :
                    float(image.image_.width()) / image.image_.height() * rect.height();

  // Center the image in the rectangle
  QRect actual_rect(0, kBorderHeight, width, rect.height());
  actual_rect.moveLeft((rect.width() - width) / 2);

  // Draw the main image
  DrawThumbnail(p, actual_rect, image);

  // Draw the reflection
  // Figure out where to draw it
  QRect reflection_rect(actual_rect);
  reflection_rect.moveTop(reflection_rect.bottom());

  // Create the reflected pixmap
  QImage reflection(reflection_rect.size(), QImage::Format_ARGB32_Premultiplied);
  reflection.fill(palette().color(QPalette::Base).rgba());
  QPainter reflection_painter(&reflection);

  // Set up the transformation
  QTransform transform;
  transform.scale(1.0, -1.0);
  transform.translate(0.0, -actual_rect.height());
  reflection_painter.setTransform(transform);

  QRect fade_rect(reflection.rect().bottomLeft() - QPoint(0, kTotalHeight - kImageHeight),
                  reflection.rect().bottomRight());

  // Draw the reflection into the buffer
  DrawThumbnail(&reflection_painter, reflection.rect(), image);

  // Make it fade out towards the bottom
  QLinearGradient fade_gradient(fade_rect.topLeft(), fade_rect.bottomLeft());
  fade_gradient.setColorAt(0.0, QColor(0, 0, 0, 0));
  fade_gradient.setColorAt(1.0, QColor(0, 0, 0, 128));

  reflection_painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
  reflection_painter.fillRect(fade_rect, fade_gradient);

  reflection_painter.end();

  // Draw the reflection on the image
  p->drawImage(reflection_rect, reflection);
}

void PrettyImageView::DrawThumbnail(QPainter* p, const QRect& rect, const Image& image) {
  if (image.image_.isNull()) {
    // Draw an empty box if there's no image to show
    p->setPen(palette().color(QPalette::Disabled, QPalette::Text));
    p->drawText(rect, Qt::AlignHCenter | Qt::AlignBottom, tr("Loading..."));
  } else {
    // Draw the image
    p->drawPixmap(rect, image.thumbnail_);
  }
}

void PrettyImageView::mouseReleaseEvent(QMouseEvent* e) {
  if (left().contains(e->pos()))
    current_index_ = qMax(0, current_index_ - 1);
  else if (right().contains(e->pos()))
    current_index_ = qMin(images_.count() - 1, current_index_ + 1);
  update();
}

void PrettyImageView::mouseMoveEvent(QMouseEvent* e) {
  SetTimeLineActive(left_timeline_, left().contains(e->pos()));
  SetTimeLineActive(right_timeline_, right().contains(e->pos()));
}

void PrettyImageView::enterEvent(QEvent*) {
  SetTimeLineActive(base_timeline_, true);
}

void PrettyImageView::leaveEvent(QEvent*) {
  SetTimeLineActive(base_timeline_, false);
}

void PrettyImageView::SetTimeLineActive(QTimeLine* timeline, bool active) {
  const QTimeLine::Direction direction =
      active ? QTimeLine::Forward : QTimeLine::Backward;

  if (timeline->state() == QTimeLine::Running && timeline->direction() == direction)
    return;

  timeline->setDirection(direction);

  if (timeline->state() != QTimeLine::Running)
    timeline->resume();
}

void PrettyImageView::Image::SetImage(const QImage& image) {
  image_ = image;
  thumbnail_ = QPixmap::fromImage(image_.scaledToHeight(
      kImageHeight, Qt::SmoothTransformation));
}
