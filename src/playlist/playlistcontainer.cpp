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

#include "core/appearance.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "playlistcontainer.h"
#include "playlistmanager.h"
#include "playlistparsers/playlistparser.h"
#include "ui/iconloader.h"
#include "ui_playlistcontainer.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QTimeLine>
#include <QTimer>
#include <QUndoStack>

const char* PlaylistContainer::kSettingsGroup = "Playlist";
const int PlaylistContainer::kFilterDelayMs = 100;
const int PlaylistContainer::kFilterDelayPlaylistSizeThreshold = 5000;

PlaylistContainer::PlaylistContainer(QWidget* parent)
    : QWidget(parent),
      ui_(new Ui_PlaylistContainer),
      manager_(nullptr),
      undo_(nullptr),
      redo_(nullptr),
      playlist_(nullptr),
      starting_up_(true),
      tab_bar_visible_(false),
      tab_bar_animation_(new QTimeLine(500, this)),
      no_matches_label_(nullptr),
      filter_timer_(new QTimer(this)) {
  ui_->setupUi(this);

  no_matches_label_ = new QLabel(ui_->playlist);
  no_matches_label_->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
  no_matches_label_->setAttribute(Qt::WA_TransparentForMouseEvents);
  no_matches_label_->setWordWrap(true);
  no_matches_label_->raise();
  no_matches_label_->hide();

  // Set the colour of the no matches label to the disabled text colour
  QPalette no_matches_palette = no_matches_label_->palette();
  const QColor no_matches_color =
      no_matches_palette.color(QPalette::Disabled, QPalette::Text);
  no_matches_palette.setColor(QPalette::Normal, QPalette::WindowText,
                              no_matches_color);
  no_matches_palette.setColor(QPalette::Inactive, QPalette::WindowText,
                              no_matches_color);
  no_matches_label_->setPalette(no_matches_palette);

  // Remove QFrame border
  ui_->toolbar->setStyleSheet("QFrame { border: 0px; }");

  QSettings settings;
  settings.beginGroup(Appearance::kSettingsGroup);
  bool hide_toolbar = settings.value("b_hide_filter_toolbar", false).toBool();
  ui_->toolbar->setVisible(!hide_toolbar);

  // Make it bold
  QFont no_matches_font = no_matches_label_->font();
  no_matches_font.setBold(true);
  no_matches_label_->setFont(no_matches_font);

  settings_.beginGroup(kSettingsGroup);

  // Tab bar
  ui_->tab_bar->setExpanding(false);
  ui_->tab_bar->setMovable(true);

  connect(tab_bar_animation_, SIGNAL(frameChanged(int)),
          SLOT(SetTabBarHeight(int)));
  ui_->tab_bar->setMaximumHeight(0);

  // Connections
  connect(ui_->tab_bar, SIGNAL(currentChanged(int)), SLOT(Save()));
  connect(ui_->tab_bar, SIGNAL(Save(int)), SLOT(SavePlaylist(int)));

  // set up timer for delayed filter updates
  filter_timer_->setSingleShot(true);
  filter_timer_->setInterval(kFilterDelayMs);
  connect(filter_timer_, SIGNAL(timeout()), this, SLOT(UpdateFilter()));

  // Replace playlist search filter with native search box.
  connect(ui_->filter, SIGNAL(textChanged(QString)), SLOT(MaybeUpdateFilter()));
  connect(ui_->playlist, SIGNAL(FocusOnFilterSignal(QKeyEvent*)),
          SLOT(FocusOnFilter(QKeyEvent*)));
  ui_->filter->installEventFilter(this);
}

PlaylistContainer::~PlaylistContainer() { delete ui_; }

PlaylistView* PlaylistContainer::view() const { return ui_->playlist; }

void PlaylistContainer::SetActions(QAction* new_playlist,
                                   QAction* load_playlist,
                                   QAction* save_playlist,
                                   QAction* next_playlist,
                                   QAction* previous_playlist) {
  ui_->create_new->setDefaultAction(new_playlist);
  ui_->save->setDefaultAction(save_playlist);
  ui_->load->setDefaultAction(load_playlist);

  ui_->tab_bar->SetActions(new_playlist, load_playlist);

  connect(new_playlist, SIGNAL(triggered()), SLOT(NewPlaylist()));
  connect(save_playlist, SIGNAL(triggered()), SLOT(SavePlaylist()));
  connect(load_playlist, SIGNAL(triggered()), SLOT(LoadPlaylist()));
  connect(next_playlist, SIGNAL(triggered()), SLOT(GoToNextPlaylistTab()));
  connect(previous_playlist, SIGNAL(triggered()),
          SLOT(GoToPreviousPlaylistTab()));
}

