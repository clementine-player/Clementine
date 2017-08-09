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

#include <memory>

#include <QStyledItemDelegate>

#include "core/song.h"
#include "ui/edittagdialog.h"
#include "widgets/autoexpandingtreeview.h"

class Application;
class LibraryFilterWidget;
class OrganiseDialog;

class QMimeData;

namespace smart_playlists {
class Wizard;
}

class LibraryItemDelegate : public QStyledItemDelegate {
  Q_OBJECT

 public:
  LibraryItemDelegate(QObject* parent);
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const;

 public slots:
  bool helpEvent(QHelpEvent* event, QAbstractItemView* view,
                 const QStyleOptionViewItem& option, const QModelIndex& index);
};

class LibraryView : public AutoExpandingTreeView {
  Q_OBJECT

 public:
  LibraryView(QWidget* parent = nullptr);
  ~LibraryView();

  static const char* kSettingsGroup;

  // Returns Songs currently selected in the library view. Please note that the
  // selection is recursive meaning that if for example an album is selected
  // this will return all of it's songs.
  SongList GetSelectedSongs() const;

  void SetApplication(Application* app);
  void SetFilter(LibraryFilterWidget* filter);

  // QTreeView
  void keyboardSearch(const QString& search);
  void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible);

 public slots:
  void TotalSongCountUpdated(int count);
  void ReloadSettings();

  void FilterReturnPressed();

  void SaveFocus();
  void RestoreFocus();

signals:
  void ShowConfigDialog();

 protected:
  // QWidget
  void paintEvent(QPaintEvent* event);
  void mouseReleaseEvent(QMouseEvent* e);
  void contextMenuEvent(QContextMenuEvent* e);

 private slots:
  void Load();
  void AddToPlaylist();
  void AddToPlaylistEnqueue();
  void OpenInNewPlaylist();
  void Organise();
  void CopyToDevice();
  void Delete();
  void EditTracks();
  void ShowInBrowser();
  void ShowInVarious();
  void NoShowInVarious();

  void SearchForThis();

  void NewSmartPlaylist();
  void EditSmartPlaylist();
  void DeleteSmartPlaylist();

  void NewSmartPlaylistFinished();
  void EditSmartPlaylistFinished();

  void DeleteFinished(const SongList& songs_with_errors);

 private:
  void RecheckIsEmpty();
  void ShowInVarious(bool on);
  bool RestoreLevelFocus(const QModelIndex& parent = QModelIndex());
  void SaveContainerPath(const QModelIndex& child);

 private:
  Application* app_;
  LibraryFilterWidget* filter_;

  int total_song_count_;

  QPixmap nomusic_;

  QMenu* context_menu_;
  QModelIndex context_menu_index_;
  QAction* load_;
  QAction* add_to_playlist_;
  QAction* add_to_playlist_enqueue_;
  QAction* open_in_new_playlist_;
  QAction* organise_;
  QAction* copy_to_device_;
  QAction* delete_;
  QAction* edit_track_;
  QAction* edit_tracks_;
  QAction* show_in_browser_;
  QAction* show_in_various_;
  QAction* no_show_in_various_;

  QAction* search_for_this_;

  QAction* new_smart_playlist_;
  QAction* edit_smart_playlist_;
  QAction* delete_smart_playlist_;

  std::unique_ptr<OrganiseDialog> organise_dialog_;
  std::unique_ptr<EditTagDialog> edit_tag_dialog_;

  bool is_in_keyboard_search_;

  // Save focus
  Song last_selected_song_;
  QString last_selected_container_;
  QString last_selected_text_;
  QSet<QString> last_selected_path_;
};

#endif  // LIBRARYVIEW_H
