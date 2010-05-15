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

#ifndef FILEVIEWLIST_H
#define FILEVIEWLIST_H

#include <QListView>
#include <QUrl>

class FileViewList : public QListView {
  Q_OBJECT

 public:
  FileViewList(QWidget* parent = 0);

 signals:
  void Load(const QList<QUrl>& urls);
  void AddToPlaylist(const QList<QUrl>& urls);
  void CopyToLibrary(const QList<QUrl>& urls);
  void MoveToLibrary(const QList<QUrl>& urls);

 protected:
  void contextMenuEvent(QContextMenuEvent* e);

 private slots:
  void LoadSlot();
  void AddToPlaylistSlot();
  void CopyToLibrarySlot();
  void MoveToLibrarySlot();

  QList<QUrl> UrlListFromSelection() const;

 private:
  QMenu* menu_;
  QItemSelection menu_selection_;
};

#endif // FILEVIEWLIST_H
