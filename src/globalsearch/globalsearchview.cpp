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

#include "globalsearch.h"
#include "globalsearchitemdelegate.h"
#include "globalsearchsortmodel.h"
#include "globalsearchview.h"
#include "searchprovider.h"
#include "ui_globalsearchview.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/mimedata.h"
#include "library/librarymodel.h"

#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QTimer>

const int GlobalSearchView::kSwapModelsTimeoutMsec = 250;

GlobalSearchView::GlobalSearchView(Application* app, QWidget* parent)
  : QWidget(parent),
    app_(app),
    engine_(app_->global_search()),
    ui_(new Ui_GlobalSearchView),
    last_search_id_(0),
    front_model_(new QStandardItemModel(this)),
    back_model_(new QStandardItemModel(this)),
    current_model_(front_model_),
    front_proxy_(new GlobalSearchSortModel(this)),
    back_proxy_(new GlobalSearchSortModel(this)),
    current_proxy_(front_proxy_),
    swap_models_timer_(new QTimer(this)),
    artist_icon_(":/icons/22x22/x-clementine-artist.png"),
    album_icon_(":/icons/22x22/x-clementine-album.png")
{
  ui_->setupUi(this);

  connect(ui_->search, SIGNAL(textChanged(QString)), SLOT(TextEdited(QString)));

  ui_->results->setItemDelegate(new GlobalSearchItemDelegate(this));

  group_by_[0] = LibraryModel::GroupBy_Artist;
  group_by_[1] = LibraryModel::GroupBy_Album;
  group_by_[2] = LibraryModel::GroupBy_None;

  no_cover_icon_ = QPixmap(":nocover.png").scaled(
        LibraryModel::kPrettyCoverSize, LibraryModel::kPrettyCoverSize,
        Qt::KeepAspectRatio, Qt::SmoothTransformation);

  // Set up the sorting proxy model
  front_proxy_->setSourceModel(front_model_);
  front_proxy_->setDynamicSortFilter(true);
  front_proxy_->sort(0);

  back_proxy_->setSourceModel(back_model_);
  back_proxy_->setDynamicSortFilter(true);
  back_proxy_->sort(0);

  swap_models_timer_->setSingleShot(true);
  swap_models_timer_->setInterval(kSwapModelsTimeoutMsec);
  connect(swap_models_timer_, SIGNAL(timeout()), SLOT(SwapModels()));

  // These have to be queued connections because they may get emitted before
  // our call to Search() (or whatever) returns and we add the ID to the map.
  connect(engine_, SIGNAL(ResultsAvailable(int,SearchProvider::ResultList)),
          SLOT(AddResults(int,SearchProvider::ResultList)),
          Qt::QueuedConnection);
  connect(engine_, SIGNAL(ArtLoaded(int,QPixmap)), SLOT(ArtLoaded(int,QPixmap)),
          Qt::QueuedConnection);
  connect(engine_, SIGNAL(TracksLoaded(int,MimeData*)), SLOT(TracksLoaded(int,MimeData*)),
          Qt::QueuedConnection);
}

GlobalSearchView::~GlobalSearchView() {
  delete ui_;
}

void GlobalSearchView::StartSearch(const QString& query) {
  ui_->search->set_text(query);
  TextEdited(query.trimmed());

  // Swap models immediately
  swap_models_timer_->stop();
  SwapModels();
}

void GlobalSearchView::TextEdited(const QString& text) {
  const QString trimmed(text.trimmed());

  // Add results to the back model, switch models after some delay.
  provider_sort_indices_.clear();
  containers_.clear();
  next_provider_sort_index_ = 1000;
  back_model_->clear();
  current_model_ = back_model_;
  current_proxy_ = back_proxy_;
  swap_models_timer_->start();

  // Cancel the last search (if any) and start the new one.
  engine_->CancelSearch(last_search_id_);
  // If text query is empty, don't start a new search
  if (trimmed.isEmpty()) {
    last_search_id_ = -1;
    return;
  }
  last_search_id_ = engine_->SearchAsync(trimmed);
}

void GlobalSearchView::AddResults(int id, const SearchProvider::ResultList& results) {
  if (id != last_search_id_ || results.isEmpty())
    return;

  int sort_index = 0;

  // Create a divider for this provider if we haven't seen it before.
  SearchProvider* provider = results.first().provider_;

  if (!provider_sort_indices_.contains(provider)) {
    // TODO: Check if the user has configured a sort order for this provider.
    sort_index = next_provider_sort_index_ ++;

    QStandardItem* divider = new QStandardItem(provider->icon(), provider->name());
    divider->setData(true, LibraryModel::Role_IsDivider);
    divider->setData(sort_index, Role_ProviderIndex);
    current_model_->appendRow(divider);

    provider_sort_indices_[provider] = sort_index;
  } else {
    sort_index = provider_sort_indices_[provider];
  }

  foreach (const SearchProvider::Result& result, results) {
    QStandardItem* parent = current_model_->invisibleRootItem();

    // Find (or create) the container nodes for this result if we can.
    if (result.group_automatically_) {
      ContainerKey key;
      key.provider_index_ = sort_index;

      parent = BuildContainers(result.metadata_, parent, &key);
    }

    // Create the item
    QStandardItem* item = new QStandardItem(result.metadata_.title());
    item->setData(QVariant::fromValue(result), Role_Result);
    item->setData(sort_index, Role_ProviderIndex);

    parent->appendRow(item);
  }
}

