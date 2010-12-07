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

#include "playlist.h"
#include "playlistbackend.h"
#include "playlistmanager.h"
#include "core/songloader.h"
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "library/libraryplaylistitem.h"
#include "playlistparsers/playlistparser.h"
#include "smartplaylists/generator.h"

#include <QFileInfo>
#include <QtDebug>

using smart_playlists::GeneratorPtr;

PlaylistManager::PlaylistManager(TaskManager* task_manager, QObject *parent)
  : QObject(parent),
    task_manager_(task_manager),
    playlist_backend_(NULL),
    library_backend_(NULL),
    sequence_(NULL),
    parser_(new PlaylistParser(this)),
    current_(-1),
    active_(-1)
{
}

PlaylistManager::~PlaylistManager() {
  foreach (const Data& data, playlists_.values()) {
    delete data.p;
  }
}

void PlaylistManager::Init(LibraryBackend* library_backend,
                           PlaylistBackend* playlist_backend,
                           PlaylistSequence* sequence) {
  library_backend_ = library_backend;
  playlist_backend_ = playlist_backend;
  sequence_ = sequence;

  connect(library_backend_, SIGNAL(SongsDiscovered(SongList)), SLOT(SongsDiscovered(SongList)));
  connect(library_backend_, SIGNAL(SongsStatisticsChanged(SongList)), SLOT(SongsDiscovered(SongList)));

  foreach (const PlaylistBackend::Playlist& p, playlist_backend->GetAllPlaylists()) {
    AddPlaylist(p.id, p.name);
  }

  // If no playlist exists then make a new one
  if (playlists_.isEmpty())
    New(tr("Playlist"));

  emit PlaylistManagerInitialized();
}

Playlist* PlaylistManager::AddPlaylist(int id, const QString& name) {
  Playlist* ret = new Playlist(playlist_backend_, task_manager_, library_backend_, id);
  ret->set_sequence(sequence_);

  connect(ret, SIGNAL(CurrentSongChanged(Song)), SIGNAL(CurrentSongChanged(Song)));
  connect(ret, SIGNAL(PlaylistChanged()), SIGNAL(PlaylistChanged()));
  connect(ret, SIGNAL(PlaylistChanged()), SLOT(UpdateSummaryText()));
  connect(ret, SIGNAL(EditingFinished(QModelIndex)), SIGNAL(EditingFinished(QModelIndex)));
  connect(ret, SIGNAL(LoadTracksError(QString)), SIGNAL(Error(QString)));
  connect(ret, SIGNAL(PlayRequested(QModelIndex)), SIGNAL(PlayRequested(QModelIndex)));

  playlists_[id] = Data(ret, name);

  emit PlaylistAdded(id, name);

  if (current_ == -1) {
    SetCurrentPlaylist(id);
  }
  if (active_ == -1) {
    SetActivePlaylist(id);
  }

  return ret;
}

void PlaylistManager::New(const QString& name, const SongList& songs) {
  int id = playlist_backend_->CreatePlaylist(name);

  if (id == -1)
    qFatal("Couldn't create playlist");

  Playlist* playlist = AddPlaylist(id, name);
  playlist->InsertSongs(songs);

  SetCurrentPlaylist(id);
}

void PlaylistManager::Load(const QString& filename) {
  QUrl url = QUrl::fromLocalFile(filename);
  SongLoader* loader = new SongLoader(library_backend_, this);
  connect(loader, SIGNAL(LoadFinished(bool)), SLOT(LoadFinished(bool)));
  SongLoader::Result result = loader->Load(url);
  QFileInfo info(filename);

  if (result == SongLoader::Error ||
      (result == SongLoader::Success && loader->songs().isEmpty())) {
    emit Error(tr("The playlist '%1' was empty or could not be loaded.").arg(
        info.completeBaseName()));
    delete loader;
    return;
  }

  if (result == SongLoader::Success) {
    New(info.baseName(), loader->songs());
    delete loader;
  }
}

void PlaylistManager::LoadFinished(bool success) {
  SongLoader* loader = qobject_cast<SongLoader*>(sender());
  loader->deleteLater();
  QString localfile = loader->url().toLocalFile();
  QFileInfo info(localfile);
  if (!success || loader->songs().isEmpty()) {
    emit Error(tr("The playlist '%1' was empty or could not be loaded.").arg(
        info.completeBaseName()));
  }

  New(info.baseName(), loader->songs());
}

