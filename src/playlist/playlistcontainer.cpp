/* This file is part of Clementine.

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

#include "playlistcontainer.h"
#include "playlistmanager.h"
#include "ui_playlistcontainer.h"
#include "ui/iconloader.h"

#include <QUndoStack>
#include <QInputDialog>
#include <QSettings>

const char* PlaylistContainer::kSettingsGroup = "Playlist";

PlaylistContainer::PlaylistContainer(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_PlaylistContainer),
    undo_(NULL),
    redo_(NULL),
    starting_up_(true)
{
  ui_->setupUi(this);

  settings_.beginGroup(kSettingsGroup);

  // Icons
  ui_->clear->setIcon(IconLoader::Load("edit-clear-locationbar-ltr"));

  // Tab bar
  ui_->tab_bar->setExpanding(false);
  ui_->tab_bar->setMovable(true);
  ui_->tab_bar->setShape(QTabBar::RoundedSouth);

  // Connections
  connect(ui_->clear, SIGNAL(clicked()), SLOT(ClearFilter()));
  connect(ui_->tab_bar, SIGNAL(currentChanged(int)), SLOT(Save()));
}

PlaylistContainer::~PlaylistContainer() {
  delete ui_;
}

PlaylistView* PlaylistContainer::view() const {
  return ui_->playlist;
}

void PlaylistContainer::SetActions(
    QAction *new_playlist, QAction *save_playlist, QAction *load_playlist) {
  ui_->create_new->setDefaultAction(new_playlist);
  ui_->save->setDefaultAction(save_playlist);
  ui_->load->setDefaultAction(load_playlist);

  ui_->tab_bar->SetActions(new_playlist, save_playlist, load_playlist);

  connect(new_playlist, SIGNAL(triggered()), SLOT(NewPlaylist()));
  connect(save_playlist, SIGNAL(triggered()), SLOT(SavePlaylist()));
  connect(load_playlist, SIGNAL(triggered()), SLOT(LoadPlaylist()));
}

void PlaylistContainer::ClearFilter() {
  ui_->filter->clear();
  ui_->filter->setFocus();
}

void PlaylistContainer::SetManager(PlaylistManager *manager) {
  manager_ = manager;

  connect(ui_->tab_bar, SIGNAL(currentChanged(int)),
          manager, SLOT(SetCurrentPlaylist(int)));
  connect(ui_->tab_bar, SIGNAL(Rename(int,QString)),
          manager, SLOT(Rename(int,QString)));
  connect(ui_->tab_bar, SIGNAL(Remove(int)),
          manager, SLOT(Remove(int)));

  connect(manager, SIGNAL(CurrentChanged(Playlist*)),
          SLOT(SetViewModel(Playlist*)));
  connect(manager, SIGNAL(PlaylistAdded(int,QString)),
          SLOT(PlaylistAdded(int,QString)));
  connect(manager, SIGNAL(PlaylistRemoved(int)),
          SLOT(PlaylistRemoved(int)));
  connect(manager, SIGNAL(PlaylistRenamed(int,QString)),
          SLOT(PlaylistRenamed(int,QString)));
}

void PlaylistContainer::SetViewModel(Playlist* playlist) {
  // Set the view
  playlist->IgnoreSorting(true);
  view()->setModel(playlist);
  view()->SetItemDelegates(manager_->library_backend());
  playlist->IgnoreSorting(false);

  // Ensure that tab is current
  if (ui_->tab_bar->currentIndex() != manager_->current_index())
    ui_->tab_bar->setCurrentIndex(manager_->current_index());

  // Sort out the undo/redo actions
  delete undo_;
  delete redo_;
  undo_ = playlist->undo_stack()->createUndoAction(this);
  redo_ = playlist->undo_stack()->createRedoAction(this);
  undo_->setIcon(IconLoader::Load("edit-undo"));
  undo_->setShortcut(QKeySequence::Undo);
  redo_->setIcon(IconLoader::Load("edit-redo"));
  redo_->setShortcut(QKeySequence::Redo);

  ui_->undo->setDefaultAction(undo_);
  ui_->redo->setDefaultAction(redo_);

  emit UndoRedoActionsChanged(undo_, redo_);
}

void PlaylistContainer::ActivePlaying() {
  UpdateActiveIcon(QIcon(":tiny-start.png"));
}

void PlaylistContainer::ActivePaused() {
  UpdateActiveIcon(QIcon(":tiny-pause.png"));
}

void PlaylistContainer::ActiveStopped() {
  UpdateActiveIcon(QIcon());
}

void PlaylistContainer::UpdateActiveIcon(const QIcon& icon) {
  // Unset all existing icons
  for (int i=0 ; i<ui_->tab_bar->count() ; ++i) {
    ui_->tab_bar->setTabIcon(i, QIcon());
  }

  // Set our icon
  if (!icon.isNull())
    ui_->tab_bar->setTabIcon(manager_->active_index(), icon);
}

void PlaylistContainer::PlaylistAdded(int index, const QString &name) {
  ui_->tab_bar->insertTab(index, name);

  // Are we startup up, should we select this tab?
  if (starting_up_ && settings_.value("current_playlist", 0).toInt() == index) {
    starting_up_ = false;
    ui_->tab_bar->setCurrentIndex(index);
  }
}

void PlaylistContainer::PlaylistRemoved(int index) {
  ui_->tab_bar->removeTab(index);
}

void PlaylistContainer::PlaylistRenamed(int index, const QString &new_name) {
  ui_->tab_bar->setTabText(index, new_name);
}

void PlaylistContainer::NewPlaylist() {
  QString name = QInputDialog::getText(this, tr("New playlist"),
                                       tr("Enter a name for the new playlist"),
                                       QLineEdit::Normal, tr("Playlist"));
  if (name.isNull())
    return;

  manager_->New(name);
}

void PlaylistContainer::LoadPlaylist() {

}

void PlaylistContainer::SavePlaylist() {

}

void PlaylistContainer::Save() {
  if (starting_up_)
    return;

  settings_.setValue("current_playlist", ui_->tab_bar->currentIndex());
}
