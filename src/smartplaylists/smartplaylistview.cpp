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

#include "smartplaylistmodel.h"
#include "smartplaylistview.h"
#include "ui/iconloader.h"

#include <QContextMenuEvent>
#include <QMenu>

SmartPlaylistView::SmartPlaylistView(QWidget* parent)
  : AutoExpandingTreeView(parent),
    menu_(NULL)
{
  setAttribute(Qt::WA_MacShowFocusRect, false);
  setHeaderHidden(true);
  setAllColumnsShowFocus(true);
  setDragEnabled(true);
  setDragDropMode(QAbstractItemView::DragOnly);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void SmartPlaylistView::contextMenuEvent(QContextMenuEvent* e) {
  if (!menu_) {
    menu_ = new QMenu(this);
    load_action_ = menu_->addAction(IconLoader::Load("media-playback-start"),
        tr("Load"), this, SLOT(Load()));
    add_action_ = menu_->addAction(IconLoader::Load("media-playback-start"),
        tr("Add to playlist"), this, SLOT(AddToPlaylist()));
    add_as_new_action_ = menu_->addAction(IconLoader::Load("view-media-playlist"),
        tr("Add as new playlist..."), this, SLOT(AddAsNewPlaylist()));
    menu_->addSeparator();
    new_smart_action_ = menu_->addAction(IconLoader::Load("list-add"),
        tr("New smart playlist..."), this, SLOT(NewSmartPlaylist()));
    new_folder_action_ = menu_->addAction(IconLoader::Load("folder-new"),
        tr("New folder..."), this, SLOT(NewFolder()));
    remove_action_ = menu_->addAction(IconLoader::Load("list-remove"),
        tr("Remove"), this, SLOT(Remove()));
  }

  menu_index_ = indexAt(e->pos());
  const SmartPlaylistModel::Type type = SmartPlaylistModel::Type(
        menu_index_.data(SmartPlaylistModel::Role_Type).toInt());
  const bool is_generator = type == SmartPlaylistModel::Type_Generator;

  load_action_->setEnabled(is_generator);
  add_action_->setEnabled(is_generator);
  add_as_new_action_->setEnabled(is_generator);

  menu_->popup(e->globalPos());
}

void SmartPlaylistView::Load() {
  if (menu_index_.isValid())
    emit Play(menu_index_, false, true);
}

void SmartPlaylistView::AddToPlaylist() {
  if (menu_index_.isValid())
    emit Play(menu_index_, false, false);
}

void SmartPlaylistView::AddAsNewPlaylist() {
  if (menu_index_.isValid())
    emit Play(menu_index_, true, false);
}

void SmartPlaylistView::NewSmartPlaylist() {
}

void SmartPlaylistView::NewFolder() {
}

void SmartPlaylistView::Remove() {
}