QStandardItem* GlobalSearchView::BuildContainers(
    const Song& s, QStandardItem* parent, ContainerKey* key, int level) {
  if (level >= 3) {
    return parent;
  }

  QIcon icon;
  QString display_text;
  QString sort_text;
  int year = 0;

  switch (group_by_[level]) {
  case LibraryModel::GroupBy_Artist:
    display_text = LibraryModel::TextOrUnknown(s.artist());
    sort_text = LibraryModel::SortTextForArtist(s.artist());
    icon = artist_icon_;
    break;

  case LibraryModel::GroupBy_YearAlbum:
    year = qMax(0, s.year());
    display_text = LibraryModel::PrettyYearAlbum(year, s.album());
    sort_text = LibraryModel::SortTextForYear(year) + s.album();
    icon = album_icon_;
    break;

  case LibraryModel::GroupBy_Year:
    year = qMax(0, s.year());
    display_text = QString::number(year);
    sort_text = LibraryModel::SortTextForYear(year) + " ";
    break;

  case LibraryModel::GroupBy_Composer:                         display_text = s.composer();
  case LibraryModel::GroupBy_Genre: if (display_text.isNull()) display_text = s.genre();
  case LibraryModel::GroupBy_Album: if (display_text.isNull()) display_text = s.album();
  case LibraryModel::GroupBy_AlbumArtist: if (display_text.isNull()) display_text = s.effective_albumartist();
    display_text = LibraryModel::TextOrUnknown(display_text);
    sort_text = LibraryModel::SortTextForArtist(display_text);
    icon = album_icon_;
    break;

  case LibraryModel::GroupBy_FileType:
    display_text = s.TextForFiletype();
    sort_text = display_text;
    break;

  case LibraryModel::GroupBy_None:
    return parent;
  }

  // Find a container for this level
  key->group_[level] = display_text;
  QStandardItem* container = containers_[*key];
  if (!container) {
    container = new QStandardItem(icon, display_text);
    container->setData(key->provider_index_, Role_ProviderIndex);
    container->setData(sort_text, LibraryModel::Role_SortText);
    container->setData(group_by_[level], LibraryModel::Role_ContainerType);

    parent->appendRow(container);
    containers_[*key] = container;
  }

  // Create the container for the next level.
  return BuildContainers(s, container, key, level + 1);
}

void GlobalSearchView::SwapModels() {
  art_requests_.clear();

  qSwap(front_model_, back_model_);
  qSwap(front_proxy_, back_proxy_);

  ui_->results->setModel(front_proxy_);
}

void GlobalSearchView::LazyLoadArt(const QModelIndex& proxy_index) {
  if (!proxy_index.isValid() || proxy_index.data(Role_LazyLoadingArt).isValid()) {
    return;
  }
  if (proxy_index.model() != front_proxy_) {
    return;
  }

  // Only load art for albums
  const LibraryModel::GroupBy container_type = LibraryModel::GroupBy(
        proxy_index.data(LibraryModel::Role_ContainerType).toInt());
  if (container_type != LibraryModel::GroupBy_Album &&
      container_type != LibraryModel::GroupBy_AlbumArtist &&
      container_type != LibraryModel::GroupBy_YearAlbum) {
    return;
  }

  // Mark the item as loading art
  const QModelIndex source_index = front_proxy_->mapToSource(proxy_index);
  QStandardItem* item = front_model_->itemFromIndex(source_index);
  item->setData(true, Role_LazyLoadingArt);

  // Walk down the item's children until we find a track
  while (item->rowCount()) {
    item = item->child(0);
  }

  // Get the track's Result
  const SearchProvider::Result result =
      item->data(Role_Result).value<SearchProvider::Result>();

  // Load the art.
  int id = engine_->LoadArtAsync(result);
  art_requests_[id] = source_index;
}

void GlobalSearchView::ArtLoaded(int id, const QPixmap& pixmap) {
  if (!art_requests_.contains(id))
    return;
  QModelIndex index = art_requests_.take(id);

  front_model_->itemFromIndex(index)->setData(pixmap, Qt::DecorationRole);
}

void GlobalSearchView::LoadTracks() {
  QModelIndex index = ui_->results->currentIndex();
  if (!index.isValid())
    index = front_proxy_->index(0, 0);

  if (!index.isValid())
    return;

  const SearchProvider::Result result =
      index.data(Role_Result).value<SearchProvider::Result>();

  engine_->LoadTracksAsync(result);
}

void GlobalSearchView::TracksLoaded(int id, MimeData* mime_data) {
  if (!mime_data)
    return;

  mime_data->from_doubleclick_ = true;
  emit AddToPlaylist(mime_data);
}