void PlaylistManager::Save(int id, const QString& filename) {
  Q_ASSERT(playlists_.contains(id));

  parser_->Save(playlist(id)->GetAllSongs(), filename);
}

void PlaylistManager::Rename(int id, const QString& new_name) {
  Q_ASSERT(playlists_.contains(id));

  playlist_backend_->RenamePlaylist(id, new_name);
  playlists_[id].name = new_name;

  emit PlaylistRenamed(id, new_name);
}

void PlaylistManager::Remove(int id) {
  Q_ASSERT(playlists_.contains(id));

  // Won't allow removing the last playlist
  if (playlists_.count() <= 1)
    return;

  playlist_backend_->RemovePlaylist(id);

  int next_id = playlists_.constBegin()->p->id();

  if (id == active_)
    SetActivePlaylist(next_id);
  if (id == current_)
    SetCurrentPlaylist(next_id);

  Data data = playlists_.take(id);
  delete data.p;

  emit PlaylistRemoved(id);
}

void PlaylistManager::SetCurrentPlaylist(int id) {
  Q_ASSERT(playlists_.contains(id));
  current_ = id;
  emit CurrentChanged(current());
  UpdateSummaryText();
}

void PlaylistManager::SetActivePlaylist(int id) {
  Q_ASSERT(playlists_.contains(id));

  // Kinda a hack: unset the current item from the old active playlist before
  // setting the new one
  if (active_ != -1 && active_ != id)
    active()->set_current_index(-1);

  active_ = id;
  emit ActiveChanged(active());

  sequence_->SetUsingDynamicPlaylist(active()->is_dynamic());
}

void PlaylistManager::ClearCurrent() {
  current()->Clear();
}

void PlaylistManager::ShuffleCurrent() {
  current()->Shuffle();
}

void PlaylistManager::SetActivePlaying() {
  active()->Playing();
}

void PlaylistManager::SetActivePaused() {
  active()->Paused();
}

void PlaylistManager::SetActiveStopped() {
  active()->Stopped();
}

void PlaylistManager::SetActiveStreamMetadata(const QUrl &url, const Song &song) {
  active()->SetStreamMetadata(url, song);
}

void PlaylistManager::ChangePlaylistOrder(const QList<int>& ids) {
  playlist_backend_->SetPlaylistOrder(ids);
}

void PlaylistManager::UpdateSummaryText() {
  int tracks = current()->rowCount();
  quint64 seconds = 0;
  int selected = 0;

  // Get the length of the selected tracks
  foreach (const QItemSelectionRange& range, playlists_[current_id()].selection) {
    if (!range.isValid())
      continue;

    selected += range.bottom() - range.top() + 1;
    for (int i=range.top() ; i<=range.bottom() ; ++i) {
      int length = range.model()->index(i, Playlist::Column_Length).data().toInt();
      if (length > 0)
        seconds += length;
    }
  }

  QString summary;
  if (selected > 1) {
    summary += tr("%1 selected of").arg(selected) + " ";
  } else {
    seconds = current()->GetTotalLength();
  }

  // TODO: Make the plurals translatable
  summary += tracks == 1 ? tr("1 track") : tr("%1 tracks").arg(tracks);

  if (seconds)
    summary += " - [ " + Utilities::WordyTime(seconds) + " ]";

  emit SummaryTextChanged(summary);
}

void PlaylistManager::SelectionChanged(const QItemSelection& selection) {
  playlists_[current_id()].selection = selection;
  UpdateSummaryText();
}

void PlaylistManager::SongsDiscovered(const SongList& songs) {
  // Some songs might've changed in the library, let's update any playlist
  // items we have that match those songs

  foreach (const Song& song, songs) {
    foreach (const Data& data, playlists_) {
      PlaylistItemList items = data.p->library_items_by_id(song.id());
      foreach (PlaylistItemPtr item, items) {
        if (item->Metadata().directory_id() != song.directory_id())
          continue;
        static_cast<LibraryPlaylistItem*>(item.get())->SetMetadata(song);
        data.p->ItemChanged(item);
      }
    }
  }
}

void PlaylistManager::PlaySmartPlaylist(GeneratorPtr generator, bool as_new, bool clear) {
  if (as_new) {
    New(generator->name());
  }

  if (clear) {
    current()->Clear();
  }

  current()->InsertSmartPlaylist(generator);
}
