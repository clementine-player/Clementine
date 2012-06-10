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
#include "globalsearchmodel.h"
#include "globalsearchsortmodel.h"
#include "globalsearchview.h"
#include "searchprovider.h"
#include "searchproviderstatuswidget.h"
#include "ui_globalsearchview.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/mimedata.h"
#include "library/librarymodel.h"

#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QTimer>

const int GlobalSearchView::kSwapModelsTimeoutMsec = 250;

GlobalSearchView::GlobalSearchView(Application* app, QWidget* parent)
  : QWidget(parent),
    app_(app),
    engine_(app_->global_search()),
    ui_(new Ui_GlobalSearchView),
    context_menu_(NULL),
    last_search_id_(0),
    front_model_(new GlobalSearchModel(engine_, this)),
    back_model_(new GlobalSearchModel(engine_, this)),
    current_model_(front_model_),
    front_proxy_(new GlobalSearchSortModel(this)),
    back_proxy_(new GlobalSearchSortModel(this)),
    current_proxy_(front_proxy_),
    swap_models_timer_(new QTimer(this))
{
  ui_->setupUi(this);

  ui_->search->installEventFilter(this);
  ui_->results->installEventFilter(this);

  // Must be a queued connection to ensure the GlobalSearch handles it first.
  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()), Qt::QueuedConnection);

  connect(ui_->search, SIGNAL(textChanged(QString)), SLOT(TextEdited(QString)));
  connect(ui_->results, SIGNAL(AddToPlaylistSignal(QMimeData*)), SIGNAL(AddToPlaylist(QMimeData*)));

  // Set the appearance of the results list
  ui_->results->setItemDelegate(new GlobalSearchItemDelegate(this));
  ui_->results->setAttribute(Qt::WA_MacShowFocusRect, false);
  ui_->results->setStyleSheet("QTreeView::item{padding-top:1px;}");

  // Show the help page initially
  ui_->stack->setCurrentWidget(ui_->help_page);
  ui_->help_frame->setBackgroundRole(QPalette::Base);
  QVBoxLayout* enabled_layout = new QVBoxLayout(ui_->enabled_list);
  QVBoxLayout* disabled_layout = new QVBoxLayout(ui_->disabled_list);
  enabled_layout->setContentsMargins(16, 0, 16, 6);
  disabled_layout->setContentsMargins(16, 0, 16, 6);

  // Set the colour of the help text to the disabled text colour
  QPalette help_palette = ui_->help_text->palette();
  const QColor help_color = help_palette.color(QPalette::Disabled, QPalette::Text);
  help_palette.setColor(QPalette::Normal, QPalette::Text, help_color);
  help_palette.setColor(QPalette::Inactive, QPalette::Text, help_color);
  ui_->help_text->setPalette(help_palette);

  // Make it bold
  QFont help_font = ui_->help_text->font();
  help_font.setBold(true);
  ui_->help_text->setFont(help_font);

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

  ReloadSettings();
}

GlobalSearchView::~GlobalSearchView() {
  delete ui_;
}

namespace {
  bool CompareProviderName(SearchProvider* left, SearchProvider* right) {
    return left->name() < right->name();
  }
}

void GlobalSearchView::ReloadSettings() {
  // Delete any old status widgets
  qDeleteAll(provider_status_widgets_);
  provider_status_widgets_.clear();

  // Sort the list of providers alphabetically
  QList<SearchProvider*> providers = engine_->providers();
  qSort(providers.begin(), providers.end(), CompareProviderName);

  bool any_disabled = false;

  foreach (SearchProvider* provider, providers) {
    QWidget* parent = ui_->enabled_list;
    if (!engine_->is_provider_usable(provider)) {
      parent = ui_->disabled_list;
      any_disabled = true;
    }

    SearchProviderStatusWidget* widget =
        new SearchProviderStatusWidget(engine_, provider);

    parent->layout()->addWidget(widget);
    provider_status_widgets_ << widget;
  }

  ui_->disabled_label->setVisible(any_disabled);

  // Update models to use pretty covers.
  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);
  const bool pretty = s.value("pretty_covers", true).toBool();
  front_model_->set_use_pretty_covers(pretty);
  back_model_->set_use_pretty_covers(pretty);
}

void GlobalSearchView::StartSearch(const QString& query) {
  ui_->search->set_text(query);
  TextEdited(query);

  // Swap models immediately
  swap_models_timer_->stop();
  SwapModels();
}

void GlobalSearchView::TextEdited(const QString& text) {
  const QString trimmed(text.trimmed());

  // Add results to the back model, switch models after some delay.
  back_model_->Clear();
  current_model_ = back_model_;
  current_proxy_ = back_proxy_;
  swap_models_timer_->start();

  // Cancel the last search (if any) and start the new one.
  engine_->CancelSearch(last_search_id_);
  // If text query is empty, don't start a new search
  if (trimmed.isEmpty()) {
    last_search_id_ = -1;
  } else {
    last_search_id_ = engine_->SearchAsync(trimmed);
  }
}

void GlobalSearchView::AddResults(int id, const SearchProvider::ResultList& results) {
  if (id != last_search_id_ || results.isEmpty())
    return;

  current_model_->AddResults(results);
}

