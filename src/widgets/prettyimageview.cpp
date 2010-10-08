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
#include "ui/iconloader.h"

#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QPainter>
#include <QScrollArea>
#include <QSettings>
#include <QStyle>
#include <QStyleOption>
#include <QTimeLine>
#include <QtDebug>

const int PrettyImageView::kEdgeWidth = 50;
const int PrettyImageView::kImageHeight = 160;
const int PrettyImageView::kTotalHeight = 200;
const int PrettyImageView::kBorderHeight = 5;
const int PrettyImageView::kEdgePadding = 5;

const int PrettyImageView::kBaseAnimationDuration = 500; // msec
const int PrettyImageView::kArrowAnimationDuration = 250; // msec

const char* PrettyImageView::kSettingsGroup = "PrettyImageView";

PrettyImageView::PrettyImageView(NetworkAccessManager* network, QWidget* parent)
  : QWidget(parent),
    network_(network),
    next_image_request_id_(1),
    current_index_(0),
    left_timeline_(new QTimeLine(kArrowAnimationDuration, this)),
    right_timeline_(new QTimeLine(kArrowAnimationDuration, this)),
    menu_(NULL)
{
  setMouseTracking(true);
  setMinimumHeight(kTotalHeight);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  connect(left_timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));
  connect(right_timeline_, SIGNAL(valueChanged(qreal)), SLOT(update()));

  QSettings s;
  s.beginGroup(kSettingsGroup);
  last_save_dir_ = s.value("last_save_dir", QDir::homePath()).toString();
}

QRect PrettyImageView::left() const {
  return QRect(0, 0, kEdgeWidth, height());
}

QRect PrettyImageView::right() const {
  return QRect(width() - kEdgeWidth, 0, kEdgeWidth, height());
}

QRect PrettyImageView::middle() const {
  return QRect(kEdgeWidth + kEdgePadding, kBorderHeight,
               width() - (kEdgeWidth + kEdgePadding) * 2, kImageHeight - kBorderHeight);
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

  const int next_index = current_index_ + 1;
  const int prev_index = current_index_ - 1;
  const int image_width = width() - kEdgeWidth * 2;

  const qreal next_opacity = 0.5 + right_timeline_->currentValue() * 0.5;
  const qreal prev_opacity = 0.5 + left_timeline_->currentValue() * 0.5;

  const QRect current_rect(kEdgeWidth + kEdgePadding, kBorderHeight,
                           image_width - kEdgePadding*2, kImageHeight - kBorderHeight);
  const QRect next_rect(width() - kEdgeWidth, kBorderHeight,
                        image_width, kImageHeight - kBorderHeight);
  const QRect prev_rect(-image_width + kEdgeWidth, kBorderHeight,
                        image_width, kImageHeight - kBorderHeight);

  // Draw the images
  DrawImage(&p, current_rect, Qt::AlignHCenter, 1.0, current_index_);
  DrawImage(&p, next_rect, Qt::AlignLeft, next_opacity, next_index);
  DrawImage(&p, prev_rect, Qt::AlignRight, prev_opacity, prev_index);
}

void PrettyImageView::DrawImage(QPainter* p, const QRect& rect, Qt::Alignment align,
                                qreal opacity, int image_index) {
  if (image_index < 0 || image_index >= images_.count())
    return;
  const Image& image = images_[image_index];

  QSize image_size = image.image_.isNull() ? QSize(100, 160) : image.image_.size();

  // Scale the image rect to fit in the rectangle
  image_size.scale(rect.size(), Qt::KeepAspectRatio);

  // Center the image in the rectangle and move it to the bottom
  QRect draw_rect(QPoint(0, 0), image_size);
  if (align & Qt::AlignLeft)
    draw_rect.moveBottomLeft(rect.bottomLeft());
  else if (align & Qt::AlignRight)
    draw_rect.moveBottomRight(rect.bottomRight());
  else if (align & Qt::AlignHCenter)
    draw_rect.moveBottomLeft(rect.bottomLeft() + QPoint((rect.width() - draw_rect.width()) / 2, 0));

  // Draw the main image
  p->setOpacity(opacity);
  DrawThumbnail(p, draw_rect, image);

  // Draw the reflection
  // Figure out where to draw it
  QRect reflection_rect(draw_rect);
  reflection_rect.moveTop(reflection_rect.bottom());

  // Create the reflected pixmap
  QImage reflection(reflection_rect.size(), QImage::Format_ARGB32_Premultiplied);
  reflection.fill(palette().color(QPalette::Base).rgba());
  QPainter reflection_painter(&reflection);

  // Set up the transformation
  QTransform transform;
  transform.scale(1.0, -1.0);
  transform.translate(0.0, -draw_rect.height());
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
  else if (middle().contains(e->pos()))
    ShowFullsize();

  update();
}

void PrettyImageView::mouseMoveEvent(QMouseEvent* e) {
  const bool in_left = left().contains(e->pos());
  const bool in_right = right().contains(e->pos());
  const bool in_middle = middle().contains(e->pos());

  SetTimeLineActive(left_timeline_, in_left);
  SetTimeLineActive(right_timeline_, in_right);

  setCursor((in_left || in_right || in_middle) ?
            QCursor(Qt::PointingHandCursor) : QCursor());
}

void PrettyImageView::leaveEvent(QEvent*) {
  SetTimeLineActive(left_timeline_, false);
  SetTimeLineActive(right_timeline_, false);
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

void PrettyImageView::contextMenuEvent(QContextMenuEvent* e) {
  if (!middle().contains(e->pos()))
    return;
  if (current_index_ < 0 || current_index_ >= images_.count())
    return;

  if (!menu_) {
    menu_ = new QMenu(this);
    menu_->addAction(IconLoader::Load("zoom-in"), tr("Show fullsize..."),
                     this, SLOT(ShowFullsize()));
    menu_->addAction(IconLoader::Load("document-save"), tr("Save image") + "...",
                     this, SLOT(SaveAs()));
  }

  menu_->popup(e->globalPos());
}

void PrettyImageView::ShowFullsize() {
  if (current_index_ < 0 || current_index_ >= images_.count())
    return;

  const QImage& image = images_[current_index_].image_;

  QScrollArea* window = new QScrollArea;

  QLabel* label = new QLabel(window);
  label->setPixmap(QPixmap::fromImage(image));
  window->setWidget(label);

  window->setAttribute(Qt::WA_DeleteOnClose, true);
  window->setWindowTitle(tr("Clementine image viewer"));
  window->resize(qMin(800, image.width() + 2), qMin(500, image.height() + 2));
  window->show();
}

void PrettyImageView::SaveAs() {
  if (current_index_ < 0 || current_index_ >= images_.count())
    return;

  const Image& image_data = images_[current_index_];
  QString filename = QFileInfo(image_data.url_.path()).fileName();
  QImage image = image_data.image_;

  if (filename.isEmpty())
    filename = "artwork.jpg";

  QString path = last_save_dir_.isEmpty() ? QDir::homePath() : last_save_dir_;
  QFileInfo path_info(path);
  if (path_info.isDir()) {
    path += "/" + filename;
  } else {
    path = path_info.path() + "/" + filename;
  }

  filename = QFileDialog::getSaveFileName(this, tr("Save image"), path);
  if (filename.isEmpty())
    return;

  image.save(filename);

  last_save_dir_ = filename;
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("last_save_dir", last_save_dir_);
}
