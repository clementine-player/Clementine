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

#include "icecastbackend.h"
#include "icecastmodel.h"

IcecastModel::IcecastModel(IcecastBackend* backend, QObject* parent)
  : SimpleTreeModel<IcecastItem>(new IcecastItem(this), parent),
    backend_(backend),
    sort_mode_(SortMode_GenreByPopularity),
    genre_icon_(":last.fm/icon_tag.png"),
    station_icon_(":last.fm/icon_radio.png")
{
  root_->lazy_loaded = true;
}

IcecastModel::~IcecastModel() {
  delete root_;
}

void IcecastModel::Init() {
  connect(backend_, SIGNAL(DatabaseReset()), SLOT(Reset()));

  Reset();
}

void IcecastModel::Reset() {
  delete root_;

  root_ = new IcecastItem(this);
  root_->lazy_loaded = false;

  LazyPopulate(root_);

  reset();
}

void IcecastModel::LazyPopulate(IcecastItem* parent) {
  if (parent->lazy_loaded)
    return;
  parent->lazy_loaded = true;

  switch (parent->type) {
    case IcecastItem::Type_Station:
      return;

    case IcecastItem::Type_Genre:
      PopulateGenre(parent, parent->key);
      break;

    case IcecastItem::Type_Root:
      switch (sort_mode_) {
        case SortMode_GenreAlphabetical:
          AddGenres(backend_->GetGenresAlphabetical(filter_));
          break;

        case SortMode_GenreByPopularity:
          AddGenres(backend_->GetGenresByPopularity(filter_));
          break;

        case SortMode_StationAlphabetical:
          PopulateGenre(parent, QString());
          break;
      }
      break;
  }
}

void IcecastModel::PopulateGenre(IcecastItem* parent, const QString& genre) {
  IcecastBackend::StationList stations = backend_->GetStations(filter_, genre);
  foreach (const IcecastBackend::Station& station, stations) {
    IcecastItem* item = new IcecastItem(IcecastItem::Type_Station, parent);
    item->display_text = station.name;
    item->sort_text = station.name;
    item->key = station.url.toString();
    item->lazy_loaded = true;
  }
}

void IcecastModel::AddGenres(const QStringList& genres) {
  foreach (const QString& genre, genres) {
    IcecastItem* item = new IcecastItem(IcecastItem::Type_Genre, root_);
    item->key = genre;
  }
}

QVariant IcecastModel::data(const QModelIndex& index, int role) const {
  const IcecastItem* item = IndexToItem(index);
  return data(item, role);
}

QVariant IcecastModel::data(const IcecastItem* item, int role) const {
  switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
      return item->DisplayText();

    case Qt::DecorationRole:
      switch (item->type) {
        case IcecastItem::Type_Genre:   return genre_icon_;
        case IcecastItem::Type_Station: return station_icon_;
      }
      break;
  }
  return QVariant();
}

void IcecastModel::SetFilterText(const QString& filter) {
  filter_ = filter;
  Reset();
}

void IcecastModel::SetSortMode(SortMode mode) {
  sort_mode_ = mode;
  Reset();
}
