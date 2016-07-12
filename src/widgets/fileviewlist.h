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

#ifndef FILEVIEWLIST_H
#define FILEVIEWLIST_H

#include <QListView>
#include <QUrl>

class MimeData;

class FileViewList : public QListView {
  Q_OBJECT

 public:
  FileViewList(QWidget* parent = nullptr);

  void mousePressEvent(QMouseEvent* e);

signals:
  void AddToPlaylist(QMimeData* data);
  void CopyToLibrary(const QList<QUrl>& urls);
  void MoveToLibrary(const QList<QUrl>& urls);
  void CopyToDevice(const QList<QUrl>& urls);
  void Delete(const QStringList& filenames);
  void EditTags(const QList<QUrl>& urls);
  void Back();
  void Forward();

 protected:
  void contextMenuEvent(QContextMenuEvent* e);

 private slots:
  void LoadSlot();
  void AddToPlaylistSlot();
  void OpenInNewPlaylistSlot();
  void CopyToLibrarySlot();
  void MoveToLibrarySlot();
  void CopyToDeviceSlot();
  void DeleteSlot();
  void EditTagsSlot();
  void ShowInBrowser();

  QStringList FilenamesFromSelection() const;
  QList<QUrl> UrlListFromSelection() const;
  MimeData* MimeDataFromSelection() const;

 private:
  QMenu* menu_;
  QItemSelection menu_selection_;
};

#endif  // FILEVIEWLIST_H
