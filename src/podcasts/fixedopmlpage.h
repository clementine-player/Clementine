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

#ifndef PODCASTS_FIXEDOPMLPAGE_H_
#define PODCASTS_FIXEDOPMLPAGE_H_

#include "addpodcastpage.h"

#include <QUrl>

class PodcastUrlLoader;
class PodcastUrlLoaderReply;

class FixedOpmlPage : public AddPodcastPage {
  Q_OBJECT

 public:
  FixedOpmlPage(const QUrl& opml_url, const QString& title, const QIcon& icon,
                Application* app, QWidget* parent = nullptr);

  bool has_visible_widget() const { return false; }
  void Show();

 private slots:
  void LoadFinished(PodcastUrlLoaderReply* reply);

 private:
  PodcastUrlLoader* loader_;
  QUrl opml_url_;

  bool done_initial_load_;
};

#endif  // PODCASTS_FIXEDOPMLPAGE_H_
