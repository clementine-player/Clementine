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

#include "globalsearchview.h"

#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QTimer>

#include <algorithm>
#include <functional>

#include "globalsearch.h"
#include "globalsearchitemdelegate.h"
#include "globalsearchmodel.h"
#include "globalsearchsortmodel.h"
#include "searchprovider.h"
#include "searchproviderstatuswidget.h"
#include "suggestionwidget.h"
#include "ui_globalsearchview.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/mimedata.h"
#include "core/timeconstants.h"
#include "internet/core/internetsongmimedata.h"
#include "library/libraryfilterwidget.h"
#include "library/librarymodel.h"
#include "library/groupbydialog.h"
#include "playlist/songmimedata.h"

using std::placeholders::_1;
using std::placeholders::_2;

const int GlobalSearchView::kSwapModelsTimeoutMsec = 250;
const int GlobalSearchView::kMaxSuggestions = 10;
const int GlobalSearchView::kUpdateSuggestionsTimeoutMsec = 60 * kMsecPerSec;

GlobalSearchView::GlobalSearchView(Application* app, QWidget* parent)
    : QWidget(parent),
      app_(app),
      engine_(app_->global_search()),
      ui_(new Ui_GlobalSearchView),
      context_menu_(nullptr),
      last_search_id_(0),
      front_model_(new GlobalSearchModel(engine_, this)),
      back_model_(new GlobalSearchModel(engine_, this)),
      current_model_(front_model_),
      front_proxy_(new GlobalSearchSortModel(this)),
      back_proxy_(new GlobalSearchSortModel(this)),
      current_proxy_(front_proxy_),
      swap_models_timer_(new QTimer(this)),
      update_suggestions_timer_(new QTimer(this)),
      search_icon_(IconLoader::Load("search", IconLoader::Base)),
      warning_icon_(IconLoader::Load("dialog-warning", IconLoader::Base)),
      show_providers_(true),
      show_suggestions_(true) {
  ui_->setupUi(this);

  front_model_->set_proxy(front_proxy_);
  back_model_->set_proxy(back_proxy_);

  ui_->search->installEventFilter(this);
  ui_->results_stack->installEventFilter(this);

  ui_->settings->setIcon(IconLoader::Load("configure", IconLoader::Base));

  // Must be a queued connection to ensure the GlobalSearch handles it first.
  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()),
          Qt::QueuedConnection);

  connect(ui_->search, SIGNAL(textChanged(QString)), SLOT(TextEdited(QString)));
  connect(ui_->results, SIGNAL(AddToPlaylistSignal(QMimeData*)),
          SIGNAL(AddToPlaylist(QMimeData*)));
  connect(ui_->results, SIGNAL(FocusOnFilterSignal(QKeyEvent*)),
          SLOT(FocusOnFilter(QKeyEvent*)));

  // Set the appearance of the results list
  ui_->results->setItemDelegate(new GlobalSearchItemDelegate(this));
  ui_->results->setAttribute(Qt::WA_MacShowFocusRect, false);
  ui_->results->setStyleSheet("QTreeView::item{padding-top:1px;}");

  // Show the help page initially
  ui_->results_stack->setCurrentWidget(ui_->help_page);
  ui_->help_frame->setBackgroundRole(QPalette::Base);
  QVBoxLayout* enabled_layout = new QVBoxLayout(ui_->enabled_list);
  QVBoxLayout* disabled_layout = new QVBoxLayout(ui_->disabled_list);
  QVBoxLayout* suggestions_layout = new QVBoxLayout(ui_->suggestions_list);
  enabled_layout->setContentsMargins(16, 0, 16, 6);
  disabled_layout->setContentsMargins(16, 0, 16, 32);
  suggestions_layout->setContentsMargins(16, 0, 16, 6);

  // Set the colour of the help text to the disabled window text colour
  QPalette help_palette = ui_->help_text->palette();
  const QColor help_color =
      help_palette.color(QPalette::Disabled, QPalette::WindowText);
  help_palette.setColor(QPalette::Normal, QPalette::WindowText, help_color);
  help_palette.setColor(QPalette::Inactive, QPalette::WindowText, help_color);
  ui_->help_text->setPalette(help_palette);

  // Create suggestion widgets
  for (int i = 0; i < kMaxSuggestions; ++i) {
    SuggestionWidget* widget = new SuggestionWidget(search_icon_);
    connect(widget, SIGNAL(SuggestionClicked(QString)),
            SLOT(StartSearch(QString)));
    suggestions_layout->addWidget(widget);
    suggestion_widgets_ << widget;
  }

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

  update_suggestions_timer_->setInterval(kUpdateSuggestionsTimeoutMsec);
  connect(update_suggestions_timer_, SIGNAL(timeout()),
          SLOT(UpdateSuggestions()));

  // Add actions to the settings menu
  group_by_actions_ = LibraryFilterWidget::CreateGroupByActions(this);
  QMenu* settings_menu = new QMenu(this);
  settings_menu->addActions(group_by_actions_->actions());
  settings_menu->addSeparator();
  settings_menu->addAction(IconLoader::Load("configure", IconLoader::Base),
                           tr("Configure global search..."), this,
                           SLOT(OpenSettingsDialog()));
  ui_->settings->setMenu(settings_menu);

  connect(group_by_actions_, SIGNAL(triggered(QAction*)),
          SLOT(GroupByClicked(QAction*)));

  // These have to be queued connections because they may get emitted before
  // our call to Search() (or whatever) returns and we add the ID to the map.
  connect(engine_, SIGNAL(ResultsAvailable(int, SearchProvider::ResultList)),
          SLOT(AddResults(int, SearchProvider::ResultList)),
          Qt::QueuedConnection);
  connect(engine_, SIGNAL(ArtLoaded(int, QPixmap)),
          SLOT(ArtLoaded(int, QPixmap)), Qt::QueuedConnection);
}

