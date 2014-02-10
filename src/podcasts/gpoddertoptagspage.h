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

#ifndef PODCASTS_GPODDERTOPTAGSPAGE_H_
#define PODCASTS_GPODDERTOPTAGSPAGE_H_

#include <QScopedPointer>

#include <ApiRequest.h>

#include "addpodcastpage.h"

class QNetworkAccessManager;

class GPodderTopTagsPage : public AddPodcastPage {
  Q_OBJECT

 public:
  GPodderTopTagsPage(Application* app, QWidget* parent = nullptr);
  ~GPodderTopTagsPage();

  static const int kMaxTagCount;

  virtual bool has_visible_widget() const { return false; }
  virtual void Show();

 private slots:
  void TagListLoaded(mygpo::TagListPtr tag_list);
  void TagListFailed(mygpo::TagListPtr tag_list);

 private:
  QNetworkAccessManager* network_;
  mygpo::ApiRequest* api_;

  bool done_initial_load_;
};

#endif  // PODCASTS_GPODDERTOPTAGSPAGE_H_
