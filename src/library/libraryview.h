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

#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include "core/song.h"
#include "ui/edittagdialog.h"
#include "widgets/autoexpandingtreeview.h"

#include <QStyledItemDelegate>

#include <boost/scoped_ptr.hpp>

class DeviceManager;
class LibraryModel;
class OrganiseDialog;
class TaskManager;

namespace smart_playlists { class Wizard; }

class LibraryItemDelegate : public QStyledItemDelegate {
 public:
  LibraryItemDelegate(QObject* parent);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class LibraryView : public AutoExpandingTreeView {
  Q_OBJECT

 public:
  LibraryView(QWidget* parent = 0);
  ~LibraryView();

  static const char* kSettingsGroup;

  void SetTaskManager(TaskManager* task_manager);
  void SetLibrary(LibraryModel* library);
  void SetDeviceManager(DeviceManager* device_manager);

  // QTreeView
  void keyboardSearch(const QString &search);
  void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible);

 public slots:
  void TotalSongCountUpdated(int count);
  void ReloadSettings();

  void UpAndFocus();
  void DownAndFocus();
  void FilterReturnPressed();

 signals:
  void ShowConfigDialog();
  void Load(const QModelIndexList& indexes);
  void AddToPlaylist(const QModelIndexList& indexes);

 protected:
  // QWidget
  void paintEvent(QPaintEvent* event);
  void mouseReleaseEvent(QMouseEvent* e);
  void contextMenuEvent(QContextMenuEvent* e);
  void keyReleaseEvent(QKeyEvent* e);

 private slots:
  void Load();
  void AddToPlaylist();
  void Organise();
  void CopyToDevice();
  void Delete();
  void EditTracks();
  void ShowInVarious();
  void NoShowInVarious();

  void NewSmartPlaylist();
  void EditSmartPlaylist();
  void DeleteSmartPlaylist();

  void NewSmartPlaylistFinished();
  void EditSmartPlaylistFinished();

  void DeleteFinished(const SongList& songs_with_errors);

 private:
  void RecheckIsEmpty();
  void ShowInVarious(bool on);
  SongList GetSelectedSongs() const;

 private:
  LibraryModel* library_;
  DeviceManager* devices_;
  TaskManager* task_manager_;

  int total_song_count_;

  QPixmap nomusic_;

  QMenu* context_menu_;
  QModelIndex context_menu_index_;
  QAction* load_;
  QAction* add_to_playlist_;
  QAction* organise_;
  QAction* copy_to_device_;
  QAction* delete_;
  QAction* edit_track_;
  QAction* show_in_various_;
  QAction* no_show_in_various_;

  QAction* new_smart_playlist_;
  QAction* edit_smart_playlist_;
  QAction* delete_smart_playlist_;

  boost::scoped_ptr<OrganiseDialog> organise_dialog_;
  boost::scoped_ptr<EditTagDialog> edit_tag_dialog_;

  bool is_in_keyboard_search_;
};

#endif // LIBRARYVIEW_H