GlobalSearchView::~GlobalSearchView() { delete ui_; }

namespace {
bool CompareProvider(const QStringList& provider_order, SearchProvider* left,
                     SearchProvider* right) {
  const int left_index = provider_order.indexOf(left->id());
  const int right_index = provider_order.indexOf(right->id());
  if (left_index == -1 && right_index == -1) {
    // None are in our provider list: compare name instead
    return left->name() < right->name();
  } else if (left_index == -1) {
    // Left provider not in provider list
    return false;
  } else if (right_index == -1) {
    // Right provider not in provider list
    return true;
  }
  return left_index < right_index;
}
}

void GlobalSearchView::ReloadSettings() {
  const bool old_show_suggestions = show_suggestions_;

  QSettings s;

  // Library settings
  s.beginGroup(LibraryView::kSettingsGroup);
  const bool pretty = s.value("pretty_covers", true).toBool();
  front_model_->set_use_pretty_covers(pretty);
  back_model_->set_use_pretty_covers(pretty);
  s.endGroup();

  // Global search settings
  s.beginGroup(GlobalSearch::kSettingsGroup);
  const QStringList provider_order =
      s.value("provider_order", QStringList() << "library").toStringList();
  front_model_->set_provider_order(provider_order);
  back_model_->set_provider_order(provider_order);
  show_providers_ = s.value("show_providers", true).toBool();
  show_suggestions_ = s.value("show_suggestions", true).toBool();
  SetGroupBy(LibraryModel::Grouping(
      LibraryModel::GroupBy(
          s.value("group_by1", int(LibraryModel::GroupBy_Artist)).toInt()),
      LibraryModel::GroupBy(
          s.value("group_by2", int(LibraryModel::GroupBy_Album)).toInt()),
      LibraryModel::GroupBy(
          s.value("group_by3", int(LibraryModel::GroupBy_None)).toInt())));
  s.endGroup();

  // Delete any old status widgets
  qDeleteAll(provider_status_widgets_);
  provider_status_widgets_.clear();

  // Toggle visibility of the providers group
  ui_->providers_group->setVisible(show_providers_);

  if (show_providers_) {
    // Sort the list of providers
    QList<SearchProvider*> providers = engine_->providers();
    std::sort(providers.begin(), providers.end(),
              std::bind(&CompareProvider, std::cref(provider_order), _1, _2));

    bool any_disabled = false;

    for (SearchProvider* provider : providers) {
      QWidget* parent = ui_->enabled_list;
      if (!engine_->is_provider_usable(provider)) {
        parent = ui_->disabled_list;
        any_disabled = true;
      }

      SearchProviderStatusWidget* widget =
          new SearchProviderStatusWidget(warning_icon_, engine_, provider);

      parent->layout()->addWidget(widget);
      provider_status_widgets_ << widget;
    }

    ui_->disabled_label->setVisible(any_disabled);
  }

  ui_->suggestions_group->setVisible(show_suggestions_);
  if (!show_suggestions_) {
    update_suggestions_timer_->stop();
  }

  if (!old_show_suggestions && show_suggestions_) {
    UpdateSuggestions();
  }
}

