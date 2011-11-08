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

#include "config.h"
#include "globalsearch.h"
#include "globalsearchitemdelegate.h"
#include "globalsearchsortmodel.h"
#include "globalsearchtooltip.h"
#include "globalsearchwidget.h"
#include "ui_globalsearchwidget.h"
#include "core/logging.h"
#include "core/stylesheetloader.h"
#include "core/utilities.h"
#include "playlist/playlistview.h"
#include "playlist/songmimedata.h"
#include "ui/qt_blurimage.h"
#include "widgets/stylehelper.h"

#include <QDesktopWidget>
#include <QListView>
#include <QPainter>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>


const int GlobalSearchWidget::kMinVisibleItems = 3;
const int GlobalSearchWidget::kMaxVisibleItems = 25;
const int GlobalSearchWidget::kSwapModelsTimeoutMsec = 250;
const int GlobalSearchWidget::kSuggestionTimeoutMsec = 60000; // 1 minute
const int GlobalSearchWidget::kSuggestionCount = 3;


GlobalSearchWidget::GlobalSearchWidget(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_GlobalSearchWidget),
    engine_(NULL),
    last_id_(0),
    order_arrived_counter_(0),
    closed_since_search_began_(false),
    front_model_(new QStandardItemModel(this)),
    back_model_(new QStandardItemModel(this)),
    current_model_(front_model_),
    front_proxy_(new GlobalSearchSortModel(this)),
    back_proxy_(new GlobalSearchSortModel(this)),
    current_proxy_(front_proxy_),
    view_(new QListView),
    consume_focus_out_(false),
    swap_models_timer_(new QTimer(this)),
    background_(":allthethings.png"),
    desktop_(qApp->desktop()),
    show_tooltip_(true),
    combine_identical_results_(true),
    next_suggestion_timer_(new QTimer(this))
{
  ui_->setupUi(this);
  ReloadSettings();

  // Set up the sorting proxy model
  front_proxy_->setSourceModel(front_model_);
  front_proxy_->setDynamicSortFilter(true);
  front_proxy_->sort(0);

  back_proxy_->setSourceModel(back_model_);
  back_proxy_->setDynamicSortFilter(true);
  back_proxy_->sort(0);

  combine_cache_[front_model_] = new CombineCache(front_model_);
  combine_cache_[back_model_] = new CombineCache(back_model_);

  // Set up the popup
  view_->setObjectName("popup");
  view_->setWindowFlags(Qt::Popup);
  view_->setFocusPolicy(Qt::NoFocus);
  view_->setFocusProxy(ui_->search);
  view_->installEventFilter(this);

  view_->setModel(front_proxy_);
  view_->setItemDelegate(new GlobalSearchItemDelegate(this));
  view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view_->setEditTriggers(QAbstractItemView::NoEditTriggers);

  ui_->search->installEventFilter(this);

  // Actions
  add_              = new QAction(tr("Add to playlist"), this);
  add_and_play_     = new QAction(tr("Add and play now"), this);
  add_and_queue_    = new QAction(tr("Queue track"), this);
  replace_          = new QAction(tr("Replace current playlist"), this);
  replace_and_play_ = new QAction(tr("Replace and play now"), this);

  add_->setShortcut(QKeySequence(Qt::Key_Return));
  add_and_play_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
  add_and_queue_->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Return));
  replace_->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Return));
  replace_and_play_->setShortcut(QKeySequence(Qt::ALT | Qt::CTRL | Qt::Key_Return));

  connect(add_,              SIGNAL(triggered()), SLOT(AddCurrent()));
  connect(add_and_play_,     SIGNAL(triggered()), SLOT(AddAndPlayCurrent()));
  connect(add_and_queue_,    SIGNAL(triggered()), SLOT(AddAndQueueCurrent()));
  connect(replace_,          SIGNAL(triggered()), SLOT(ReplaceCurrent()));
  connect(replace_and_play_, SIGNAL(triggered()), SLOT(ReplaceAndPlayCurrent()));

  actions_ << add_ << add_and_play_ << add_and_queue_ << replace_
           << replace_and_play_;

  // Load style sheets
  StyleSheetLoader* style_loader = new StyleSheetLoader(this);
  style_loader->SetStyleSheet(this, ":globalsearch.css");

  // Icons
  ui_->settings->setIcon(IconLoader::Load("configure"));

  swap_models_timer_->setSingleShot(true);
  swap_models_timer_->setInterval(kSwapModelsTimeoutMsec);

  next_suggestion_timer_->setInterval(kSuggestionTimeoutMsec);
  hint_text_ = ui_->search->hint();

  connect(ui_->search, SIGNAL(textEdited(QString)), SLOT(TextEdited(QString)));
  connect(view_, SIGNAL(doubleClicked(QModelIndex)), SLOT(ResultDoubleClicked()));
  connect(view_->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          SLOT(UpdateTooltip()));
  connect(swap_models_timer_, SIGNAL(timeout()), SLOT(SwapModels()));
  connect(ui_->settings, SIGNAL(clicked()), SLOT(SettingsClicked()));
  connect(next_suggestion_timer_, SIGNAL(timeout()), SLOT(NextSuggestion()));
}