void PlaylistContainer::SetManager(PlaylistManager* manager) {
  manager_ = manager;
  ui_->tab_bar->SetManager(manager);

  connect(ui_->tab_bar, SIGNAL(CurrentIdChanged(int)), manager,
          SLOT(SetCurrentPlaylist(int)));
  connect(ui_->tab_bar, SIGNAL(Rename(int, QString)), manager,
          SLOT(Rename(int, QString)));
  connect(ui_->tab_bar, SIGNAL(Close(int)), manager, SLOT(Close(int)));
  connect(ui_->tab_bar, SIGNAL(PlaylistFavorited(int, bool)), manager,
          SLOT(Favorite(int, bool)));

  connect(ui_->tab_bar, SIGNAL(PlaylistOrderChanged(QList<int>)), manager,
          SLOT(ChangePlaylistOrder(QList<int>)));

  connect(manager, SIGNAL(CurrentChanged(Playlist*)),
          SLOT(SetViewModel(Playlist*)));
  connect(manager, SIGNAL(PlaylistAdded(int, QString, bool)),
          SLOT(PlaylistAdded(int, QString, bool)));
  connect(manager, SIGNAL(PlaylistManagerInitialized()), SLOT(Started()));
  connect(manager, SIGNAL(PlaylistClosed(int)), SLOT(PlaylistClosed(int)));
  connect(manager, SIGNAL(PlaylistRenamed(int, QString)),
          SLOT(PlaylistRenamed(int, QString)));
}

void PlaylistContainer::SetViewModel(Playlist* playlist) {
  if (view()->selectionModel()) {
    disconnect(view()->selectionModel(),
               SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
               SLOT(SelectionChanged()));
  }
  if (playlist_ && playlist_->proxy()) {
    disconnect(playlist_->proxy(), SIGNAL(modelReset()), this,
               SLOT(UpdateNoMatchesLabel()));
    disconnect(playlist_->proxy(), SIGNAL(rowsInserted(QModelIndex, int, int)),
               this, SLOT(UpdateNoMatchesLabel()));
    disconnect(playlist_->proxy(), SIGNAL(rowsRemoved(QModelIndex, int, int)),
               this, SLOT(UpdateNoMatchesLabel()));
  }
  if (playlist_) {
    disconnect(playlist_, SIGNAL(modelReset()), this,
               SLOT(UpdateNoMatchesLabel()));
    disconnect(playlist_, SIGNAL(rowsInserted(QModelIndex, int, int)), this,
               SLOT(UpdateNoMatchesLabel()));
    disconnect(playlist_, SIGNAL(rowsRemoved(QModelIndex, int, int)), this,
               SLOT(UpdateNoMatchesLabel()));
  }

  playlist_ = playlist;

  // Set the view
  playlist->IgnoreSorting(true);
  view()->setModel(playlist->proxy());
  view()->SetItemDelegates(manager_->library_backend());
  view()->SetPlaylist(playlist);
  view()->selectionModel()->select(manager_->current_selection(),
                                   QItemSelectionModel::ClearAndSelect);
  playlist->IgnoreSorting(false);

  connect(view()->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
          SLOT(SelectionChanged()));
  emit ViewSelectionModelChanged();

  // Update filter
  ui_->filter->setText(playlist->proxy()->filterRegExp().pattern());

  // Update the no matches label
  connect(playlist_->proxy(), SIGNAL(modelReset()),
          SLOT(UpdateNoMatchesLabel()));
  connect(playlist_->proxy(), SIGNAL(rowsInserted(QModelIndex, int, int)),
          SLOT(UpdateNoMatchesLabel()));
  connect(playlist_->proxy(), SIGNAL(rowsRemoved(QModelIndex, int, int)),
          SLOT(UpdateNoMatchesLabel()));
  connect(playlist_, SIGNAL(modelReset()), SLOT(UpdateNoMatchesLabel()));
  connect(playlist_, SIGNAL(rowsInserted(QModelIndex, int, int)),
          SLOT(UpdateNoMatchesLabel()));
  connect(playlist_, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          SLOT(UpdateNoMatchesLabel()));
  UpdateNoMatchesLabel();

  // Ensure that tab is current
  if (ui_->tab_bar->current_id() != manager_->current_id())
    ui_->tab_bar->set_current_id(manager_->current_id());

  // Sort out the undo/redo actions
  delete undo_;
  delete redo_;
  undo_ = playlist->undo_stack()->createUndoAction(this);
  redo_ = playlist->undo_stack()->createRedoAction(this);
  undo_->setIcon(IconLoader::Load("edit-undo", IconLoader::Base));
  undo_->setShortcut(QKeySequence::Undo);
  redo_->setIcon(IconLoader::Load("edit-redo", IconLoader::Base));
  redo_->setShortcut(QKeySequence::Redo);

  ui_->undo->setDefaultAction(undo_);
  ui_->redo->setDefaultAction(redo_);

  emit UndoRedoActionsChanged(undo_, redo_);
}

