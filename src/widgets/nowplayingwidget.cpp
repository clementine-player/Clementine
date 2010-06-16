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

#include "nowplayingwidget.h"
#include "core/albumcoverloader.h"
#include "core/networkaccessmanager.h"

#include <QPainter>
#include <QPaintEvent>
#include <QTextDocument>
#include <QTimeLine>
#include <QtDebug>

// Space between the cover and the details
const int NowPlayingWidget::kPadding = 4;

NowPlayingWidget::NowPlayingWidget(QWidget *parent)
  : QWidget(parent),
    cover_loader_(new BackgroundThreadImplementation<AlbumCoverLoader, AlbumCoverLoader>(this)),
    network_(NULL),
    visible_(false),
    ideal_height_(0),
    show_hide_animation_(new QTimeLine(500, this)),
    fade_animation_(new QTimeLine(1000, this)),
    no_cover_(":nocover.png"),
    load_cover_id_(0),
    details_(new QTextDocument(this)),
    previous_track_opacity_(0.0)
{
  connect(show_hide_animation_, SIGNAL(frameChanged(int)), SLOT(SetHeight(int)));
  setMaximumHeight(0);

  connect(fade_animation_, SIGNAL(valueChanged(qreal)), SLOT(FadePreviousTrack(qreal)));
  fade_animation_->setDirection(QTimeLine::Backward); // 1.0 -> 0.0

  cover_loader_->Start();
  connect(cover_loader_, SIGNAL(Initialised()), SLOT(CoverLoaderInitialised()));
}

void NowPlayingWidget::set_ideal_height(int height) {
  ideal_height_= height;
  show_hide_animation_->setFrameRange(0, ideal_height_);
}

QSize NowPlayingWidget::sizeHint() const {
  return QSize(ideal_height_, ideal_height_);
}

void NowPlayingWidget::CoverLoaderInitialised() {
  cover_loader_->Worker()->SetNetwork(network_);
  cover_loader_->Worker()->SetDesiredHeight(ideal_height_);
  cover_loader_->Worker()->SetPadOutputImage(true);
  cover_loader_->Worker()->SetDefaultOutputImage(QImage(":nocover.png"));
  connect(cover_loader_->Worker().get(), SIGNAL(ImageLoaded(quint64,QImage)),
          SLOT(AlbumArtLoaded(quint64,QImage)));
}

void NowPlayingWidget::NowPlaying(const Song& metadata) {
  if (visible_) {
    // Cache the current pixmap so we can fade between them
    previous_track_ = QPixmap(size());
    previous_track_.fill(palette().background().color());
    previous_track_opacity_ = 1.0;
    QPainter p(&previous_track_);
    DrawContents(&p);
    p.end();
  }

  metadata_ = metadata;

  // Load the cover
  cover_ = QPixmap();
  load_cover_id_ = cover_loader_->Worker()->LoadImageAsync(
      metadata.art_automatic(), metadata.art_manual());

  // TODO: Make this configurable
  details_->setHtml(QString("<i>%1</i><br/>%2<br/>%3").arg(
      Qt::escape(metadata.title()), Qt::escape(metadata.artist()),
      Qt::escape(metadata.album())));

  SetVisible(true);
  update();
}

void NowPlayingWidget::Stopped() {
  SetVisible(false);
}

void NowPlayingWidget::AlbumArtLoaded(quint64 id, const QImage& image) {
  if (id != load_cover_id_)
    return;

  cover_ = QPixmap::fromImage(image);
  update();

  // Were we waiting for this cover to load before we started fading?
  if (!previous_track_.isNull()) {
    fade_animation_->start();
  }
}

void NowPlayingWidget::SetHeight(int height) {
  setMaximumHeight(height);
}

void NowPlayingWidget::SetVisible(bool visible) {
  if (visible == visible_)
    return;
  visible_ = visible;

  show_hide_animation_->setDirection(visible ? QTimeLine::Forward : QTimeLine::Backward);
  show_hide_animation_->start();
}

void NowPlayingWidget::paintEvent(QPaintEvent *e) {
  QPainter p(this);

  DrawContents(&p);

  // Draw the previous track's image if we're fading
  if (!previous_track_.isNull()) {
    p.setOpacity(previous_track_opacity_);
    p.drawPixmap(0, 0, previous_track_);
  }
}

void NowPlayingWidget::DrawContents(QPainter *p) {
  // Draw the cover
  p->drawPixmap(0, 0, ideal_height_, ideal_height_, cover_);

  // Draw the details
  p->translate(ideal_height_ + kPadding, 0);
  details_->drawContents(p);
  p->translate(-ideal_height_ - kPadding, 0);
}

void NowPlayingWidget::FadePreviousTrack(qreal value) {
  previous_track_opacity_ = value;
  if (qFuzzyCompare(previous_track_opacity_, 0.0)) {
    previous_track_ = QPixmap();
  }

  update();
}
