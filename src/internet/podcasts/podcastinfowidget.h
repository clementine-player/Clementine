/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef INTERNET_PODCASTS_PODCASTINFOWIDGET_H_
#define INTERNET_PODCASTS_PODCASTINFOWIDGET_H_

#include <QFrame>

#include "covers/albumcoverloaderoptions.h"
#include "podcast.h"

class Application;
class Ui_PodcastInfoWidget;

class QLabel;

class PodcastInfoWidget : public QWidget {
  Q_OBJECT

 public:
  explicit PodcastInfoWidget(QWidget* parent = nullptr);
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

#endif  // INTERNET_PODCASTS_PODCASTINFOWIDGET_H_