void GlobalSearchView::UpdateSuggestions() {
  const QStringList suggestions = engine_->GetSuggestions(kMaxSuggestions);

  for (int i = 0; i < suggestions.count(); ++i) {
    suggestion_widgets_[i]->SetText(suggestions[i]);
    suggestion_widgets_[i]->show();
  }

  for (int i = suggestions.count(); i < kMaxSuggestions; ++i) {
    suggestion_widgets_[i]->hide();
  }
}

void GlobalSearchView::StartSearch(const QString& query) {
  ui_->search->setText(query);
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

void GlobalSearchView::AddResults(int id,
                                  const SearchProvider::ResultList& results) {
  if (id != last_search_id_ || results.isEmpty()) return;

  current_model_->AddResults(results);
}

void GlobalSearchView::SwapModels() {
  art_requests_.clear();

  std::swap(front_model_, back_model_);
  std::swap(front_proxy_, back_proxy_);

  ui_->results->setModel(front_proxy_);

  if (ui_->search->text().trimmed().isEmpty()) {
    ui_->results_stack->setCurrentWidget(ui_->help_page);
  } else {
    ui_->results_stack->setCurrentWidget(ui_->results_page);
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
      container_type != LibraryModel::GroupBy_YearAlbum &&
      container_type != LibraryModel::GroupBy_OriginalYearAlbum) {
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
      item->data(GlobalSearchModel::Role_Result)
          .value<SearchProvider::Result>();

  // Load the art.
  int id = engine_->LoadArtAsync(result);
  art_requests_[id] = source_index;
}

void GlobalSearchView::ArtLoaded(int id, const QPixmap& pixmap) {
  if (!art_requests_.contains(id)) return;
  QModelIndex index = art_requests_.take(id);

  if (!pixmap.isNull()) {
    front_model_->itemFromIndex(index)->setData(pixmap, Qt::DecorationRole);
  }
}

MimeData* GlobalSearchView::SelectedMimeData() {
  if (!ui_->results->selectionModel()) return nullptr;

  // Get all selected model indexes
  QModelIndexList indexes = ui_->results->selectionModel()->selectedRows();
  if (indexes.isEmpty()) {
    // There's nothing selected - take the first thing in the model that isn't
    // a divider.
    for (int i = 0; i < front_proxy_->rowCount(); ++i) {
      QModelIndex index = front_proxy_->index(i, 0);
      if (!index.data(LibraryModel::Role_IsDivider).toBool()) {
        indexes << index;
        ui_->results->setCurrentIndex(index);
        break;
      }
    }
  }

  // Still got nothing?  Give up.
  if (indexes.isEmpty()) {
    return nullptr;
  }

  // Get items for these indexes
  QList<QStandardItem*> items;
  for (const QModelIndex& index : indexes) {
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
  } else if (object == ui_->results_stack &&
             event->type() == QEvent::ContextMenu) {
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
      ui_->search->clear();
      break;

    case Qt::Key_Return:
      AddSelectedToPlaylist();
      break;

    default:
      return false;
  }

  event->accept();
  return true;
}

bool GlobalSearchView::ResultsContextMenuEvent(QContextMenuEvent* event) {
  context_menu_ = new QMenu(this);
  context_actions_ << context_menu_->addAction(
      IconLoader::Load("media-playback-start", IconLoader::Base),
      tr("Append to current playlist"), this, SLOT(AddSelectedToPlaylist()));
  context_actions_ << context_menu_->addAction(
      IconLoader::Load("media-playback-start", IconLoader::Base),
      tr("Replace current playlist"), this, SLOT(LoadSelected()));
  context_actions_ << context_menu_->addAction(
      IconLoader::Load("document-new", IconLoader::Base),
      tr("Open in new playlist"), this, SLOT(OpenSelectedInNewPlaylist()));

  context_menu_->addSeparator();
  context_actions_ << context_menu_->addAction(
      IconLoader::Load("go-next", IconLoader::Base), tr("Queue track"), this,
      SLOT(AddSelectedToPlaylistEnqueue()));

  context_menu_->addSeparator();

  if (ui_->results->selectionModel() &&
      ui_->results->selectionModel()->selectedRows().length() == 1) {
    context_actions_ << context_menu_->addAction(
        IconLoader::Load("system-search", IconLoader::Base),
        tr("Search for this"), this, SLOT(SearchForThis()));
  }

  context_menu_->addSeparator();
  context_menu_->addMenu(tr("Group by"))
      ->addActions(group_by_actions_->actions());
  context_menu_->addAction(IconLoader::Load("configure", IconLoader::Base),
                           tr("Configure global search..."), this,
                           SLOT(OpenSettingsDialog()));

  const bool enable_context_actions =
      ui_->results->selectionModel() &&
      ui_->results->selectionModel()->hasSelection();

  for (QAction* action : context_actions_) {
    action->setEnabled(enable_context_actions);
  }

  context_menu_->popup(event->globalPos());

  return true;
}

void GlobalSearchView::AddSelectedToPlaylist() {
  emit AddToPlaylist(SelectedMimeData());
}

void GlobalSearchView::LoadSelected() {
  MimeData* data = SelectedMimeData();
  if (!data) return;

  data->clear_first_ = true;
  emit AddToPlaylist(data);
}

void GlobalSearchView::AddSelectedToPlaylistEnqueue() {
  MimeData* data = SelectedMimeData();
  if (!data) return;

  data->enqueue_now_ = true;
  emit AddToPlaylist(data);
}

void GlobalSearchView::OpenSelectedInNewPlaylist() {
  MimeData* data = SelectedMimeData();
  if (!data) return;

  data->open_in_new_playlist_ = true;
  emit AddToPlaylist(data);
}

void GlobalSearchView::SearchForThis() {
  StartSearch(
      ui_->results->selectionModel()->selectedRows().first().data().toString());
}

void GlobalSearchView::showEvent(QShowEvent* e) {
  if (show_suggestions_) {
    UpdateSuggestions();
    update_suggestions_timer_->start();
  }
  QWidget::showEvent(e);

  FocusSearchField();
}

void GlobalSearchView::FocusSearchField() {
  ui_->search->setFocus();
  ui_->search->selectAll();
}

void GlobalSearchView::hideEvent(QHideEvent* e) {
  update_suggestions_timer_->stop();
  QWidget::hideEvent(e);
}

void GlobalSearchView::FocusOnFilter(QKeyEvent* event) {
  ui_->search->setFocus();
  QApplication::sendEvent(ui_->search, event);
}

void GlobalSearchView::OpenSettingsDialog() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_GlobalSearch);
}

