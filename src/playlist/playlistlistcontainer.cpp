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

#include "playlist.h"
#include "playlistlistcontainer.h"
#include "playlistlistmodel.h"
#include "playlistmanager.h"
#include "ui_playlistlistcontainer.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "ui/iconloader.h"

#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include <iostream>

/* This filter proxy will:
 - Accept all ancestors if at least a single child matches
 - Accept all children if at least a single ancestor matches

   The tree is then expanded only to the level at which the match occurs
*/
class PlaylistListFilterProxyModel : public QSortFilterProxyModel {
 public:
  explicit PlaylistListFilterProxyModel(QObject* parent)
      : QSortFilterProxyModel(parent) {}

  bool lessThan(const QModelIndex& left, const QModelIndex& right) const {
    // Compare the display text first.
    const int ret =
        left.data().toString().localeAwareCompare(right.data().toString());
    if (ret < 0) return true;
    if (ret > 0) return false;

    // Now use the source model row order to ensure we always get a
    // deterministic sorting even when two items are named the same.
    return left.row() < right.row();
  }

  QList<QModelIndex> expandList;

  void setFilterRegExp(const QRegExp & regExp) {
    expandList.clear();
    QSortFilterProxyModel::setFilterRegExp(regExp);
  }

  void refreshExpanded(QTreeView *tree) {
    tree->collapseAll();
    for (const QModelIndex& sourceIndex : expandList) {
      QModelIndex mappedIndex = mapFromSource( sourceIndex );
      tree->setExpanded( mappedIndex, true );
    }
  }

  // Depth first search of all the items
  bool hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const {
    QModelIndex item = sourceModel()->index(source_row,0,source_parent);
    if (!item.isValid()) {
      return false;
    }

    //check if there are children
    int childCount = item.model()->rowCount(item);
    if (childCount == 0)
      return false;

    for (int i = 0; i < childCount; ++i) {
      if (filterAcceptsRowItself(i, item))
        return true;

      if (hasAcceptedChildren(i, item))
        return true;
    }
    return false;
  }

  bool filterAcceptsRowItself(int source_row, const QModelIndex &source_parent) const {
    bool rv = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    if(rv) {
      if(sourceModel()->hasIndex(source_row,0,source_parent)) {
        QModelIndex idx = sourceModel()->index(source_row,0,source_parent);

        // Bit of a hack to get around the const in this function
        auto * me = const_cast<PlaylistListFilterProxyModel*>(this);

        QModelIndex pidx = sourceModel()->parent(idx);
        while(pidx.isValid()) {
          me->expandList.append(pidx);
          pidx = sourceModel()->parent(pidx);
        }
      }
    }
    return rv;
  }

  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    if (filterAcceptsRowItself(source_row, source_parent)) {
      return true;
    }

    //accept if any of the parents is accepted on it's own merits
    QModelIndex parent = source_parent;
    while (parent.isValid()) {
      if (filterAcceptsRowItself(parent.row(), parent.parent()))
        return true;
      parent = parent.parent();
    }

    //accept if any of the children is accepted on it's own merits
    if (hasAcceptedChildren(source_row, source_parent)) {
      return true;
    }

    return false;
  }
};

