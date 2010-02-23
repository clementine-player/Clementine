#include "fileviewlist.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QFileSystemModel>
#include <QtDebug>

FileViewList::FileViewList(QWidget* parent)
    : QListView(parent),
      menu_(new QMenu(this))
{
  menu_->addAction(QIcon(":media-playback-start.png"), tr("Add to playlist"),
                   this, SLOT(AddToPlaylistSlot()));
  menu_->addSeparator();
  menu_->addAction(QIcon(":copy.png"), tr("Copy to library..."),
                   this, SLOT(CopyToLibrarySlot()));
  menu_->addAction(QIcon(":move.png"), tr("Move to library..."),
                   this, SLOT(MoveToLibrarySlot()));
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

void FileViewList::AddToPlaylistSlot() {
  emit AddToPlaylist(UrlListFromSelection());
}

void FileViewList::CopyToLibrarySlot() {
  emit CopyToLibrary(UrlListFromSelection());
}

void FileViewList::MoveToLibrarySlot() {
  emit MoveToLibrary(UrlListFromSelection());
}
