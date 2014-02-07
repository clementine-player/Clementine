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

#ifndef ADDPODCASTPAGE_H
#define ADDPODCASTPAGE_H

#include <QWidget>

class Application;
class PodcastDiscoveryModel;

class AddPodcastPage : public QWidget {
  Q_OBJECT

 public:
  AddPodcastPage(Application* app, QWidget* parent = 0);

  PodcastDiscoveryModel* model() const { return model_; }

  virtual bool has_visible_widget() const { return true; }
  virtual void Show() {}

signals:
  void Busy(bool busy);

 protected:
  void SetModel(PodcastDiscoveryModel* model);

 private:
  PodcastDiscoveryModel* model_;
};

#endif  // ADDPODCASTPAGE_H
