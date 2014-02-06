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

#include "searchpreview.h"
#include "ui_searchpreview.h"

#include <memory>

#include <QFutureWatcher>
#include <QtConcurrentRun>

#include "querygenerator.h"
#include "playlist/playlist.h"

namespace smart_playlists {

typedef QFuture<PlaylistItemList> Future;
typedef QFutureWatcher<PlaylistItemList> FutureWatcher;

SearchPreview::SearchPreview(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_SmartPlaylistSearchPreview),
    model_(nullptr)
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

SearchPreview::~SearchPreview() {
  delete ui_;
}

void SearchPreview::set_application(Application* app) {
  ui_->tree->SetApplication(app);
}

void SearchPreview::set_library(LibraryBackend* backend) {
  backend_ = backend;

  model_ = new Playlist(nullptr, NULL, backend_, -1, QString(), false, this);
  ui_->tree->setModel(model_);
  ui_->tree->SetPlaylist(model_);
  ui_->tree->SetItemDelegates(backend_);
}

void SearchPreview::Update(const Search& search) {
  if (search == last_search_) {
    // This search was the same as the last one we did
    return;
  }

  if (generator_ || isHidden()) {
    // It's busy generating something already, or the widget isn't visible
    pending_search_ = search;
    return;
  }

  RunSearch(search);
}

void SearchPreview::showEvent(QShowEvent* e) {
  if (pending_search_.is_valid() && !generator_) {
    // There was a search waiting while we were hidden, so run it now
    RunSearch(pending_search_);
    pending_search_ = Search();
  }

  QWidget::showEvent(e);
}

PlaylistItemList DoRunSearch(GeneratorPtr gen) {
  return gen->Generate();
}

void SearchPreview::RunSearch(const Search& search) {
  generator_.reset(new QueryGenerator);
  generator_->set_library(backend_);
  std::dynamic_pointer_cast<QueryGenerator>(generator_)->Load(search);

  ui_->busy_container->show();
  ui_->count_label->hide();
  Future future = QtConcurrent::run(DoRunSearch, generator_);

  FutureWatcher* watcher = new FutureWatcher(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(SearchFinished()));
}

void SearchPreview::SearchFinished() {
  FutureWatcher* watcher = static_cast<FutureWatcher*>(sender());
  watcher->deleteLater();

  last_search_ = std::dynamic_pointer_cast<QueryGenerator>(generator_)->search();
  generator_.reset();

  if (pending_search_.is_valid() && pending_search_ != last_search_) {
    // There was another search done while we were running - throw away these
    // results and do that one now instead
    RunSearch(pending_search_);
    pending_search_ = Search();
    return;
  }

  PlaylistItemList all_items = watcher->result();
  PlaylistItemList displayed_items = all_items.mid(0, Generator::kDefaultLimit);

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

} // namespace