void PlaylistContainer::ActivePlaying() {
  UpdateActiveIcon(IconLoader::Load("tiny-start", IconLoader::Other));
}

void PlaylistContainer::ActivePaused() {
  UpdateActiveIcon(IconLoader::Load("tiny-pause", IconLoader::Other));
}

void PlaylistContainer::ActiveStopped() { UpdateActiveIcon(QIcon()); }

void PlaylistContainer::UpdateActiveIcon(const QIcon& icon) {
  // Unset all existing icons
  for (int i = 0; i < ui_->tab_bar->count(); ++i) {
    ui_->tab_bar->setTabIcon(i, QIcon());
  }

  // Set our icon
  if (!icon.isNull()) ui_->tab_bar->set_icon_by_id(manager_->active_id(), icon);
}

void PlaylistContainer::PlaylistAdded(int id, const QString& name,
                                      bool favorite) {
  const int index = ui_->tab_bar->count();
  ui_->tab_bar->InsertTab(id, index, name, favorite);

  // Are we startup up, should we select this tab?
  if (starting_up_ && settings_.value("current_playlist", 1).toInt() == id) {
    starting_up_ = false;
    ui_->tab_bar->set_current_id(id);
  }

  if (ui_->tab_bar->count() > 1) {
    // Have to do this here because sizeHint() is only valid when there's a
    // tab in the bar.
    tab_bar_animation_->setFrameRange(0, ui_->tab_bar->sizeHint().height());

    if (!isVisible()) {
      // Skip the animation since the window is hidden (eg. if we're still
      // loading the UI).
      tab_bar_visible_ = true;
      ui_->tab_bar->setMaximumHeight(tab_bar_animation_->endFrame());
    } else {
      SetTabBarVisible(true);
    }
  }
}

void PlaylistContainer::Started() { starting_up_ = false; }

void PlaylistContainer::PlaylistClosed(int id) {
  ui_->tab_bar->RemoveTab(id);

  if (ui_->tab_bar->count() <= 1) SetTabBarVisible(false);
}

void PlaylistContainer::PlaylistRenamed(int id, const QString& new_name) {
  ui_->tab_bar->set_text_by_id(id, new_name);
}

void PlaylistContainer::NewPlaylist() { manager_->New(tr("Playlist")); }

void PlaylistContainer::LoadPlaylist() {
  QString filename = settings_.value("last_load_playlist").toString();
  filename = QFileDialog::getOpenFileName(this, tr("Load playlist"), filename,
                                          manager_->parser()->filters());

  if (filename.isNull()) return;

  settings_.setValue("last_load_playlist", filename);

  manager_->Load(filename);
}

void PlaylistContainer::SavePlaylist(int id = -1) {
  // Use the tab name as the suggested name
  QString suggested_name = ui_->tab_bar->tabText(ui_->tab_bar->currentIndex());

  manager_->SaveWithUI(id, suggested_name);
}

void PlaylistContainer::GoToNextPlaylistTab() {
  // Get the next tab' id
  int id_next = ui_->tab_bar->id_of((ui_->tab_bar->currentIndex() + 1) %
                                    ui_->tab_bar->count());
  // Switch to next tab
  manager_->SetCurrentPlaylist(id_next);
}

void PlaylistContainer::GoToPreviousPlaylistTab() {
  // Get the next tab' id
  int id_previous = ui_->tab_bar->id_of(
      (ui_->tab_bar->currentIndex() + ui_->tab_bar->count() - 1) %
      ui_->tab_bar->count());
  // Switch to next tab
  manager_->SetCurrentPlaylist(id_previous);
}

void PlaylistContainer::Save() {
  if (starting_up_) return;

  settings_.setValue("current_playlist", ui_->tab_bar->current_id());
}

