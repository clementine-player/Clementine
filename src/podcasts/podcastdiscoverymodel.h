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

#ifndef PODCASTDISCOVERYMODEL_H
#define PODCASTDISCOVERYMODEL_H

#include "covers/albumcoverloaderoptions.h"

#include <QStandardItemModel>

class Application;
class OpmlContainer;
class OpmlFeed;
class Podcast;
class StandardItemIconLoader;

class PodcastDiscoveryModel : public QStandardItemModel {
  Q_OBJECT

 public:
  PodcastDiscoveryModel(Application* app, QObject* parent = 0);

  enum Type { Type_Folder, Type_Podcast, Type_LoadingIndicator };

  enum Role {
    Role_Podcast = Qt::UserRole,
    Role_Type,
    Role_ImageUrl,
    Role_StartedLoadingImage,
    RoleCount
  };

  void CreateOpmlContainerItems(const OpmlContainer& container,
                                QStandardItem* parent);
  QStandardItem* CreateOpmlContainerItem(const OpmlContainer& container);
  QStandardItem* CreatePodcastItem(const Podcast& podcast);
  QStandardItem* CreateFolder(const QString& name);
  QStandardItem* CreateLoadingIndicator();

  QVariant data(const QModelIndex& index, int role) const;

 private:
  void LazyLoadImage(const QUrl& url, const QModelIndex& index);

 private:
  Application* app_;
  StandardItemIconLoader* icon_loader_;

  QIcon default_icon_;
  QIcon folder_icon_;
};

#endif  // PODCASTDISCOVERYMODEL_H