void GlobalSearchView::SwapModels() {
  art_requests_.clear();

  qSwap(front_model_, back_model_);
  qSwap(front_proxy_, back_proxy_);

  ui_->results->setModel(front_proxy_);

  if (ui_->search->text().trimmed().isEmpty()) {
    ui_->stack->setCurrentWidget(ui_->help_page);
  } else {
    ui_->stack->setCurrentWidget(ui_->results_page);
  }
}

void GlobalSearchView::LazyLoadArt(const QModelIndex& proxy_index) {
  if (!proxy_index.isValid() || proxy_index.model() != front_proxy_) {
    return;
  }

  // Already loading art for this item?
  if (proxy_index.data(GlobalSearchModel::Role_LazyLoadingArt).isValid()) {
    return;
  }

  // Should we even load art at all?
  if (!app_->library_model()->use_pretty_covers()) {
    return;
  }

  // Is this an album?
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
  item->setData(true, GlobalSearchModel::Role_LazyLoadingArt);

  // Walk down the item's children until we find a track
  while (item->rowCount()) {
    item = item->child(0);
  }

  // Get the track's Result
  const SearchProvider::Result result =
      item->data(GlobalSearchModel::Role_Result).value<SearchProvider::Result>();

  // Load the art.
  int id = engine_->LoadArtAsync(result);
  art_requests_[id] = source_index;
}

void GlobalSearchView::ArtLoaded(int id, const QPixmap& pixmap) {
  if (!art_requests_.contains(id))
    return;
  QModelIndex index = art_requests_.take(id);

  if (!pixmap.isNull()) {
    front_model_->itemFromIndex(index)->setData(pixmap, Qt::DecorationRole);
  }
}

MimeData* GlobalSearchView::SelectedMimeData() {
  // Get all selected model indexes
  QModelIndexList indexes = ui_->results->selectionModel()->selectedRows();
  if (indexes.isEmpty()) {
    // There's nothing selected - take the first thing in the model that isn't
    // a divider.
    for (int i=0 ; i<front_proxy_->rowCount() ; ++i) {
      QModelIndex index = front_proxy_->index(i, 0);
      if (!index.data(LibraryModel::Role_IsDivider).toBool()) {
        indexes << index;
        break;
      }
    }
  }

  // Still got nothing?  Give up.
  if (indexes.isEmpty()) {
    return NULL;
  }

  // Get items for these indexes
  QList<QStandardItem*> items;
  foreach (const QModelIndex& index, indexes) {
    items << (front_model_->itemFromIndex(front_proxy_->mapToSource(index)));
  }

  // Get a MimeData for these items
  return engine_->LoadTracks(front_model_->GetChildResults(items));
}

bool GlobalSearchView::eventFilter(QObject* object, QEvent* event) {
  if (object == ui_->search && event->type() == QEvent::KeyRelease) {
    if (SearchKeyEvent(static_cast<QKeyEvent*>(event))) {
      return true;
    }
  } else if (object == ui_->results && event->type() == QEvent::ContextMenu) {
    if (ResultsContextMenuEvent(static_cast<QContextMenuEvent*>(event))) {
      return true;
    }
  }

  return QWidget::eventFilter(object, event);
}

bool GlobalSearchView::SearchKeyEvent(QKeyEvent* event) {
  switch (event->key()) {
  case Qt::Key_Up:
    ui_->results->UpAndFocus();
    break;

  case Qt::Key_Down:
    ui_->results->DownAndFocus();
    break;

  case Qt::Key_Escape:
    static_cast<LineEditInterface*>(ui_->search)->clear();
    break;

  default:
    return false;
  }

  event->accept();
  return true;
}

bool GlobalSearchView::ResultsContextMenuEvent(QContextMenuEvent* event) {
  if (!context_menu_) {
    context_menu_ = new QMenu(this);
    context_menu_->addAction(IconLoader::Load("media-playback-start"),
        tr("Append to current playlist"), this, SLOT(AddSelectedToPlaylist()));
    context_menu_->addAction(IconLoader::Load("media-playback-start"),
        tr("Replace current playlist"), this, SLOT(LoadSelected()));
    context_menu_->addAction(IconLoader::Load("document-new"),
        tr("Open in new playlist"), this, SLOT(OpenSelectedInNewPlaylist()));

    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("go-next"),
        tr("Queue track"), this, SLOT(AddSelectedToPlaylistEnqueue()));
  }

  context_menu_->popup(event->globalPos());

  return true;
}

void GlobalSearchView::AddSelectedToPlaylist() {
  emit AddToPlaylist(SelectedMimeData());
}

void GlobalSearchView::LoadSelected() {
  MimeData* data = SelectedMimeData();
  data->clear_first_ = true;
  emit AddToPlaylist(data);
}

void GlobalSearchView::AddSelectedToPlaylistEnqueue() {
  MimeData* data = SelectedMimeData();
  data->enqueue_now_ = true;
  emit AddToPlaylist(data);
}

void GlobalSearchView::OpenSelectedInNewPlaylist() {
  MimeData* data = SelectedMimeData();
  data->open_in_new_playlist_ = true;
  emit AddToPlaylist(data);
}