GlobalSearchWidget::~GlobalSearchWidget() {
  delete ui_;
  qDeleteAll(combine_cache_.values());
}

void GlobalSearchWidget::Init(GlobalSearch* engine) {
  engine_ = engine;

  // These have to be queued connections because they may get emitted before
  // our call to Search() (or whatever) returns and we add the ID to the map.
  connect(engine_, SIGNAL(ResultsAvailable(int,SearchProvider::ResultList)),
          SLOT(AddResults(int,SearchProvider::ResultList)),
          Qt::QueuedConnection);
  connect(engine_, SIGNAL(ArtLoaded(int,QPixmap)), SLOT(ArtLoaded(int,QPixmap)),
          Qt::QueuedConnection);
  connect(engine_, SIGNAL(TracksLoaded(int,MimeData*)), SLOT(TracksLoaded(int,MimeData*)),
          Qt::QueuedConnection);

  view_->setStyle(new PlaylistProxyStyle(style()));

  // The style helper's base color doesn't get initialised until after the
  // constructor.
  QPalette view_palette = view_->palette();
  view_palette.setColor(QPalette::Text, Utils::StyleHelper::panelTextColor());
  view_palette.setColor(QPalette::HighlightedText, QColor(60, 60, 60));
  view_palette.setColor(QPalette::Base, Utils::StyleHelper::shadowColor().darker(109));

  QFont view_font = view_->font();
  view_font.setPointSizeF(Utils::StyleHelper::sidebarFontSize());

  view_->setFont(view_font);
  view_->setPalette(view_palette);
}

void GlobalSearchWidget::resizeEvent(QResizeEvent* e) {
  background_scaled_ = background_.scaled(size(), Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);

  QWidget::resizeEvent(e);
}

void GlobalSearchWidget::paintEvent(QPaintEvent* e) {
  QPainter p(this);

  QRect total_rect = rect().adjusted(0, 0, 1, 0);
  total_rect = style()->visualRect(layoutDirection(), geometry(), total_rect);
  Utils::StyleHelper::verticalGradient(&p, total_rect, total_rect);

  QRect background_rect = background_scaled_.rect();
  background_rect.moveLeft(ui_->settings->mapTo(this, ui_->settings->rect().center()).x() -
                           background_rect.width());
  background_rect.moveTop(total_rect.top());

  p.setOpacity(0.5);
  p.drawPixmap(background_rect, background_scaled_);
  p.setOpacity(1.0);

  p.setPen(Utils::StyleHelper::borderColor());
  p.drawLine(total_rect.topRight(), total_rect.bottomRight());

  QColor light = Utils::StyleHelper::sidebarHighlight();
  p.setPen(light);
  p.drawLine(total_rect.bottomLeft(), total_rect.bottomRight());
}

void GlobalSearchWidget::hideEvent(QHideEvent* e) {
  QWidget::hideEvent(e);

  next_suggestion_timer_->stop();
}

void GlobalSearchWidget::showEvent(QShowEvent* e) {
  QWidget::showEvent(e);

  next_suggestion_timer_->start();
  NextSuggestion();
}

