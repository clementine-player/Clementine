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

#ifndef GPODDERSEARCHPAGE_H
#define GPODDERSEARCHPAGE_H

#include "addpodcastpage.h"

class QNetworkAccessManager;

class Ui_GPodderSearchPage;

namespace mygpo {
  class ApiRequest;
  class PodcastList;
}

class GPodderSearchPage : public AddPodcastPage {
  Q_OBJECT

public:
  GPodderSearchPage(Application* app, QWidget* parent = 0);
  ~GPodderSearchPage();

  void Show();

private slots:
  void SearchClicked();
  void SearchFinished(mygpo::PodcastList* list);
  void SearchFailed(mygpo::PodcastList* list);

private:
  Ui_GPodderSearchPage* ui_;

  QNetworkAccessManager* network_;
  mygpo::ApiRequest* api_;

};

#endif // GPODDERSEARCHPAGE_H
