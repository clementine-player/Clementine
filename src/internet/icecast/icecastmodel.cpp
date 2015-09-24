/* This file is part of Clementine.
   Copyright 2010, David Sansome <davidsansome@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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
#include "playlist/songmimedata.h"
#include "ui/iconloader.h"

IcecastModel::IcecastModel(IcecastBackend* backend, QObject* parent)
    : SimpleTreeModel<IcecastItem>(new IcecastItem(this), parent),
      backend_(backend),
      sort_mode_(SortMode_GenreByPopularity),
      genre_icon_(IconLoader::Load("icon_tag", IconLoader::Lastfm)),
      station_icon_(IconLoader::Load("icon_radio", IconLoader::Lastfm)) {
  root_->lazy_loaded = true;
}

IcecastModel::~IcecastModel() { delete root_; }

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
  if (parent->lazy_loaded) return;
  parent->lazy_loaded = true;

  switch (parent->type) {
    case IcecastItem::Type_Station:
      return;

    case IcecastItem::Type_Genre:
      PopulateGenre(parent, parent->key, false);
      break;

    case IcecastItem::Type_Root:
      switch (sort_mode_) {
        case SortMode_GenreAlphabetical:
          AddGenres(backend_->GetGenresAlphabetical(filter_), true);
          break;

        case SortMode_GenreByPopularity:
          AddGenres(backend_->GetGenresByPopularity(filter_), false);
          break;

        case SortMode_StationAlphabetical:
          PopulateGenre(parent, QString(), true);
          break;
      }
      break;
  }
}

void IcecastModel::PopulateGenre(IcecastItem* parent, const QString& genre,
                                 bool create_dividers) {
  QChar last_divider;

  IcecastBackend::StationList stations = backend_->GetStations(filter_, genre);
  for (const IcecastBackend::Station& station : stations) {
    QChar divider_char = DividerKey(station.name);
    if (create_dividers && !divider_char.isNull() &&
        divider_char != last_divider) {
      last_divider = divider_char;

      IcecastItem* divider = new IcecastItem(IcecastItem::Type_Divider, parent);
      divider->display_text = DividerDisplayText(divider_char);
      divider->lazy_loaded = true;
    }

    IcecastItem* item = new IcecastItem(IcecastItem::Type_Station, parent);
    item->station = station;
    item->display_text = station.name;
    item->sort_text = station.name;
    item->key = station.url.toString();
    item->lazy_loaded = true;
  }
}

void IcecastModel::AddGenres(const QStringList& genres, bool create_dividers) {
  QChar last_divider;

  for (const QString& genre : genres) {
    QChar divider_char = DividerKey(genre);
    if (create_dividers && divider_char != last_divider) {
      last_divider = divider_char;

      IcecastItem* divider = new IcecastItem(IcecastItem::Type_Divider, root_);
      divider->display_text = DividerDisplayText(divider_char);
      divider->lazy_loaded = true;
    }

    IcecastItem* item = new IcecastItem(IcecastItem::Type_Genre, root_);
    item->key = genre;
  }
}

QChar IcecastModel::DividerKey(const QString& text) {
  if (text.isEmpty()) return QChar();

  QChar c;
  c = text[0];

  if (c.isDigit()) return '0';
  if (c.isPunct() || c.isSymbol()) return QChar();

  if (c.decompositionTag() != QChar::NoDecomposition)
    return QChar(c.decomposition()[0]);
  return c.toUpper();
}

QString IcecastModel::DividerDisplayText(const QChar& key) {
  if (key == '0') return "0-9";
  return key;
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
        case IcecastItem::Type_Genre:
          return genre_icon_;
        case IcecastItem::Type_Station:
          return station_icon_;
      }
      break;

    case Role_IsDivider:
      return item->type == IcecastItem::Type_Divider;
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

Qt::ItemFlags IcecastModel::flags(const QModelIndex& index) const {
  switch (IndexToItem(index)->type) {
    case IcecastItem::Type_Station:
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
    case IcecastItem::Type_Genre:
    case IcecastItem::Type_Root:
    case IcecastItem::Type_Divider:
    default:
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }
}

QStringList IcecastModel::mimeTypes() const {
  return QStringList() << "text/uri-list";
}

QMimeData* IcecastModel::mimeData(const QModelIndexList& indexes) const {
  if (indexes.isEmpty()) return nullptr;

  SongMimeData* data = new SongMimeData;
  QList<QUrl> urls;

  for (const QModelIndex& index : indexes) {
    IcecastItem* item = IndexToItem(index);
    if (!item || item->type != IcecastItem::Type_Station) continue;

    data->songs << item->station.ToSong();
    urls << item->station.url;
  }

  if (data->songs.isEmpty()) {
    delete data;
    return nullptr;
  }

  data->setUrls(urls);
  data->name_for_new_playlist_ = "Icecast";

  return data;
}

Song IcecastModel::GetSong(const QModelIndex& index) const {
  IcecastItem* item = IndexToItem(index);
  if (!item || item->type != IcecastItem::Type_Station) return Song();

  return item->station.ToSong();
}
