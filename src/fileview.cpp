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

#include "fileview.h"

#include <QFileSystemModel>
#include <QUndoStack>

FileView::FileView(QWidget* parent)
    : QWidget(parent),
      model_(new QFileSystemModel(this)),
      undo_stack_(new QUndoStack(this))
{
  ui_.setupUi(this);

  ui_.list->setModel(model_);
  ChangeFilePathWithoutUndo(QDir::homePath());

  connect(ui_.back, SIGNAL(clicked()), SLOT(FileBack()));
  connect(ui_.forward, SIGNAL(clicked()), SLOT(FileForward()));
  connect(ui_.home, SIGNAL(clicked()), SLOT(FileHome()));
  connect(ui_.up, SIGNAL(clicked()), SLOT(FileUp()));
  connect(ui_.path, SIGNAL(textChanged(QString)), SLOT(ChangeFilePath(QString)));

  connect(undo_stack_, SIGNAL(canUndoChanged(bool)), ui_.back, SLOT(setEnabled(bool)));
  connect(undo_stack_, SIGNAL(canRedoChanged(bool)), ui_.forward, SLOT(setEnabled(bool)));

  connect(ui_.list, SIGNAL(activated(QModelIndex)), SLOT(ItemActivated(QModelIndex)));
  connect(ui_.list, SIGNAL(doubleClicked(QModelIndex)), SLOT(ItemDoubleClick(QModelIndex)));
  connect(ui_.list, SIGNAL(AddToPlaylist(QList<QUrl>)), SIGNAL(Queue(QList<QUrl>)));
  connect(ui_.list, SIGNAL(CopyToLibrary(QList<QUrl>)), SIGNAL(CopyToLibrary(QList<QUrl>)));
  connect(ui_.list, SIGNAL(MoveToLibrary(QList<QUrl>)), SIGNAL(MoveToLibrary(QList<QUrl>)));
}

void FileView::SetPath(const QString& path) {
  ChangeFilePathWithoutUndo(path);
}

void FileView::FileUp() {
  QDir dir(model_->rootDirectory());
  dir.cdUp();

  ChangeFilePath(dir.path());
}

void FileView::FileBack() {
  QString new_path(undo_stack_->command(undo_stack_->index()-1)->text());
  undo_stack_->undo();
  ChangeFilePathWithoutUndo(new_path);
}

void FileView::FileForward() {
  QString new_path(undo_stack_->command(undo_stack_->index()+1)->text());
  undo_stack_->redo();
  ChangeFilePathWithoutUndo(new_path);
}

void FileView::FileHome() {
  ChangeFilePath(QDir::homePath());
}

void FileView::ChangeFilePath(const QString& new_path) {
  QFileInfo info(new_path);
  if (!info.exists() || !info.isDir())
    return;

  QString old_path(model_->rootPath());

  ChangeFilePathWithoutUndo(new_path);
  undo_stack_->push(new QUndoCommand(old_path));
}

void FileView::ChangeFilePathWithoutUndo(const QString& new_path) {
  ui_.list->setRootIndex(model_->setRootPath(new_path));
  ui_.path->setText(new_path);

  QDir dir(new_path);
  ui_.up->setEnabled(dir.cdUp());

  emit PathChanged(new_path);
}

void FileView::ItemActivated(const QModelIndex& index) {
  if (model_->isDir(index))
    ChangeFilePath(model_->filePath(index));
}

void FileView::ItemDoubleClick(const QModelIndex& index) {
  if (model_->isDir(index))
    return;

  emit Queue(QList<QUrl>() << QUrl::fromLocalFile(model_->filePath(index)));
}
