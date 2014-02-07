/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef PODCASTINFOWIDGET_H
#define PODCASTINFOWIDGET_H

#include "podcast.h"
#include "covers/albumcoverloaderoptions.h"

#include <QFrame>

class Application;
class Ui_PodcastInfoWidget;

class QLabel;

class PodcastInfoWidget : public QWidget {
  Q_OBJECT

 public:
  PodcastInfoWidget(QWidget* parent = 0);
  ~PodcastInfoWidget();

  void SetApplication(Application* app);

  void SetPodcast(const Podcast& podcast);

signals:
  void LoadingFinished();

 private slots:
  void ImageLoaded(quint64 id, const QImage& image);

 private:
  Ui_PodcastInfoWidget* ui_;

  AlbumCoverLoaderOptions cover_options_;

  Application* app_;
  Podcast podcast_;
  quint64 image_id_;
};

#endif  // PODCASTINFOWIDGET_H
