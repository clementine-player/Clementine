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

#ifndef PODCASTSERVICE_H
#define PODCASTSERVICE_H

#include "internet/internetservice.h"

#include <QScopedPointer>

class AddPodcastDialog;


class PodcastService : public InternetService {
  Q_OBJECT

public:
  PodcastService(Application* app, InternetModel* parent);
  ~PodcastService();

  static const char* kServiceName;
  static const char* kSettingsGroup;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);

  void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos);

protected:
  QModelIndex GetCurrentIndex();

private slots:
  void AddPodcast();

private:
  QMenu* context_menu_;
  QStandardItem* root_;

  QScopedPointer<AddPodcastDialog> add_podcast_dialog_;
};

#endif // PODCASTSERVICE_H