void GlobalSearchWidget::TextEdited(const QString& text) {
  const QString trimmed_text = text.trimmed();
  closed_since_search_began_ = false;

  if (trimmed_text.length() < 3) {
    RepositionPopup();
    last_id_ = -1;
    return;
  }

  // Add results to the back model, switch models after some delay.
  back_model_->clear();
  combine_cache_[back_model_]->Clear();
  current_model_ = back_model_;
  current_proxy_ = back_proxy_;
  order_arrived_counter_ = 0;
  swap_models_timer_->start();

  // Cancel the last search (if any) and start the new one.
  engine_->CancelSearch(last_id_);
  last_id_ = engine_->SearchAsync(trimmed_text);
}

void GlobalSearchWidget::SwapModels() {
  art_requests_.clear();

  qSwap(front_model_, back_model_);
  qSwap(front_proxy_, back_proxy_);

  view_->setModel(front_proxy_);
  connect(view_->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          SLOT(UpdateTooltip()));

  if (!closed_since_search_began_)
    RepositionPopup();
}

void GlobalSearchWidget::AddResults(int id, const SearchProvider::ResultList& results) {
  if (id != last_id_)
    return;

  foreach (const SearchProvider::Result& result, results) {
    QStandardItem* item = new QStandardItem;
    item->setData(QVariant::fromValue(result), Role_PrimaryResult);
    item->setData(QVariant::fromValue(SearchProvider::ResultList() << result), Role_AllResults);
    item->setData(order_arrived_counter_, Role_OrderArrived);

    QPixmap pixmap;
    if (engine_->FindCachedPixmap(result, &pixmap)) {
      item->setData(pixmap, Qt::DecorationRole);
    }

    current_model_->appendRow(item);

    QModelIndex index = item->index();
    combine_cache_[current_model_]->Insert(index);

    if (combine_identical_results_) {
      // Maybe we can combine this result with an identical result from another
      // provider.
      QModelIndexList candidates = combine_cache_[current_model_]->FindCandidates(index);

      foreach (const QModelIndex& candidate, candidates) {
        if (!candidate.isValid())
          continue;

        CombineAction action = CanCombineResults(index, candidate);

        switch (action) {
        case CannotCombine:
          continue;

        case LeftPreferred:
          CombineResults(index, candidate);
          break;

        case RightPreferred:
          CombineResults(candidate, index);
          break;
        }

        // We've just invalidated the indexes so we have to stop.
        break;
      }
    }
  }

  order_arrived_counter_ ++;

  if (!closed_since_search_began_) {
    RepositionPopup();
    UpdateTooltipPosition();
  }
}

void GlobalSearchWidget::RepositionPopup() {
  if (front_model_->rowCount() == 0) {
    HidePopup(false);
    return;
  }

  closed_since_search_began_ = false;

  int h = view_->sizeHintForRow(0) * float(0.5 +
      qBound(kMinVisibleItems, front_model_->rowCount(), kMaxVisibleItems));
  int w = ui_->search->width();

  const QPoint pos = ui_->search->mapToGlobal(ui_->search->rect().bottomLeft());

  // Shrink the popup if it would otherwise go off the screen
  const QRect screen = desktop_->availableGeometry(ui_->search);
  h = qMin(h, screen.bottom() - pos.y());

  view_->setGeometry(QRect(pos, QSize(w, h)));

  if (!view_->isVisible()) {
    view_->show();
    ui_->search->setFocus();
  }
}

bool GlobalSearchWidget::eventFilter(QObject* o, QEvent* e) {
  if (o == ui_->search)
    return EventFilterSearchWidget(o, e);

  if (o == view_)
    return EventFilterPopup(o, e);

  return QWidget::eventFilter(o, e);
}

bool GlobalSearchWidget::EventFilterSearchWidget(QObject* o, QEvent* e) {
  switch (e->type()) {
  case QEvent::FocusOut:
    if (consume_focus_out_ && view_->isVisible())
      return true;
    break;

  case QEvent::FocusIn: {
    QFocusEvent* fe = static_cast<QFocusEvent*>(e);
    switch (fe->reason()) {
    case Qt::MouseFocusReason:
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
      if (!ui_->search->text().isEmpty())
        RepositionPopup();
      break;

    default:
      break;
    }

    break;
  }

  default:
    break;
  }

  return QWidget::eventFilter(o, e);
}

