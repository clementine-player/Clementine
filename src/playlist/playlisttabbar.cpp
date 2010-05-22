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
    menu_index_(-1),
    suppress_current_changed_(false)
{
  rename_ = menu_->addAction(IconLoader::Load("edit-rename"), tr("Rename playlist"), this, SLOT(Rename()));
  remove_ = menu_->addAction(IconLoader::Load("list-remove"), tr("Remove playlist"), this, SLOT(Remove()));
  menu_->addSeparator();

  connect(this, SIGNAL(currentChanged(int)), this, SLOT(CurrentIndexChanged(int)));
  connect(this, SIGNAL(tabMoved(int,int)), this, SLOT(TabMoved()));
}

void PlaylistTabBar::SetActions(
    QAction* new_playlist, QAction* save_playlist, QAction* load_playlist) {
  menu_->insertAction(0, new_playlist);
  menu_->insertAction(0, save_playlist);
  menu_->insertAction(0, load_playlist);

  new_ = new_playlist;
}

void PlaylistTabBar::contextMenuEvent(QContextMenuEvent* e) {
  menu_index_ = tabAt(e->pos());
  rename_->setEnabled(menu_index_ != -1);
  remove_->setEnabled(menu_index_ != -1 && count() > 1);

  menu_->popup(e->globalPos());
}

void PlaylistTabBar::mouseReleaseEvent(QMouseEvent* e) {
  if (e->button() == Qt::MidButton) {
    int index = tabAt(e->pos());
    if (index != -1) {
      emit Remove(tabData(index).toInt());
    }
  }

  QTabBar::mouseReleaseEvent(e);
}

void PlaylistTabBar::mouseDoubleClickEvent(QMouseEvent *e) {
  int index = tabAt(e->pos());
  if (index == -1) {
    new_->activate(QAction::Trigger);
  }

  QTabBar::mouseDoubleClickEvent(e);
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

  emit Rename(tabData(menu_index_).toInt(), name);
}

void PlaylistTabBar::Remove() {
  if (menu_index_ == -1)
    return;

  emit Remove(tabData(menu_index_).toInt());
}

int PlaylistTabBar::current_id() const {
  if (currentIndex() == -1)
    return -1;
  return tabData(currentIndex()).toInt();
}


int PlaylistTabBar::index_of(int id) const {
  for (int i=0 ; i<count() ; ++i) {
    if (tabData(i).toInt() == id) {
      return i;
    }
  }
  return -1;
}

void PlaylistTabBar::set_current_id(int id) {
  setCurrentIndex(index_of(id));
}

void PlaylistTabBar::set_icon_by_id(int id, const QIcon &icon) {
  setTabIcon(index_of(id), icon);
}

void PlaylistTabBar::RemoveTab(int id) {
  removeTab(index_of(id));
}

void PlaylistTabBar::set_text_by_id(int id, const QString &text) {
  setTabText(index_of(id), text);
}

void PlaylistTabBar::CurrentIndexChanged(int index) {
  if (!suppress_current_changed_)
    emit CurrentIdChanged(tabData(index).toInt());
}

void PlaylistTabBar::InsertTab(int id, int index, const QString& text) {
  suppress_current_changed_ = true;
  insertTab(index, text);
  setTabData(index, id);
  suppress_current_changed_ = false;

  if (currentIndex() == index)
    emit CurrentIdChanged(id);
}

void PlaylistTabBar::TabMoved() {
  QList<int> ids;
  for (int i=0 ; i<count() ; ++i) {
    ids << tabData(i).toInt();
  }
  emit PlaylistOrderChanged(ids);
}
