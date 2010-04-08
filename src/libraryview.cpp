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

#include "library.h"
#include "libraryview.h"
#include "libraryitem.h"

#include <QPainter>
#include <QContextMenuEvent>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QSettings>

const int LibraryView::kRowsToShow = 50;
const char* LibraryView::kSettingsGroup = "LibraryView";

LibraryItemDelegate::LibraryItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
}

void LibraryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const {
  LibraryItem::Type type =
      static_cast<LibraryItem::Type>(index.data(Library::Role_Type).toInt());

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
  : QTreeView(parent),
    library_(NULL),
    total_song_count_(-1),
    auto_open_(true),
    nomusic_(":nomusic.png"),
    context_menu_(new QMenu(this))
{
  setItemDelegate(new LibraryItemDelegate(this));

  connect(this, SIGNAL(expanded(QModelIndex)), SLOT(ItemExpanded(QModelIndex)));

  add_to_playlist_ = context_menu_->addAction(
      tr("Add to playlist"), this, SLOT(AddToPlaylist()));
  context_menu_->addSeparator();
  show_in_various_ = context_menu_->addAction(
      tr("Show in various artists"), this, SLOT(ShowInVarious()));
  no_show_in_various_ = context_menu_->addAction(
      tr("Don't show in various artists"), this, SLOT(NoShowInVarious()));

  ReloadSettings();
}

void LibraryView::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  auto_open_ = s.value("auto_open", true).toBool();
}

void LibraryView::SetLibrary(Library *library) {
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

void LibraryView::reset() {
  QTreeView::reset();

  // Expand nodes in the tree until we have about 50 rows visible in the view
  if (auto_open_) {
    int rows = model()->rowCount(rootIndex());
    RecursivelyExpand(rootIndex(), &rows);
  }
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

bool LibraryView::RecursivelyExpand(const QModelIndex& index, int* count) {
  int children = model()->rowCount(index);
  if (*count + children > kRowsToShow)
    return false;

  expand(index);
  *count += children;

  for (int i=0 ; i<children ; ++i) {
    if (!RecursivelyExpand(model()->index(i, 0, index), count))
      return false;
  }

  return true;
}

void LibraryView::ItemExpanded(const QModelIndex& index) {
  if (model()->rowCount(index) == 1 && auto_open_)
    expand(model()->index(0, 0, index));
}

void LibraryView::contextMenuEvent(QContextMenuEvent *e) {
  context_menu_index_ = indexAt(e->pos());
  if (!context_menu_index_.isValid())
    return;

  context_menu_index_ = qobject_cast<QSortFilterProxyModel*>(model())
                        ->mapToSource(context_menu_index_);

  int type = library_->data(context_menu_index_, Library::Role_Type).toInt();
  int container_type = library_->data(context_menu_index_, Library::Role_ContainerType).toInt();
  bool enable_various = container_type == Library::GroupBy_Album;
  bool enable_add = type == LibraryItem::Type_Container ||
                    type == LibraryItem::Type_Song;

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

  QString artist = library_->data(context_menu_index_, Library::Role_Artist).toString();
  QString album = library_->data(context_menu_index_, Library::Role_Key).toString();
  library_->GetBackend()->ForceCompilation(artist, album, on);
}

void LibraryView::AddToPlaylist() {
  if (!context_menu_index_.isValid())
    return;

  emit AddToPlaylist(context_menu_index_);
}
