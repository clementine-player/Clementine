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

#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QWidget>

#include "ui_fileview.h"

class QFileSystemModel;
class QUndoStack;

class FileView : public QWidget {
  Q_OBJECT

 public:
  FileView(QWidget* parent = 0);

  void SetPath(const QString& path);

 signals:
  void PathChanged(const QString& path);

  void Queue(const QList<QUrl>& urls);
  void CopyToLibrary(const QList<QUrl>& urls);
  void MoveToLibrary(const QList<QUrl>& urls);

 private slots:
  void FileUp();
  void FileBack();
  void FileForward();
  void FileHome();
  void ChangeFilePath(const QString& new_path);
  void ItemActivated(const QModelIndex& index);
  void ItemDoubleClick(const QModelIndex& index);

 private:
  void ChangeFilePathWithoutUndo(const QString& new_path);

 private:
  Ui::FileView ui_;

  QFileSystemModel* model_;
  QUndoStack* undo_stack_;
};

#endif // FILEVIEW_H