PlaylistListContainer::PlaylistListContainer(QWidget* parent)
    : QWidget(parent),
      app_(nullptr),
      ui_(new Ui_PlaylistListContainer),
      menu_(nullptr),
      action_new_folder_(new QAction(this)),
      action_remove_(new QAction(this)),
      action_save_playlist_(new QAction(this)),
      model_(new PlaylistListModel(this)),
      proxy_(new PlaylistListFilterProxyModel(this)),
      loaded_icons_(false),
      active_playlist_id_(-1) {
  ui_->setupUi(this);
  ui_->tree->setAttribute(Qt::WA_MacShowFocusRect, false);
  ui_->tree->SetAutoOpen(false);

  action_new_folder_->setText(tr("New folder"));
  action_remove_->setText(tr("Delete"));
  action_save_playlist_->setText(
      tr("Save playlist", "Save playlist menu action."));

  ui_->new_folder->setDefaultAction(action_new_folder_);
  ui_->remove->setDefaultAction(action_remove_);
  ui_->save_playlist->setDefaultAction(action_save_playlist_);

  connect(action_new_folder_, SIGNAL(triggered()), SLOT(NewFolderClicked()));
  connect(action_remove_, SIGNAL(triggered()), SLOT(DeleteClicked()));
  connect(action_save_playlist_, SIGNAL(triggered()), SLOT(SavePlaylist()));
  connect(model_, SIGNAL(PlaylistPathChanged(int, QString)),
          SLOT(PlaylistPathChanged(int, QString)));

  proxy_->setSourceModel(model_);
  proxy_->setDynamicSortFilter(true);
  proxy_->sort(0);
  ui_->tree->setModel(proxy_);

  connect(ui_->tree, SIGNAL(doubleClicked(QModelIndex)),
          SLOT(ItemDoubleClicked(QModelIndex)));

  model_->invisibleRootItem()->setData(PlaylistListModel::Type_Folder,
                                       PlaylistListModel::Role_Type);

  connect(ui_->search, SIGNAL(textChanged(QString)), SLOT(SearchTextEdited(QString)));
}

PlaylistListContainer::~PlaylistListContainer() { delete ui_; }

void PlaylistListContainer::showEvent(QShowEvent* e) {
  // Loading icons is expensive so only do it when the view is first opened
  if (loaded_icons_) {
    QWidget::showEvent(e);
    return;
  }
  loaded_icons_ = true;

  action_new_folder_->setIcon(IconLoader::Load("folder-new", IconLoader::Base));
  action_remove_->setIcon(IconLoader::Load("edit-delete", IconLoader::Base));
  action_save_playlist_->setIcon(IconLoader::Load("document-save", IconLoader::Base));

  model_->SetIcons(IconLoader::Load("view-media-playlist", IconLoader::Base),
                   IconLoader::Load("folder", IconLoader::Base));

  // Apply these icons to items that have already been created.
  RecursivelySetIcons(model_->invisibleRootItem());

  QWidget::showEvent(e);
}

void PlaylistListContainer::RecursivelySetIcons(QStandardItem* parent) const {
  for (int i = 0; i < parent->rowCount(); ++i) {
    QStandardItem* child = parent->child(i);
    switch (child->data(PlaylistListModel::Role_Type).toInt()) {
      case PlaylistListModel::Type_Folder:
        child->setIcon(model_->folder_icon());
        RecursivelySetIcons(child);
        break;

      case PlaylistListModel::Type_Playlist:
        child->setIcon(model_->playlist_icon());
        break;

      case PlaylistListModel::Type_Track:
        child->setIcon(model_->track_icon());
        break;
    }
  }
}

void PlaylistListContainer::SetApplication(Application* app) {
  app_ = app;
  PlaylistManager* manager = app_->playlist_manager();
  Player* player = app_->player();

  connect(manager, SIGNAL(PlaylistAdded(int, QString, bool)),
          SLOT(AddPlaylist(int, QString, bool)));
  connect(manager, SIGNAL(PlaylistFavorited(int, bool)),
          SLOT(PlaylistFavoriteStateChanged(int, bool)));
  connect(manager, SIGNAL(PlaylistRenamed(int, QString)),
          SLOT(PlaylistRenamed(int, QString)));
  connect(manager, SIGNAL(CurrentChanged(Playlist*)),
          SLOT(CurrentChanged(Playlist*)));
  connect(manager, SIGNAL(ActiveChanged(Playlist*)),
          SLOT(ActiveChanged(Playlist*)));

  connect(model_, SIGNAL(PlaylistRenamed(int, QString)), manager,
          SLOT(Rename(int, QString)));

  connect(player, SIGNAL(Paused()), SLOT(ActivePaused()));
  connect(player, SIGNAL(Playing()), SLOT(ActivePlaying()));
  connect(player, SIGNAL(Stopped()), SLOT(ActiveStopped()));

  // Get all playlists, even ones that are hidden in the UI.
  for (const PlaylistBackend::Playlist& p :
       app->playlist_backend()->GetAllFavoritePlaylists()) {
    AddPlaylist(p.id,p.name,true,&p.ui_path);
  }
}