bool GlobalSearchWidget::EventFilterPopup(QObject*, QEvent* e) {
  // Most of this is borrowed from QCompleter::eventFilter

  switch (e->type()) {
  case QEvent::KeyPress: {
    QKeyEvent* ke = static_cast<QKeyEvent*>(e);

    QModelIndex cur_index = view_->currentIndex();
    const int key = ke->key();

    // Handle popup navigation keys. These are hardcoded because up/down might make the
    // widget do something else (lineedit cursor moves to home/end on mac, for instance)
    switch (key) {
    case Qt::Key_End:
    case Qt::Key_Home:
      if (ke->modifiers() & Qt::ControlModifier)
        return false;
      break;

    case Qt::Key_Up:
      if (!cur_index.isValid()) {
        view_->setCurrentIndex(front_proxy_->index(front_proxy_->rowCount() - 1, 0));
        return true;
      } else if (cur_index.row() == 0) {
        return true;
      }
      return false;

    case Qt::Key_Down:
      if (!cur_index.isValid()) {
        view_->setCurrentIndex(front_proxy_->index(0, 0));
        return true;
      } else if (cur_index.row() == front_proxy_->rowCount() - 1) {
        return true;
      }
      return false;

    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
      return false;
    }

    // Send the event to the widget. If the widget accepted the event, do nothing
    // If the widget did not accept the event, provide a default implementation
    consume_focus_out_ = false;
    (static_cast<QObject *>(ui_->search))->event(ke);
    consume_focus_out_ = true;

    if (e->isAccepted() || !view_->isVisible()) {
      // widget lost focus, hide the popup
      if (!ui_->search->hasFocus())
        HidePopup(true);
      if (e->isAccepted())
        return true;
    }

    // default implementation for keys not handled by the widget when popup is open
    switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
      // Handle the QActions here - they don't activate when the tooltip is showing
      if (ke->modifiers() & Qt::AltModifier && ke->modifiers() & Qt::ControlModifier)
        replace_and_play_->trigger();
      else if (ke->modifiers() & Qt::AltModifier)
        replace_->trigger();
      else if (ke->modifiers() & Qt::ControlModifier)
        add_and_play_->trigger();
      else if (ke->modifiers() & Qt::ShiftModifier)
        add_and_queue_->trigger();
      else
        add_->trigger();
      break;

    case Qt::Key_F4:
      if (ke->modifiers() & Qt::AltModifier)
        HidePopup(true);
      break;

    case Qt::Key_Backtab:
    case Qt::Key_Escape:
      HidePopup(true);
      break;

    default:
      break;
    }

    return true;
  }

  case QEvent::MouseButtonPress:
    if (!view_->underMouse()) {
      HidePopup(true);
      return true;
    }
    return false;

  case QEvent::InputMethod:
  case QEvent::ShortcutOverride:
    QApplication::sendEvent(ui_->search, e);
    break;

  default:
    return false;
  }

  return false;
}

void GlobalSearchWidget::LazyLoadArt(const QModelIndex& proxy_index) {
  if (!proxy_index.isValid() || proxy_index.data(Role_LazyLoadingArt).isValid()) {
    return;
  }
  if (proxy_index.model() != front_proxy_) {
    return;
  }

  const QModelIndex source_index = front_proxy_->mapToSource(proxy_index);
  front_model_->itemFromIndex(source_index)->setData(true, Role_LazyLoadingArt);

  const SearchProvider::Result result =
      source_index.data(Role_PrimaryResult).value<SearchProvider::Result>();

  int id = engine_->LoadArtAsync(result);
  art_requests_[id] = source_index;
}

void GlobalSearchWidget::ArtLoaded(int id, const QPixmap& pixmap) {
  if (!art_requests_.contains(id))
    return;
  QModelIndex index = art_requests_.take(id);

  front_model_->itemFromIndex(index)->setData(pixmap, Qt::DecorationRole);
}

void GlobalSearchWidget::ResultDoubleClicked() {
  LoadTracks(NULL);
}

void GlobalSearchWidget::AddCurrent() {
  LoadTracks(add_);
}

void GlobalSearchWidget::AddAndPlayCurrent() {
  LoadTracks(add_and_play_);
}

