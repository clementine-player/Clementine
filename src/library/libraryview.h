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

#ifndef LIBRARYVIEW_H
#define LIBRARYVIEW_H

#include <QStyledItemDelegate>
#include <QTreeView>

class LibraryModel;

class LibraryItemDelegate : public QStyledItemDelegate {
 public:
  LibraryItemDelegate(QObject* parent);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

class LibraryView : public QTreeView {
  Q_OBJECT

 public:
  LibraryView(QWidget* parent = 0);

  static const char* kSettingsGroup;

  void SetLibrary(LibraryModel* library);

 public slots:
  void TotalSongCountUpdated(int count);
  void ReloadSettings();

 signals:
  void ShowConfigDialog();
  void AddToPlaylist(const QModelIndex& index);

 protected:
  // QAbstractItemView
  void reset();

  // QWidget
  void paintEvent(QPaintEvent* event);
  void mouseReleaseEvent(QMouseEvent* e);
  void contextMenuEvent(QContextMenuEvent* e);

 private slots:
  void ItemExpanded(const QModelIndex& index);
  void AddToPlaylist();
  void ShowInVarious();
  void NoShowInVarious();

 private:
  void RecheckIsEmpty();
  bool RecursivelyExpand(const QModelIndex& index, int* count);
  void ShowInVarious(bool on);

 private:
  static const int kRowsToShow;

  LibraryModel* library_;
  int total_song_count_;
  bool auto_open_;

  QPixmap nomusic_;

  QMenu* context_menu_;
  QModelIndex context_menu_index_;
  QAction* add_to_playlist_;
  QAction* show_in_various_;
  QAction* no_show_in_various_;
};

#endif // LIBRARYVIEW_H
