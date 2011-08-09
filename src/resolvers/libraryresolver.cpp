#include "libraryresolver.h"

#include <QtConcurrentRun>

#include <boost/scoped_ptr.hpp>

#include "core/boundfuturewatcher.h"
#include "core/song.h"
#include "library/librarybackend.h"
#include "library/libraryquery.h"
#include "library/sqlrow.h"

using boost::scoped_ptr;

LibraryResolver::LibraryResolver(LibraryBackendInterface* backend, QObject* parent)
    : Resolver(parent),
      backend_(backend),
      next_id_(0) {
}

int LibraryResolver::ResolveSong(const Song& song) {
  LibraryQuery* query = new LibraryQuery;
  query->AddWhere("artist", song.artist());
  query->AddWhere("title", song.title());
  query->SetColumnSpec("%songs_table.ROWID, " + Song::kColumnSpec);

  QFuture<bool> future = QtConcurrent::run(
      backend_, &LibraryBackendInterface::ExecQuery, query);
  BoundFutureWatcher<bool, LibraryQuery*>* watcher =
      new BoundFutureWatcher<bool, LibraryQuery*>(query);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(QueryFinished()));

  int id = next_id_++;
  queries_[query] = id;

  return id;
}

void LibraryResolver::QueryFinished() {
  BoundFutureWatcher<bool, LibraryQuery*>* watcher =
      static_cast<BoundFutureWatcher<bool, LibraryQuery*>*>(sender());
  scoped_ptr<LibraryQuery> query(watcher->data());
  watcher->deleteLater();

  QMap<LibraryQuery*, int>::iterator it = queries_.find(query.get());
  if (it == queries_.end()) {
    return;
  }

  int id = it.value();
  queries_.erase(it);

  SongList songs;
  if (watcher->result() && query->Next()) {
    do {
      Song song;
      song.InitFromQuery(*query, true);
      songs << song;
    } while (query->Next());
  }

  emit ResolveFinished(id, songs);
}
