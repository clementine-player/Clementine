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

#include "playlisttabbar.h"
#include "ui/iconloader.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QInputDialog>

PlaylistTabBar::PlaylistTabBar(QWidget *parent)
  : QTabBar(parent),
    menu_(new QMenu(this)),
    menu_index_(-1)
{
  rename_ = menu_->addAction(IconLoader::Load("edit-rename"), tr("Rename playlist"), this, SLOT(Rename()));
  remove_ = menu_->addAction(IconLoader::Load("list-remove"), tr("Remove playlist"), this, SLOT(Remove()));
  menu_->addSeparator();
}

void PlaylistTabBar::SetActions(
    QAction* new_playlist, QAction* save_playlist, QAction* load_playlist) {
  menu_->insertAction(0, new_playlist);
  menu_->insertAction(0, save_playlist);
  menu_->insertAction(0, load_playlist);
}

void PlaylistTabBar::contextMenuEvent(QContextMenuEvent* e) {
  menu_index_ = tabAt(e->pos());
  rename_->setEnabled(menu_index_ != -1);
  remove_->setEnabled(menu_index_ != -1 && count() > 1);

  menu_->popup(e->globalPos());
}

void PlaylistTabBar::Rename() {
  if (menu_index_ == -1)
    return;

  QString name = tabText(menu_index_);
  name = QInputDialog::getText(
      this, tr("Rename playlist"), tr("Enter a new name for this playlist"),
      QLineEdit::Normal, name);

  if (name.isNull())
    return;

  emit Rename(menu_index_, name);
}

void PlaylistTabBar::Remove() {
  if (menu_index_ == -1)
    return;

  emit Remove(menu_index_);
}