void PlaylistListContainer::NewFolderClicked() {
  QString name = QInputDialog::getText(this, tr("New folder"),
                                       tr("Enter the name of the folder"));
  if (name.isEmpty()) {
    return;
  }

  name.replace("/", " ");

  model_->invisibleRootItem()->appendRow(model_->NewFolder(name));
}

void PlaylistListContainer::AddPlaylist(int id, const QString& name,
                                        bool favorite, const QString *ui_path) {
  if (!favorite) {
    return;
  }

  if (model_->PlaylistById(id)) {
    // We know about this playlist already - it was probably one of the open
    // ones that was loaded on startup.
    return;
  }

  if(ui_path == nullptr)
    ui_path = &app_->playlist_manager()->playlist(id)->ui_path();

  QStandardItem* playlist_item = model_->NewPlaylist(name, id);
  QStandardItem* parent_folder = model_->FolderByPath(*ui_path);
  parent_folder->appendRow(playlist_item);
  for (const Song& s : app_->playlist_backend()->GetPlaylistSongs(id)) {
    QStandardItem* track_item = model_->NewTrack(s);
    track_item->setDragEnabled(false);
    playlist_item->appendRow(track_item);
  }

}

void PlaylistListContainer::PlaylistRenamed(int id, const QString& new_name) {
  QStandardItem* item = model_->PlaylistById(id);
  if (!item) {
    return;
  }

  item->setText(new_name);
}

void PlaylistListContainer::RemovePlaylist(int id) {
  QStandardItem* item = model_->PlaylistById(id);
  if (item) {
    QStandardItem* parent = item->parent();
    if (!parent) {
      parent = model_->invisibleRootItem();
    }
    parent->removeRow(item->row());
  }
}

void PlaylistListContainer::SavePlaylist() {
  const QModelIndex& current_index =
      proxy_->mapToSource(ui_->tree->currentIndex());

  // Is it a playlist?
  if (current_index.data(PlaylistListModel::Role_Type).toInt() ==
      PlaylistListModel::Type_Playlist) {
    const int playlist_id =
        current_index.data(PlaylistListModel::Role_PlaylistId).toInt();
    QStandardItem* item = model_->PlaylistById(playlist_id);
    QString playlist_name = item ? item->text() : tr("Playlist");
    app_->playlist_manager()->SaveWithUI(playlist_id, playlist_name);
  }
}

void PlaylistListContainer::PlaylistFavoriteStateChanged(int id,
                                                         bool favorite) {
  if (favorite) {
    const QString& name = app_->playlist_manager()->GetPlaylistName(id);
    AddPlaylist(id, name, favorite);
  } else {
    RemovePlaylist(id);
  }
}

void PlaylistListContainer::ActiveChanged(Playlist* new_playlist) {
  const int new_id = new_playlist->id();

  if (new_id != active_playlist_id_) {
    UpdateActiveIcon(active_playlist_id_, QIcon());
  }

  active_playlist_id_ = new_id;
}

void PlaylistListContainer::CurrentChanged(Playlist* new_playlist) {
  if (!new_playlist) {
    return;
  }

  // Focus this playlist in the tree
  QStandardItem* item = model_->PlaylistById(new_playlist->id());
  if (!item) {
    return;
  }

  QModelIndex index = proxy_->mapFromSource(item->index());
  ui_->tree->selectionModel()->setCurrentIndex(
      index, QItemSelectionModel::ClearAndSelect);
  ui_->tree->scrollTo(index);
}

void PlaylistListContainer::SearchTextEdited(const QString& text) {
    QRegExp regexp(text);
    regexp.setCaseSensitivity(Qt::CaseInsensitive);

    proxy_->setFilterRegExp(regexp);

    if(regexp.isEmpty()) {
      ui_->tree->collapseAll();
    } else {
      proxy_->refreshExpanded(ui_->tree);
    }
}

void PlaylistListContainer::PlaylistPathChanged(int id,
                                                const QString& new_path) {
  // Update the path in the database
  app_->playlist_backend()->SetPlaylistUiPath(id, new_path);
  Playlist* playlist = app_->playlist_manager()->playlist(id);
  // Check the playlist exists (if it's not opened it's not in the manager)
  if (playlist) {
    playlist->set_ui_path(new_path);
  }
}

