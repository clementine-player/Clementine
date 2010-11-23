/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef ICECASTMODEL_H
#define ICECASTMODEL_H

#include "icecastitem.h"
#include "core/simpletreemodel.h"

#include <QIcon>

class IcecastBackend;

class IcecastModel : public SimpleTreeModel<IcecastItem> {
  Q_OBJECT

public:
  IcecastModel(IcecastBackend* backend, QObject* parent = 0);
  ~IcecastModel();

  // These values get saved in QSettings - don't change them
  enum SortMode {
    SortMode_GenreAlphabetical = 0,
    SortMode_GenreByPopularity = 1,
    SortMode_StationAlphabetical = 2,
  };

  IcecastBackend* backend() const { return backend_; }

  // QAbstractItemModel
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  /*Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;
  bool canFetchMore(const QModelIndex &parent) const;*/

public slots:
  void Init();
  void Reset();

protected:
  void LazyPopulate(IcecastItem* parent);

private:
  QVariant data(const IcecastItem* item, int role) const;
  void PopulateGenre(IcecastItem* parent, const QString& genre);
  void AddGenres(const QStringList& genres);

private:
  IcecastBackend* backend_;

  SortMode sort_mode_;

  QIcon genre_icon_;
  QIcon station_icon_;
};

#endif // ICECASTMODEL_H