void PlaylistContainer::SetTabBarVisible(bool visible) {
  if (tab_bar_visible_ == visible) return;
  tab_bar_visible_ = visible;

  tab_bar_animation_->setDirection(visible ? QTimeLine::Forward
                                           : QTimeLine::Backward);
  tab_bar_animation_->start();
}

void PlaylistContainer::SetTabBarHeight(int height) {
  ui_->tab_bar->setMaximumHeight(height);
}

void PlaylistContainer::MaybeUpdateFilter() {
  // delaying the filter update on small playlists is undesirable
  // and an empty filter applies very quickly, too
  if (manager_->current()->rowCount() < kFilterDelayPlaylistSizeThreshold ||
      ui_->filter->text().isEmpty()) {
    UpdateFilter();
  } else {
    filter_timer_->start();
  }
}

void PlaylistContainer::UpdateFilter() {
  manager_->current()->proxy()->setFilterFixedString(ui_->filter->text());
  ui_->playlist->JumpToCurrentlyPlayingTrack();

  UpdateNoMatchesLabel();
}

void PlaylistContainer::UpdateNoMatchesLabel() {
  Playlist* playlist = manager_->current();
  const bool has_rows = playlist->rowCount() != 0;
  const bool has_results = playlist->proxy()->rowCount() != 0;

  QString text;
  if (has_rows && !has_results) {
    if (ui_->filter->text().trimmed().compare(
            "the answer to life the universe "
            "and everything",
            Qt::CaseInsensitive) == 0) {
      text = "42";
    } else {
      text = tr(
          "No matches found.  Clear the search box to show the whole playlist "
          "again.");
    }
  }

  if (!text.isEmpty()) {
    no_matches_label_->setText(text);
    RepositionNoMatchesLabel(true);
    no_matches_label_->show();
  } else {
    no_matches_label_->hide();
  }
}

void PlaylistContainer::resizeEvent(QResizeEvent* e) {
  QWidget::resizeEvent(e);
  RepositionNoMatchesLabel();
}

void PlaylistContainer::FocusOnFilter(QKeyEvent* event) {
  ui_->filter->setFocus();

  switch (event->key()) {
    case Qt::Key_Backspace:
      break;

    case Qt::Key_Escape:
      ui_->filter->clear();
      break;

    default:
      ui_->filter->setText(ui_->filter->text() + event->text());
      break;
  }
}

void PlaylistContainer::RepositionNoMatchesLabel(bool force) {
  if (!force && !no_matches_label_->isVisible()) return;

  const int kBorder = 10;

  QPoint pos =
      ui_->playlist->viewport()->mapTo(ui_->playlist, QPoint(kBorder, kBorder));
  QSize size = ui_->playlist->viewport()->size();
  size.setWidth(size.width() - kBorder * 2);
  size.setHeight(size.height() - kBorder * 2);

  no_matches_label_->move(pos);
  no_matches_label_->resize(size);
}

void PlaylistContainer::SelectionChanged() {
  manager_->SelectionChanged(view()->selectionModel()->selection());
}

bool PlaylistContainer::eventFilter(QObject* objectWatched, QEvent* event) {
  if (objectWatched == ui_->filter) {
    if (event->type() == QEvent::KeyPress) {
      QKeyEvent* e = static_cast<QKeyEvent*>(event);
      switch (e->key()) {
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Return:
        case Qt::Key_Enter:
          view()->setFocus(Qt::OtherFocusReason);
          QApplication::sendEvent(ui_->playlist, event);
          return true;
        case Qt::Key_Escape:
          ui_->filter->clear();
          return true;
        default:
          break;
      }
    }
  }
  return QWidget::eventFilter(objectWatched, event);
}

void PlaylistContainer::ReloadSettings() {
  QSettings settings;
  settings.beginGroup(Appearance::kSettingsGroup);
  bool hide_toolbar = settings.value("b_hide_filter_toolbar", false).toBool();
  ui_->toolbar->setVisible(!hide_toolbar);
  settings.endGroup();

  settings.beginGroup(Player::kSettingsGroup);
  if (settings.value("play_count_short_duration").toBool()) {
    playlist_->set_max_play_count_point_nsecs(60ll * kNsecPerSec);
  } else {
    playlist_->set_max_play_count_point_nsecs(240ll * kNsecPerSec);
  }
  settings.endGroup();

  qLog(Debug) << "new max scrobble point:"
              << (playlist_->get_max_play_count_point_nsecs() / kNsecPerSec);
}