void GlobalSearchWidget::AddAndQueueCurrent() {
  LoadTracks(add_and_queue_);
}

void GlobalSearchWidget::ReplaceCurrent() {
  LoadTracks(replace_);
}

void GlobalSearchWidget::ReplaceAndPlayCurrent() {
  LoadTracks(replace_and_play_);
}

void GlobalSearchWidget::LoadTracks(QAction* trigger) {
  QModelIndex index = view_->currentIndex();
  if (!index.isValid())
    index = front_proxy_->index(0, 0);

  if (!index.isValid())
    return;

  int result_index = 0;
  if (tooltip_ && tooltip_->isVisible()) {
    result_index = tooltip_->ActiveResultIndex();
  }

  const SearchProvider::ResultList results =
      index.data(Role_AllResults).value<SearchProvider::ResultList>();

  if (result_index < 0 || result_index >= results.count())
    return;

  int id = engine_->LoadTracksAsync(results[result_index]);
  track_requests_[id] = trigger;
}

void GlobalSearchWidget::TracksLoaded(int id, MimeData* mime_data) {
  if (!track_requests_.contains(id))
    return;

  QAction* trigger = track_requests_.take(id);

  if (!mime_data)
    return;

  if (trigger == NULL) {
    mime_data->from_doubleclick_ = true;
  } else {
    if (trigger == add_and_play_) {
      mime_data->override_user_settings_ = true;
      mime_data->play_now_ = true;
    } else if (trigger == add_and_queue_) {
      mime_data->enqueue_now_ = true;
    } else if (trigger == replace_) {
      mime_data->clear_first_= true;
    } else if (trigger == replace_and_play_) {
      mime_data->clear_first_ = true;
      mime_data->override_user_settings_ = true;
      mime_data->play_now_ = true;
    }
  }

  emit AddToPlaylist(mime_data);
}

void GlobalSearchWidget::ReloadSettings() {
  QSettings s;
  s.beginGroup(GlobalSearch::kSettingsGroup);

  show_tooltip_ = s.value("tooltip", true).toBool();
  combine_identical_results_ = s.value("combine_identical_results", true).toBool();
  provider_order_ = s.value("provider_order", QStringList() << "library").toStringList();
  setVisible(s.value("show_globalsearch", true).toBool());

  if (tooltip_) {
    tooltip_->ReloadSettings();
  }

}

GlobalSearchWidget::CombineAction GlobalSearchWidget::CanCombineResults(
      const QModelIndex& left, const QModelIndex& right) const {
  const SearchProvider::Result r1 = left.data(Role_PrimaryResult)
      .value<SearchProvider::Result>();
  const SearchProvider::Result r2 = right.data(Role_PrimaryResult)
      .value<SearchProvider::Result>();

  // If you change the logic here remember to change CombineCache::Hash too.

  if (r1.match_quality_ != r2.match_quality_ || r1.type_ != r2.type_)
    return CannotCombine;

#define StringsDiffer(field) \
  (QString::compare(r1.metadata_.field(), r2.metadata_.field(), Qt::CaseInsensitive) != 0)

  switch (r1.type_) {
  case globalsearch::Type_Track:
    if (StringsDiffer(title))
      return CannotCombine;
    // fallthrough
  case globalsearch::Type_Album:
    if (StringsDiffer(album) || StringsDiffer(artist))
      return CannotCombine;
    break;
  case globalsearch::Type_Stream:
    if (StringsDiffer(url().toString))
      return CannotCombine;
    break;
  }

#undef StringsDiffer

  // They look the same - decide which provider we like best.
  const int p1 = provider_order_.indexOf(r1.provider_->id());
  const int p2 = provider_order_.indexOf(r2.provider_->id());

  return p2 > p1 ? LeftPreferred : RightPreferred;
}

void GlobalSearchWidget::CombineResults(const QModelIndex& superior, const QModelIndex& inferior) {
  QStandardItem* superior_item = current_model_->itemFromIndex(superior);
  QStandardItem* inferior_item = current_model_->itemFromIndex(inferior);

  SearchProvider::ResultList superior_results =
      superior_item->data(Role_AllResults).value<SearchProvider::ResultList>();
  SearchProvider::ResultList inferior_results =
      inferior_item->data(Role_AllResults).value<SearchProvider::ResultList>();

  superior_results.append(inferior_results);
  superior_item->setData(QVariant::fromValue(superior_results), Role_AllResults);

  combine_cache_[current_model_]->Remove(inferior_item->index());
  current_model_->invisibleRootItem()->removeRow(inferior_item->row());
}

