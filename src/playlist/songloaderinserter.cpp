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

#include "playlist.h"
#include "songloaderinserter.h"
#include "core/songloader.h"

SongLoaderInserter::SongLoaderInserter(QObject *parent)
  : QObject(parent),
    destination_(NULL),
    row_(-1),
    play_now_(true)
{
}

SongLoaderInserter::~SongLoaderInserter() {
  qDeleteAll(pending_);
}

void SongLoaderInserter::Load(Playlist *destination, int row, bool play_now,
                              const QList<QUrl> &urls) {
  destination_ = destination;
  row_ = row;
  play_now_ = play_now;

  foreach (const QUrl& url, urls) {
    SongLoader* loader = new SongLoader(this);
    SongLoader::Result ret = loader->Load(url);

    if (ret == SongLoader::WillLoadAsync) {
      pending_.insert(loader);
      connect(loader, SIGNAL(LoadFinished(bool)), SLOT(PendingLoadFinished(bool)));
      continue;
    }

    if (ret == SongLoader::Success)
      songs_ << loader->songs();
    else
      emit Error(tr("Error loading %1").arg(url.toString()));
    delete loader;
  }

  if (pending_.isEmpty())
    Finished();
  else
    emit AsyncLoadStarted();
}

void SongLoaderInserter::PendingLoadFinished(bool success) {
  SongLoader* loader = qobject_cast<SongLoader*>(sender());
  if (!loader || !pending_.contains(loader))
    return;
  pending_.remove(loader);

  if (success)
    songs_ << loader->songs();
  else
    emit Error(tr("Error loading %1").arg(loader->url().toString()));

  loader->deleteLater();

  if (pending_.isEmpty()) {
    emit AsyncLoadFinished();
    Finished();
  }
}

void SongLoaderInserter::Finished() {
  QModelIndex index = destination_->InsertSongs(songs_, row_);
  if (play_now_)
    emit PlayRequested(index);

  deleteLater();
}