void PlaylistListContainer::ItemDoubleClicked(const QModelIndex& proxy_index) {
  const QModelIndex& index = proxy_->mapToSource(proxy_index);

  // Is it a playlist?
  if (index.data(PlaylistListModel::Role_Type).toInt() ==
      PlaylistListModel::Type_Playlist) {
    app_->playlist_manager()->SetCurrentOrOpen(
        index.data(PlaylistListModel::Role_PlaylistId).toInt());
  }
}

void PlaylistListContainer::DeleteClicked() {
  QSet<int> ids;
  QList<QPersistentModelIndex> folders_to_delete;

  for (const QModelIndex& proxy_index :
       ui_->tree->selectionModel()->selectedRows(0)) {
    const QModelIndex& index = proxy_->mapToSource(proxy_index);

    // Is it a playlist?
    switch (index.data(PlaylistListModel::Role_Type).toInt()) {
      case PlaylistListModel::Type_Playlist:
        ids << index.data(PlaylistListModel::Role_PlaylistId).toInt();
        break;

      case PlaylistListModel::Type_Folder:
        // Find all the playlists inside.
        RecursivelyFindPlaylists(index, &ids);
        folders_to_delete << index;
        break;
    }
  }

  // Make sure the user really wants to unfavorite all these playlists.
  if (ids.count() > 1) {
    const int button = QMessageBox::question(
        this, tr("Remove playlists"),
        tr("You are about to remove %1 playlists from your favorites, are you "
           "sure?").arg(ids.count()),
        QMessageBox::Yes, QMessageBox::Cancel);

    if (button != QMessageBox::Yes) {
      return;
    }
  }

  // Unfavorite the playlists
  for (int id : ids) {
    app_->playlist_manager()->Favorite(id, false);
  }

  // Delete the top-level folders.
  for (const QPersistentModelIndex& index : folders_to_delete) {
    if (index.isValid()) {
      model_->removeRow(index.row(), index.parent());
    }
  }
}

void PlaylistListContainer::RecursivelyFindPlaylists(const QModelIndex& parent,
                                                     QSet<int>* ids) const {
  switch (parent.data(PlaylistListModel::Role_Type).toInt()) {
    case PlaylistListModel::Type_Playlist:
      ids->insert(parent.data(PlaylistListModel::Role_PlaylistId).toInt());
      break;

    case PlaylistListModel::Type_Folder:
      for (int i = 0; i < parent.model()->rowCount(parent); ++i) {
        RecursivelyFindPlaylists(parent.child(i, 0), ids);
      }
      break;
  }
}

void PlaylistListContainer::contextMenuEvent(QContextMenuEvent* e) {
  if (!menu_) {
    menu_ = new QMenu(this);
    menu_->addAction(action_new_folder_);
    menu_->addAction(action_remove_);
    menu_->addSeparator();
    menu_->addAction(action_save_playlist_);
  }
  menu_->popup(e->globalPos());
}

void PlaylistListContainer::ActivePlaying() {
  if (padded_play_icon_.isNull()) {
    QPixmap pixmap(":tiny-start.png");
    QPixmap new_pixmap(QSize(pixmap.height(), pixmap.height()));
    new_pixmap.fill(Qt::transparent);

    QPainter p(&new_pixmap);
    p.drawPixmap((new_pixmap.width() - pixmap.width()) / 2, 0, pixmap.width(),
                 pixmap.height(), pixmap);
    p.end();

    padded_play_icon_.addPixmap(new_pixmap);
  }
  UpdateActiveIcon(active_playlist_id_, padded_play_icon_);
}

void PlaylistListContainer::ActivePaused() {
  UpdateActiveIcon(active_playlist_id_, QIcon(":tiny-pause.png"));
}

void PlaylistListContainer::ActiveStopped() {
  UpdateActiveIcon(active_playlist_id_, QIcon());
}

void PlaylistListContainer::UpdateActiveIcon(int id, const QIcon& icon) {
  if (id == -1) {
    return;
  }

  QStandardItem* item = model_->PlaylistById(id);
  if (!item) {
    return;
  }

  if (icon.isNull()) {
    item->setIcon(model_->playlist_icon());
  } else {
    item->setIcon(icon);
  }
}
