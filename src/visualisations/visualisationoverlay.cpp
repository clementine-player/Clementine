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

#include "visualisationoverlay.h"

#include <QGraphicsProxyWidget>
#include <QMouseEvent>
#include <QTimeLine>
#include <QTimerEvent>
#include <QtDebug>

#include "ui/iconloader.h"
#include "ui_visualisationoverlay.h"

const int VisualisationOverlay::kFadeDuration = 500;
const int VisualisationOverlay::kFadeTimeout = 5000;

VisualisationOverlay::VisualisationOverlay(QWidget* parent)
    : QWidget(parent),
      ui_(new Ui_VisualisationOverlay),
      fade_timeline_(new QTimeLine(kFadeDuration, this)),
      visible_(false) {
  ui_->setupUi(this);

  setAttribute(Qt::WA_TranslucentBackground);
  setMouseTracking(true);

  ui_->settings->setIcon(IconLoader::Load("configure", IconLoader::Base));
  connect(ui_->settings, SIGNAL(clicked()), SLOT(ShowSettingsMenu()));

  connect(fade_timeline_, SIGNAL(valueChanged(qreal)),
          SIGNAL(OpacityChanged(qreal)));
}

VisualisationOverlay::~VisualisationOverlay() { delete ui_; }

QGraphicsProxyWidget* VisualisationOverlay::title(
    QGraphicsProxyWidget* proxy) const {
  return proxy->createProxyForChildWidget(ui_->song_title);
}

void VisualisationOverlay::SetActions(QAction* previous, QAction* play_pause,
                                      QAction* stop, QAction* next) {
  ui_->previous->setDefaultAction(previous);
  ui_->play_pause->setDefaultAction(play_pause);
  ui_->stop->setDefaultAction(stop);
  ui_->next->setDefaultAction(next);
}

void VisualisationOverlay::ShowSettingsMenu() {
  emit ShowPopupMenu(
      ui_->settings->mapToGlobal(ui_->settings->rect().bottomLeft()));
}

void VisualisationOverlay::timerEvent(QTimerEvent* e) {
  QWidget::timerEvent(e);

  if (e->timerId() == fade_out_timeout_.timerId()) {
    SetVisible(false);
  }
}

void VisualisationOverlay::SetVisible(bool visible) {
  // If we're showing the overlay, then fade out again in a little while
  fade_out_timeout_.stop();
  if (visible) fade_out_timeout_.start(kFadeTimeout, this);

  // Don't change to the state we're in already
  if (visible == visible_) return;
  visible_ = visible;

  // If there's already another fader running then start from the same time
  // that one was already at.
  int start_time = visible ? 0 : fade_timeline_->duration();
  if (fade_timeline_->state() == QTimeLine::Running)
    start_time = fade_timeline_->currentTime();

  fade_timeline_->stop();
  fade_timeline_->setDirection(visible ? QTimeLine::Forward
                                       : QTimeLine::Backward);
  fade_timeline_->setCurrentTime(start_time);
  fade_timeline_->resume();
}

void VisualisationOverlay::SetSongTitle(const QString& title) {
  ui_->song_title->setText(title);
  SetVisible(true);
}
