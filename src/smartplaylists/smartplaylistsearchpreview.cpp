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

#include "queryplaylistgenerator.h"
#include "smartplaylistsearchpreview.h"
#include "ui_smartplaylistsearchpreview.h"
#include "playlist/playlistdelegates.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

typedef QFuture<PlaylistItemList> Future;
typedef QFutureWatcher<PlaylistItemList> FutureWatcher;

SmartPlaylistSearchPreview::SmartPlaylistSearchPreview(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_SmartPlaylistSearchPreview)
{
  ui_->setupUi(this);

  QFont bold_font;
  bold_font.setBold(true);
  ui_->preview_label->setFont(bold_font);
  ui_->busy_container->hide();

  fields_ << SmartPlaylistSearchTerm::Field_Artist;
  fields_ << SmartPlaylistSearchTerm::Field_Album;
  fields_ << SmartPlaylistSearchTerm::Field_Title;
  fields_ << SmartPlaylistSearchTerm::Field_Length;
  fields_ << SmartPlaylistSearchTerm::Field_PlayCount;
  fields_ << SmartPlaylistSearchTerm::Field_SkipCount;
  fields_ << SmartPlaylistSearchTerm::Field_Rating;

  QStringList column_names;
  foreach (SmartPlaylistSearchTerm::Field field, fields_) {
    column_names << SmartPlaylistSearchTerm::FieldName(field);
  }
  ui_->tree->setHeaderLabels(column_names);

  ui_->tree->setItemDelegateForColumn(0, new TextItemDelegate(this));
  ui_->tree->setItemDelegateForColumn(1, new TextItemDelegate(this));
  ui_->tree->setItemDelegateForColumn(2, new TextItemDelegate(this));
  ui_->tree->setItemDelegateForColumn(3, new LengthItemDelegate(this));
  ui_->tree->setItemDelegateForColumn(6, new RatingItemDelegate(this));
}

SmartPlaylistSearchPreview::~SmartPlaylistSearchPreview() {
  delete ui_;
}

void SmartPlaylistSearchPreview::set_library(LibraryBackend* backend) {
  backend_ = backend;
}

void SmartPlaylistSearchPreview::Update(const SmartPlaylistSearch& search) {
  if (generator_) {
    // It's busy generating something already
    pending_search_ = search;
    return;
  }

  RunSearch(search);
}

void SmartPlaylistSearchPreview::RunSearch(const SmartPlaylistSearch& search) {
  generator_.reset(new QueryPlaylistGenerator);
  generator_->set_library(backend_);
  generator_->Load(search);

  ui_->busy_container->show();
  ui_->count_label->hide();
  Future future = QtConcurrent::run(generator_.get(), &QueryPlaylistGenerator::Generate);

  FutureWatcher* watcher = new FutureWatcher(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(SearchFinished()));
}

void SmartPlaylistSearchPreview::SearchFinished() {
  FutureWatcher* watcher = static_cast<FutureWatcher*>(sender());
  watcher->deleteLater();
  generator_.reset();

  PlaylistItemList items = watcher->result();
  int count = 0;

  ui_->tree->clear();
  foreach (PlaylistItemPtr item, items) {
    if (count >= PlaylistGenerator::kDefaultLimit)
      break;
    count ++;

    QTreeWidgetItem* tree_item = new QTreeWidgetItem;
    tree_item->setData(0, Qt::DisplayRole, item->Metadata().album());
    tree_item->setData(1, Qt::DisplayRole, item->Metadata().artist());
    tree_item->setData(2, Qt::DisplayRole, item->Metadata().title());
    tree_item->setData(3, Qt::DisplayRole, item->Metadata().length());
    tree_item->setData(4, Qt::DisplayRole, item->Metadata().playcount());
    tree_item->setData(5, Qt::DisplayRole, item->Metadata().skipcount());
    tree_item->setData(6, Qt::DisplayRole, item->Metadata().rating());
    ui_->tree->addTopLevelItem(tree_item);
  }

  if (items.count() > count) {
    ui_->count_label->setText(tr("%1 songs found (showing %2)").arg(items.count()).arg(count));
  } else {
    ui_->count_label->setText(tr("%1 songs found").arg(items.count()));
  }

  ui_->busy_container->hide();
  ui_->count_label->show();
}
