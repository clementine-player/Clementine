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

#include "fileviewlist.h"
#include "core/mimedata.h"
#include "ui/iconloader.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QFileSystemModel>
#include <QtDebug>

FileViewList::FileViewList(QWidget* parent)
    : QListView(parent),
      menu_(new QMenu(this))
{
  menu_->addAction(IconLoader::Load("media-playback-start"), tr("Append to current playlist"),
                   this, SLOT(AddToPlaylistSlot()));
  menu_->addAction(IconLoader::Load("media-playback-start"), tr("Replace current playlist"),
                   this, SLOT(LoadSlot()));
  menu_->addSeparator();
  menu_->addAction(IconLoader::Load("edit-copy"), tr("Copy to library..."),
                   this, SLOT(CopyToLibrarySlot()));
  menu_->addAction(IconLoader::Load("go-jump"), tr("Move to library..."),
                   this, SLOT(MoveToLibrarySlot()));
  menu_->addAction(IconLoader::Load("multimedia-player-ipod-mini-blue"),
                   tr("Copy to device..."), this, SLOT(CopyToDeviceSlot()));
  menu_->addAction(IconLoader::Load("edit-delete"), tr("Delete from disk..."),
                   this, SLOT(DeleteSlot()));
  setAttribute(Qt::WA_MacShowFocusRect, false);
}

void FileViewList::contextMenuEvent(QContextMenuEvent* e) {
  menu_selection_ = selectionModel()->selection();

  menu_->popup(e->globalPos());
  e->accept();
}

QList<QUrl> FileViewList::UrlListFromSelection() const {
  QList<QUrl> urls;
  foreach (const QModelIndex& index, menu_selection_.indexes()) {
    if (index.column() == 0)
      urls << QUrl::fromLocalFile(
          static_cast<QFileSystemModel*>(model())->filePath(index));
  }
  return urls;
}

MimeData* FileViewList::MimeDataFromSelection() const {
  MimeData* data = new MimeData;
  data->setUrls(UrlListFromSelection());
  return data;
}

QStringList FileViewList::FilenamesFromSelection() const {
  QStringList filenames;
  foreach (const QModelIndex& index, menu_selection_.indexes()) {
    if (index.column() == 0)
      filenames << static_cast<QFileSystemModel*>(model())->filePath(index);
  }
  return filenames;
}

void FileViewList::LoadSlot() {
  MimeData* data = MimeDataFromSelection();
  data->clear_first_ = true;
  emit AddToPlaylist(data);
}

void FileViewList::AddToPlaylistSlot() {
  emit AddToPlaylist(MimeDataFromSelection());
}

void FileViewList::CopyToLibrarySlot() {
  emit CopyToLibrary(UrlListFromSelection());
}

void FileViewList::MoveToLibrarySlot() {
  emit MoveToLibrary(UrlListFromSelection());
}

void FileViewList::CopyToDeviceSlot() {
  emit CopyToDevice(UrlListFromSelection());
}

void FileViewList::DeleteSlot() {
  emit Delete(FilenamesFromSelection());
}
