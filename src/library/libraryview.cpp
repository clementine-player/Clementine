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

#include "librarydirectorymodel.h"
#include "librarymodel.h"
#include "libraryview.h"
#include "libraryitem.h"
#include "librarybackend.h"
#include "ui/iconloader.h"
#include "ui/organisedialog.h"

#include <QPainter>
#include <QContextMenuEvent>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QSettings>

const char* LibraryView::kSettingsGroup = "LibraryView";

LibraryItemDelegate::LibraryItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const {
  LibraryItem::Type type =
      static_cast<LibraryItem::Type>(index.data(LibraryModel::Role_Type).toInt());

  switch (type) {
    case LibraryItem::Type_Divider: {
      QString text(index.data().toString());

      // Draw the background
      //QStyledItemDelegate::paint(painter, opt, QModelIndex());

      painter->save();

      // Draw the text
      QFont bold_font(opt.font);
      bold_font.setBold(true);

      QRect text_rect(opt.rect);
      text_rect.setLeft(text_rect.left() + 30);

      painter->setPen(opt.palette.color(QPalette::Text));
      painter->setFont(bold_font);
      painter->drawText(text_rect, text);

      //Draw the line under the item
      QPen line_pen(opt.palette.color(QPalette::Dark));
      line_pen.setWidth(2);

      painter->setPen(line_pen);
      painter->drawLine(opt.rect.bottomLeft(), opt.rect.bottomRight());

      painter->restore();
      break;
    }

    default:
      QStyledItemDelegate::paint(painter, opt, index);
      break;
  }
}

LibraryView::LibraryView(QWidget* parent)
  : AutoExpandingTreeView(parent),
    library_(NULL),
    total_song_count_(-1),
    nomusic_(":nomusic.png"),
    context_menu_(new QMenu(this)),
    is_in_keyboard_search_(false)
{
  setItemDelegate(new LibraryItemDelegate(this));

  load_ = context_menu_->addAction(IconLoader::Load("media-playback-start"),
      tr("Load"), this, SLOT(Load()));
  add_to_playlist_ = context_menu_->addAction(IconLoader::Load("media-playback-start"),
      tr("Add to playlist"), this, SLOT(AddToPlaylist()));
  context_menu_->addSeparator();
  organise_ = context_menu_->addAction(IconLoader::Load("edit-copy"),
      tr("Organise files..."), this, SLOT(Organise()));
  delete_ = context_menu_->addAction(IconLoader::Load("edit-delete"),
      tr("Delete from disk..."), this, SLOT(Delete()));
  context_menu_->addSeparator();
  show_in_various_ = context_menu_->addAction(
      tr("Show in various artists"), this, SLOT(ShowInVarious()));
  no_show_in_various_ = context_menu_->addAction(
      tr("Don't show in various artists"), this, SLOT(NoShowInVarious()));

  delete_->setVisible(false); // TODO

  ReloadSettings();
}

LibraryView::~LibraryView() {
}

void LibraryView::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  SetAutoOpen(s.value("auto_open", true).toBool());
}

void LibraryView::SetTaskManager(TaskManager *task_manager) {
  organise_dialog_.reset(new OrganiseDialog(task_manager));
  organise_dialog_->SetDestinationModel(library_->directory_model());
}

void LibraryView::SetLibrary(LibraryModel *library) {
  library_ = library;
}

void LibraryView::TotalSongCountUpdated(int count) {
  bool old = total_song_count_;
  total_song_count_ = count;
  if (old != total_song_count_)
    update();

  if (total_song_count_ == 0)
    setCursor(Qt::PointingHandCursor);
  else
    unsetCursor();
}

void LibraryView::paintEvent(QPaintEvent* event) {
  QTreeView::paintEvent(event);
  QPainter p(viewport());

  QRect rect(viewport()->rect());
  if (total_song_count_ == 0) {
    // Draw the confused clementine
    QRect image_rect((rect.width() - nomusic_.width()) / 2, 50,
                     nomusic_.width(), nomusic_.height());
    p.drawPixmap(image_rect, nomusic_);

    // Draw the title text
    QFont bold_font;
    bold_font.setBold(true);
    p.setFont(bold_font);

    QFontMetrics metrics(bold_font);

    QRect title_rect(0, image_rect.bottom() + 20, rect.width(), metrics.height());
    p.drawText(title_rect, Qt::AlignHCenter, tr("Your library is empty!"));

    // Draw the other text
    p.setFont(QFont());

    QRect text_rect(0, title_rect.bottom() + 5, rect.width(), metrics.height());
    p.drawText(text_rect, Qt::AlignHCenter, tr("Click here to add some music"));
  }
}

void LibraryView::mouseReleaseEvent(QMouseEvent* e) {
  QTreeView::mouseReleaseEvent(e);

  if (total_song_count_ == 0) {
    emit ShowConfigDialog();
  }
}

void LibraryView::contextMenuEvent(QContextMenuEvent *e) {
  context_menu_index_ = indexAt(e->pos());
  if (!context_menu_index_.isValid())
    return;

  context_menu_index_ = qobject_cast<QSortFilterProxyModel*>(model())
                        ->mapToSource(context_menu_index_);

  int type = library_->data(context_menu_index_, LibraryModel::Role_Type).toInt();
  int container_type = library_->data(context_menu_index_, LibraryModel::Role_ContainerType).toInt();
  bool enable_various = container_type == LibraryModel::GroupBy_Album;
  bool enable_add = type == LibraryItem::Type_Container ||
                    type == LibraryItem::Type_Song;

  load_->setEnabled(enable_add);
  add_to_playlist_->setEnabled(enable_add);
  show_in_various_->setEnabled(enable_various);
  no_show_in_various_->setEnabled(enable_various);

  context_menu_->popup(e->globalPos());
}

void LibraryView::ShowInVarious() {
  ShowInVarious(true);
}

void LibraryView::NoShowInVarious() {
  ShowInVarious(false);
}

void LibraryView::ShowInVarious(bool on) {
  if (!context_menu_index_.isValid())
    return;

  QString artist = library_->data(context_menu_index_, LibraryModel::Role_Artist).toString();
  QString album = library_->data(context_menu_index_, LibraryModel::Role_Key).toString();
  library_->backend()->ForceCompilation(artist, album, on);
}

void LibraryView::Load() {
  if (!context_menu_index_.isValid())
    return;

  emit Load(selectedIndexes());
}

void LibraryView::AddToPlaylist() {
  if (!context_menu_index_.isValid())
    return;

  emit AddToPlaylist(selectedIndexes());
}

void LibraryView::keyboardSearch(const QString &search) {
  is_in_keyboard_search_ = true;
  QTreeView::keyboardSearch(search);
  is_in_keyboard_search_ = false;
}

void LibraryView::scrollTo(const QModelIndex &index, ScrollHint hint) {
  if (is_in_keyboard_search_)
    QTreeView::scrollTo(index, QAbstractItemView::PositionAtTop);
  else
    QTreeView::scrollTo(index, hint);
}

void LibraryView::Organise() {
  QModelIndexList selected_indexes =
      qobject_cast<QSortFilterProxyModel*>(model())->mapSelectionToSource(
          selectionModel()->selection()).indexes();
  SongList songs = library_->GetChildSongs(selected_indexes);
  QStringList filenames;

  foreach (const Song& song, songs) {
    filenames << song.filename();
  }

  organise_dialog_->SetCopy(false);
  organise_dialog_->SetFilenames(filenames);
  organise_dialog_->show();
}

void LibraryView::Delete() {

}
