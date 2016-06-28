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

#include "prettyimage.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFuture>
#include <QLabel>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>
#include <QScrollArea>
#include <QSettings>
#include <QtConcurrentRun>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "ui/iconloader.h"

const int PrettyImage::kTotalHeight = 200;
const int PrettyImage::kReflectionHeight = 40;
const int PrettyImage::kImageHeight =
    PrettyImage::kTotalHeight - PrettyImage::kReflectionHeight;

const int PrettyImage::kMaxImageWidth = 300;

const char* PrettyImage::kSettingsGroup = "PrettyImageView";

PrettyImage::PrettyImage(const QUrl& url, QNetworkAccessManager* network,
                         QWidget* parent)
    : QWidget(parent),
      network_(network),
      state_(State_WaitingForLazyLoad),
      url_(url),
      menu_(nullptr) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  LazyLoad();
}

void PrettyImage::LazyLoad() {
  if (state_ != State_WaitingForLazyLoad) return;

  // Start fetching the image
  QNetworkReply* reply = network_->get(QNetworkRequest(url_));
  RedirectFollower* follower = new RedirectFollower(reply);
  state_ = State_Fetching;
  NewClosure(follower, SIGNAL(finished()), this,
             SLOT(ImageFetched(RedirectFollower*)), follower);
}

QSize PrettyImage::image_size() const {
  if (state_ != State_Finished) return QSize(kImageHeight * 1.6, kImageHeight);

  QSize ret = image_.size();
  ret.scale(kMaxImageWidth, kImageHeight, Qt::KeepAspectRatio);
  return ret;
}

QSize PrettyImage::sizeHint() const {
  return QSize(image_size().width(), kTotalHeight);
}

void PrettyImage::ImageFetched(RedirectFollower* follower) {
  follower->deleteLater();
  QNetworkReply* reply = follower->reply();
  reply->deleteLater();

  QImage image = QImage::fromData(reply->readAll());
  if (image.isNull()) {
    qLog(Debug) << "Image failed to load" << reply->request().url()
                << reply->error();
    deleteLater();
  } else {
    state_ = State_CreatingThumbnail;
    image_ = image;

    QFuture<QImage> future =
        QtConcurrent::run(image_, &QImage::scaled, image_size(),
                          Qt::KeepAspectRatio, Qt::SmoothTransformation);
    NewClosure(future, this, SLOT(ImageScaled(QFuture<QImage>)), future);
  }
}

void PrettyImage::ImageScaled(QFuture<QImage> future) {
  thumbnail_ = QPixmap::fromImage(future.result());
  state_ = State_Finished;

  updateGeometry();
  update();
  emit Loaded();
}

void PrettyImage::paintEvent(QPaintEvent*) {
  // Draw at the bottom of our area
  QRect image_rect(QPoint(0, 0), image_size());
  image_rect.moveBottom(kImageHeight);

  QPainter p(this);

  // Draw the main image
  DrawThumbnail(&p, image_rect);

  // Draw the reflection
  // Figure out where to draw it
  QRect reflection_rect(image_rect);
  reflection_rect.moveTop(image_rect.bottom());

  // Create the reflected pixmap
  QImage reflection(reflection_rect.size(),
                    QImage::Format_ARGB32_Premultiplied);
  reflection.fill(palette().color(QPalette::Base).rgba());
  QPainter reflection_painter(&reflection);

  // Set up the transformation
  QTransform transform;
  transform.scale(1.0, -1.0);
  transform.translate(0.0, -reflection_rect.height());
  reflection_painter.setTransform(transform);

  QRect fade_rect(reflection.rect().bottomLeft() - QPoint(0, kReflectionHeight),
                  reflection.rect().bottomRight());

  // Draw the reflection into the buffer
  DrawThumbnail(&reflection_painter, reflection.rect());

  // Make it fade out towards the bottom
  QLinearGradient fade_gradient(fade_rect.topLeft(), fade_rect.bottomLeft());
  fade_gradient.setColorAt(0.0, QColor(0, 0, 0, 0));
  fade_gradient.setColorAt(1.0, QColor(0, 0, 0, 128));

  reflection_painter.setCompositionMode(
      QPainter::CompositionMode_DestinationIn);
  reflection_painter.fillRect(fade_rect, fade_gradient);

  reflection_painter.end();

  // Draw the reflection on the image
  p.drawImage(reflection_rect, reflection);
}

void PrettyImage::DrawThumbnail(QPainter* p, const QRect& rect) {
  switch (state_) {
    case State_WaitingForLazyLoad:
    case State_Fetching:
    case State_CreatingThumbnail:
      p->setPen(palette().color(QPalette::Disabled, QPalette::Text));
      p->drawText(rect, Qt::AlignHCenter | Qt::AlignBottom, tr("Loading..."));
      break;

    case State_Finished:
      p->drawPixmap(rect, thumbnail_);
      break;
  }
}

void PrettyImage::contextMenuEvent(QContextMenuEvent* e) {
  if (e->pos().y() >= kImageHeight) return;

  if (!menu_) {
    menu_ = new QMenu(this);
    menu_->addAction(IconLoader::Load("zoom-in", IconLoader::Base),
                     tr("Show fullsize..."), this, SLOT(ShowFullsize()));
    menu_->addAction(IconLoader::Load("document-save", IconLoader::Base),
                     tr("Save image") + "...", this, SLOT(SaveAs()));
  }

  menu_->popup(e->globalPos());
}

void PrettyImage::ShowFullsize() {
  // Work out how large to make the window, based on the size of the screen
  QRect desktop_rect(QApplication::desktop()->availableGeometry(this));
  QSize window_size(qMin(desktop_rect.width() - 20, image_.width()),
                    qMin(desktop_rect.height() - 20, image_.height()));

  // Create the window
  QScrollArea* window = new QScrollArea;
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  window->setWindowTitle(tr("Clementine image viewer"));
  window->resize(window_size);

  // Create the label that displays the image
  QLabel* label = new QLabel(window);
  label->setPixmap(QPixmap::fromImage(image_));

  // Show the label in the window
  window->setWidget(label);
  window->setFrameShape(QFrame::NoFrame);
  window->show();
}

void PrettyImage::SaveAs() {
  QString filename = QFileInfo(url_.path()).fileName();

  if (filename.isEmpty()) filename = "artwork.jpg";

  QSettings s;
  s.beginGroup(kSettingsGroup);
  QString last_save_dir = s.value("last_save_dir", QDir::homePath()).toString();

  QString path = last_save_dir.isEmpty() ? QDir::homePath() : last_save_dir;
  QFileInfo path_info(path);
  if (path_info.isDir()) {
    path += "/" + filename;
  } else {
    path = path_info.path() + "/" + filename;
  }

  filename = QFileDialog::getSaveFileName(this, tr("Save image"), path);
  if (filename.isEmpty()) return;

  image_.save(filename);

  s.setValue("last_save_dir", last_save_dir);
}