void GlobalSearchView::GroupByClicked(QAction* action) {
  if (action->property("group_by").isNull()) {
    if (!group_by_dialog_) {
      group_by_dialog_.reset(new GroupByDialog);
      connect(group_by_dialog_.data(), SIGNAL(Accepted(LibraryModel::Grouping)),
              SLOT(SetGroupBy(LibraryModel::Grouping)));
    }

    group_by_dialog_->show();
    return;
  }

  SetGroupBy(action->property("group_by").value<LibraryModel::Grouping>());
}

void GlobalSearchView::SetGroupBy(const LibraryModel::Grouping& g) {
  // Clear requests: changing "group by" on the models will cause all the items
  // to be removed/added
  // again, so all the QModelIndex here will become invalid. New requests will
  // be created for those
  // songs when they will be displayed again anyway (when
  // GlobalSearchItemDelegate::paint will call
  // LazyLoadArt)
  art_requests_.clear();
  // Update the models
  front_model_->SetGroupBy(g, true);
  back_model_->SetGroupBy(g, false);

  // Save the setting
  QSettings s;
  s.beginGroup(GlobalSearch::kSettingsGroup);
  s.setValue("group_by1", int(g.first));
  s.setValue("group_by2", int(g.second));
  s.setValue("group_by3", int(g.third));

  // Make sure the correct action is checked.
  for (QAction* action : group_by_actions_->actions()) {
    if (action->property("group_by").isNull()) continue;

    if (g == action->property("group_by").value<LibraryModel::Grouping>()) {
      action->setChecked(true);
      return;
    }
  }

  // Check the advanced action
  group_by_actions_->actions().last()->setChecked(true);
}
