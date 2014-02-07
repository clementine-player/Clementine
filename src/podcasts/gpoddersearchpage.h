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

#ifndef PODCASTS_GPODDERSEARCHPAGE_H_
#define PODCASTS_GPODDERSEARCHPAGE_H_

#include "addpodcastpage.h"

#include <ApiRequest.h>

class QNetworkAccessManager;

class Ui_GPodderSearchPage;

class GPodderSearchPage : public AddPodcastPage {
  Q_OBJECT

 public:
  GPodderSearchPage(Application* app, QWidget* parent = 0);
  ~GPodderSearchPage();

  void Show();

 private slots:
  void SearchClicked();
  void SearchFinished(mygpo::PodcastListPtr list);
  void SearchFailed(mygpo::PodcastListPtr list);

 private:
  Ui_GPodderSearchPage* ui_;

  QNetworkAccessManager* network_;
  mygpo::ApiRequest* api_;
};

#endif  // PODCASTS_GPODDERSEARCHPAGE_H_
