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
#include "playlist/playlist.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

typedef QFuture<PlaylistItemList> Future;
typedef QFutureWatcher<PlaylistItemList> FutureWatcher;

SmartPlaylistSearchPreview::SmartPlaylistSearchPreview(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_SmartPlaylistSearchPreview),
    model_(NULL)
{
  ui_->setupUi(this);

  // Prevent editing songs and saving settings (like header columns and geometry)
  ui_->tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui_->tree->SetReadOnlySettings(true);

  QFont bold_font;
  bold_font.setBold(true);
  ui_->preview_label->setFont(bold_font);
  ui_->busy_container->hide();
}

SmartPlaylistSearchPreview::~SmartPlaylistSearchPreview() {
  delete ui_;
}

void SmartPlaylistSearchPreview::set_library(LibraryBackend* backend) {
  backend_ = backend;

  model_ = new Playlist(NULL, NULL, backend_, -1, this);
  ui_->tree->setModel(model_);
  ui_->tree->SetPlaylist(model_);
  ui_->tree->SetItemDelegates(backend_);
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

  PlaylistItemList all_items = watcher->result();
  PlaylistItemList displayed_items = all_items.mid(0, PlaylistGenerator::kDefaultLimit);

  model_->Clear();
  model_->InsertItems(displayed_items);

  if (displayed_items.count() < all_items.count()) {
    ui_->count_label->setText(tr("%1 songs found (showing %2)")
        .arg(all_items.count()).arg(displayed_items.count()));
  } else {
    ui_->count_label->setText(tr("%1 songs found").arg(all_items.count()));
  }

  ui_->busy_container->hide();
  ui_->count_label->show();
}
