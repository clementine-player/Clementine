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
#include "playlistbackend.h"
#include "playlistmanager.h"

PlaylistManager::PlaylistManager(QObject *parent)
  : QObject(parent),
    playlist_backend_(NULL),
    library_backend_(NULL),
    sequence_(NULL),
    current_(-1),
    active_(-1)
{
}

void PlaylistManager::Init(LibraryBackend* library_backend,
                           PlaylistBackend* playlist_backend,
                           PlaylistSequence* sequence) {
  library_backend_ = library_backend;
  playlist_backend_ = playlist_backend;
  sequence_ = sequence;

  foreach (const PlaylistBackend::Playlist& p, playlist_backend->GetAllPlaylists()) {
    AddPlaylist(p.id, p.name);
  }

  // If no playlist exists then make a new one
  if (playlists_.isEmpty())
    New(tr("Playlist"));
}

Playlist* PlaylistManager::AddPlaylist(int id, const QString& name) {
  Playlist* ret = new Playlist(playlist_backend_, id);
  ret->set_sequence(sequence_);

  connect(ret, SIGNAL(CurrentSongChanged(Song)), SIGNAL(CurrentSongChanged(Song)));
  connect(ret, SIGNAL(PlaylistChanged()), SIGNAL(PlaylistChanged()));
  connect(ret, SIGNAL(EditingFinished(QModelIndex)), SIGNAL(EditingFinished(QModelIndex)));

  playlists_ << Data(ret, name);

  int index = playlists_.count() - 1;
  emit PlaylistAdded(index, name);

  if (current_ == -1) {
    SetCurrentPlaylist(index);
  }
  if (active_ == -1) {
    SetActivePlaylist(index);
  }

  return ret;
}

void PlaylistManager::New(const QString& name) {
  int id = playlist_backend_->CreatePlaylist(name);

  if (id == -1)
    qFatal("Couldn't create playlist");

  AddPlaylist(id, name);
}

void PlaylistManager::Load(const QString& filename) {

}

void PlaylistManager::Save(int index, const QString& filename) {
  Q_ASSERT(index >= 0 && index < playlists_.count());
}

void PlaylistManager::Rename(int index, const QString& new_name) {
  Q_ASSERT(index >= 0 && index < playlists_.count());

  playlist_backend_->RenamePlaylist(playlist(index)->id(), new_name);
  playlists_[index].name = new_name;

  emit PlaylistRenamed(index, new_name);
}

void PlaylistManager::Remove(int index) {
  Q_ASSERT(index >= 0 && index < playlists_.count());

  // Won't allow removing the last playlist
  if (playlists_.count() <= 1)
    return;

  playlist_backend_->RemovePlaylist(playlist(index)->id());

  playlists_.takeAt(index);
  if (index == active_)
    SetActivePlaylist(qMin(0, index-1));
  if (index == current_)
    SetCurrentPlaylist(qMin(0, index-1));

  emit PlaylistRemoved(index);
}

void PlaylistManager::SetCurrentPlaylist(int index) {
  Q_ASSERT(index >= 0 && index < playlists_.count());
  current_ = index;
  emit CurrentChanged(current());
}

void PlaylistManager::SetActivePlaylist(int index) {
  Q_ASSERT(index >= 0 && index < playlists_.count());

  // Kinda a hack: unset the current item from the old active playlist before
  // setting the new one
  if (active_ != -1)
    active()->set_current_index(-1);

  active_ = index;
  emit ActiveChanged(current());
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
