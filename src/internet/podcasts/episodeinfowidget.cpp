/* This file is part of Clementine.
   Copyright 2018, Jim Broadus <jbroadus@gmail.com>

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

#include "core/utilities.h"
#include "episodeinfowidget.h"
#include "ui_episodeinfowidget.h"

#include <QTime>

EpisodeInfoWidget::EpisodeInfoWidget(QWidget* parent)
    : QWidget(parent), ui_(new Ui_EpisodeInfoWidget), app_(nullptr) {
  ui_->setupUi(this);
}

EpisodeInfoWidget::~EpisodeInfoWidget() { delete ui_; }

void EpisodeInfoWidget::SetApplication(Application* app) { app_ = app; }

void EpisodeInfoWidget::SetEpisode(const PodcastEpisode& episode) {
  episode_ = episode;
  ui_->title->setText(episode.title());
  ui_->description->setText(episode.description());
  ui_->author->setText(episode.author());
  ui_->date->setText(episode.publication_date().toString("d MMMM yyyy"));
  ui_->duration->setText(Utilities::PrettyTime(episode.duration_secs(), true));
}
