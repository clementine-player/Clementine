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

#include "playlist.h"
#include "playlistmanager.h"
#include "playlisttabbar.h"
#include "songmimedata.h"
#include "radio/radiomimedata.h"
#include "ui/iconloader.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QInputDialog>
#include <QtDebug>

PlaylistTabBar::PlaylistTabBar(QWidget *parent)
  : QTabBar(parent),
    manager_(NULL),
    menu_(new QMenu(this)),
    menu_index_(-1),
    suppress_current_changed_(false)
{
  setAcceptDrops(true);

  remove_ = menu_->addAction(IconLoader::Load("list-remove"), tr("Remove playlist"), this, SLOT(Remove()));
  rename_ = menu_->addAction(IconLoader::Load("edit-rename"), tr("Rename playlist..."), this, SLOT(Rename()));
  save_ = menu_->addAction(IconLoader::Load("document-save"), tr("Save playlist..."), this, SLOT(Save()));
  menu_->addSeparator();

  connect(this, SIGNAL(currentChanged(int)), this, SLOT(CurrentIndexChanged(int)));
  connect(this, SIGNAL(tabMoved(int,int)), this, SLOT(TabMoved()));
}

void PlaylistTabBar::SetActions(
    QAction* new_playlist, QAction* load_playlist) {
  menu_->insertAction(0, new_playlist);
  menu_->insertAction(0, load_playlist);

  new_ = new_playlist;
}

void PlaylistTabBar::SetManager(PlaylistManager *manager) {
  manager_ = manager;
}

void PlaylistTabBar::contextMenuEvent(QContextMenuEvent* e) {
  menu_index_ = tabAt(e->pos());
  rename_->setEnabled(menu_index_ != -1);
  remove_->setEnabled(menu_index_ != -1 && count() > 1 && menu_index_ != index_of(manager_->active_id()));
  save_->setEnabled(menu_index_ != -1);

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

void PlaylistTabBar::Save() {
  if (menu_index_ == -1)
    return;

  emit Save(tabData(menu_index_).toInt());
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

void PlaylistTabBar::dragEnterEvent(QDragEnterEvent* e) {
  if (e->mimeData()->hasUrls() ||
      e->mimeData()->hasFormat(Playlist::kRowsMimetype) ||
      qobject_cast<const SongMimeData*>(e->mimeData()) ||
      qobject_cast<const RadioMimeData*>(e->mimeData())) {
    e->acceptProposedAction();
  }
}

void PlaylistTabBar::dragMoveEvent(QDragMoveEvent* e) {
  drag_hover_tab_ = tabAt(e->pos());

  if (drag_hover_tab_ == -1) {
    e->setDropAction(Qt::IgnoreAction);
    e->ignore();
    drag_hover_timer_.stop();
  } else {
    e->setDropAction(Qt::CopyAction);
    e->accept(tabRect(drag_hover_tab_));

    if (!drag_hover_timer_.isActive())
      drag_hover_timer_.start(kDragHoverTimeout, this);
  }
}

void PlaylistTabBar::dragLeaveEvent(QDragLeaveEvent*) {
  drag_hover_timer_.stop();
}

void PlaylistTabBar::timerEvent(QTimerEvent* e) {
  QTabBar::timerEvent(e);

  if (e->timerId() == drag_hover_timer_.timerId()) {
    drag_hover_timer_.stop();
    if (drag_hover_tab_ != -1)
      setCurrentIndex(drag_hover_tab_);
  }
}

void PlaylistTabBar::dropEvent(QDropEvent* e) {
  if (drag_hover_tab_ == -1) {
    e->ignore();
    return;
  }

  setCurrentIndex(drag_hover_tab_);
  manager_->current()->dropMimeData(e->mimeData(), e->proposedAction(), -1, 0, QModelIndex());
}