void GlobalSearchWidget::HidePopup(bool manual) {
  if (manual) {
    closed_since_search_began_ = true;
  }

  if (tooltip_)
    tooltip_->hide();
  view_->hide();
}

void GlobalSearchWidget::UpdateTooltip() {
  if (!view_->isVisible() || !show_tooltip_) {
    if (tooltip_)
      tooltip_->hide();
    return;
  }

  const QModelIndex current = view_->selectionModel()->currentIndex();
  if (!current.isValid())
    return;

  const SearchProvider::ResultList results = current.data(Role_AllResults)
      .value<SearchProvider::ResultList>();

  if (!tooltip_) {
    tooltip_.reset(new GlobalSearchTooltip(view_));
    tooltip_->setFont(view_->font());
    tooltip_->setPalette(view_->palette());
    tooltip_->SetActions(actions_);
  }

  tooltip_->SetResults(results);
  UpdateTooltipPosition();
}

void GlobalSearchWidget::UpdateTooltipPosition() {
  if (!tooltip_ || !view_->isVisible())
    return;

  const QModelIndex current = view_->selectionModel()->currentIndex();
  if (!current.isValid()) {
    tooltip_->hide();
    return;
  }

  const QRect item_rect = view_->visualRect(current);
  const QPoint popup_pos = item_rect.topRight() +
      QPoint(-GlobalSearchTooltip::kArrowWidth,
             item_rect.height() / 2);

  tooltip_->ShowAt(view_->mapToGlobal(popup_pos));
}

void GlobalSearchWidget::SettingsClicked() {
  emit OpenSettingsAtPage(SettingsDialog::Page_GlobalSearch);
}

void GlobalSearchWidget::NextSuggestion() {
  const QStringList suggestions = engine_->GetSuggestions(kSuggestionCount);
  QString hint = hint_text_;

  if (!suggestions.isEmpty()) {
    hint += QString(", %1 ").arg(tr("e.g.")) + suggestions.join(", ");
  }

  ui_->search->set_hint(hint);
}

GlobalSearchWidget::CombineCache::CombineCache(QAbstractItemModel* model)
  : model_(model)
{
}

uint GlobalSearchWidget::CombineCache::Hash(const QModelIndex& index) {
  const SearchProvider::Result r = index.data(Role_PrimaryResult)
      .value<SearchProvider::Result>();

  uint ret = qHash(r.match_quality_) ^ qHash(r.type_);

  switch (r.type_) {
  case globalsearch::Type_Track:
    ret ^= qHash(r.metadata_.title());
    // fallthrough
  case globalsearch::Type_Album:
    ret ^= qHash(r.metadata_.album());
    ret ^= qHash(r.metadata_.artist());
    break;

  case globalsearch::Type_Stream:
    ret ^= qHash(r.metadata_.url().toString());
    break;
  }

  return ret;
}

void GlobalSearchWidget::CombineCache::Insert(const QModelIndex& index) {
  data_.insert(Hash(index), index.row());
}

void GlobalSearchWidget::CombineCache::Remove(const QModelIndex& index) {
  // This is really inefficient but we're not doing it much - find any items
  // with a row greater than this one and shuffle them down one.

  for (QMultiMap<uint, int>::iterator it = data_.begin() ; it != data_.end() ; ++it) {
    if (it.value() > index.row())
      (*it) --;
  }

  // Now remove the row itself.
  QMultiMap<uint, int>::iterator it = data_.find(Hash(index), index.row());
  if (it != data_.end())
    data_.erase(it);
}

QModelIndexList GlobalSearchWidget::CombineCache::FindCandidates(
    const QModelIndex& result) const {
  QModelIndexList ret;
  foreach (int row, data_.values(Hash(result))) {
    if (row != result.row()) {
      ret << model_->index(row, 0);
    }
  }

  return ret;
}

void GlobalSearchWidget::CombineCache::Clear() {
  data_.clear();
}
