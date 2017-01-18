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

#include "playlistbackend.h"
#include "playlistcontainer.h"
#include "playlistmanager.h"
#include "playlistsaveoptionsdialog.h"
#include "playlistview.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/songloader.h"
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "library/libraryplaylistitem.h"
#include "playlistparsers/playlistparser.h"
#include "smartplaylists/generator.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QFuture>
#include <QMessageBox>
#include <QtConcurrentRun>
#include <QtDebug>

using smart_playlists::GeneratorPtr;

PlaylistManager::PlaylistManager(Application* app, QObject* parent)
    : PlaylistManagerInterface(app, parent),
      app_(app),
      playlist_backend_(nullptr),
      library_backend_(nullptr),
      sequence_(nullptr),
      parser_(nullptr),
      playlist_container_(nullptr),
      current_(-1),
      active_(-1) {
  connect(app_->player(), SIGNAL(Paused()), SLOT(SetActivePaused()));
  connect(app_->player(), SIGNAL(Playing()), SLOT(SetActivePlaying()));
  connect(app_->player(), SIGNAL(Stopped()), SLOT(SetActiveStopped()));
}

PlaylistManager::~PlaylistManager() {
  for (const Data& data : playlists_.values()) {
    delete data.p;
  }
}

void PlaylistManager::Init(LibraryBackend* library_backend,
                           PlaylistBackend* playlist_backend,
                           PlaylistSequence* sequence,
                           PlaylistContainer* playlist_container) {
  library_backend_ = library_backend;
  playlist_backend_ = playlist_backend;
  sequence_ = sequence;
  parser_ = new PlaylistParser(library_backend, this);
  playlist_container_ = playlist_container;

  connect(library_backend_, SIGNAL(SongsDiscovered(SongList)),
          SLOT(SongsDiscovered(SongList)));
  connect(library_backend_, SIGNAL(SongsStatisticsChanged(SongList)),
          SLOT(SongsDiscovered(SongList)));
  connect(library_backend_, SIGNAL(SongsRatingChanged(SongList)),
          SLOT(SongsDiscovered(SongList)));

  for (const PlaylistBackend::Playlist& p :
       playlist_backend->GetAllOpenPlaylists()) {
    AddPlaylist(p.id, p.name, p.special_type, p.ui_path, p.favorite);
  }

  // If no playlist exists then make a new one
  if (playlists_.isEmpty()) New(tr("Playlist"));

  emit PlaylistManagerInitialized();
}

QList<Playlist*> PlaylistManager::GetAllPlaylists() const {
  QList<Playlist*> result;

  for (const Data& data : playlists_.values()) {
    result.append(data.p);
  }

  return result;
}

QItemSelection PlaylistManager::selection(int id) const {
  QMap<int, Data>::const_iterator it = playlists_.find(id);
  return it->selection;
}

Playlist* PlaylistManager::AddPlaylist(int id, const QString& name,
                                       const QString& special_type,
                                       const QString& ui_path, bool favorite) {
  Playlist* ret = new Playlist(playlist_backend_, app_->task_manager(),
                               library_backend_, id, special_type, favorite);
  ret->set_sequence(sequence_);
  ret->set_ui_path(ui_path);

  connect(ret, SIGNAL(CurrentSongChanged(Song)),
          SIGNAL(CurrentSongChanged(Song)));
  connect(ret, SIGNAL(PlaylistChanged()), SLOT(OneOfPlaylistsChanged()));
  connect(ret, SIGNAL(PlaylistChanged()), SLOT(UpdateSummaryText()));
  connect(ret, SIGNAL(EditingFinished(QModelIndex)),
          SIGNAL(EditingFinished(QModelIndex)));
  connect(ret, SIGNAL(Error(QString)), SIGNAL(Error(QString)));
  connect(ret, SIGNAL(PlayRequested(QModelIndex)),
          SIGNAL(PlayRequested(QModelIndex)));
  connect(playlist_container_->view(),
          SIGNAL(ColumnAlignmentChanged(ColumnAlignmentMap)), ret,
          SLOT(SetColumnAlignment(ColumnAlignmentMap)));

  playlists_[id] = Data(ret, name);

  emit PlaylistAdded(id, name, favorite);

  if (current_ == -1) {
    SetCurrentPlaylist(id);
  }
  if (active_ == -1) {
    SetActivePlaylist(id);
  }

  return ret;
}

void PlaylistManager::New(const QString& name, const SongList& songs,
                          const QString& special_type) {
  if (name.isNull()) return;

  int id = playlist_backend_->CreatePlaylist(name, special_type);

  if (id == -1) qFatal("Couldn't create playlist");

  Playlist* playlist = AddPlaylist(id, name, special_type, QString(), false);
  playlist->InsertSongsOrLibraryItems(songs);

  SetCurrentPlaylist(id);

  // If the name is just "Playlist", append the id
  if (name == tr("Playlist")) {
    Rename(id, QString("%1 %2").arg(name).arg(id));
  }
}

void PlaylistManager::Load(const QString& filename) {
  QFileInfo info(filename);

  int id = playlist_backend_->CreatePlaylist(info.baseName(), QString());

  if (id == -1) {
    emit Error(tr("Couldn't create playlist"));
    return;
  }

  Playlist* playlist =
      AddPlaylist(id, info.baseName(), QString(), QString(), false);

  QList<QUrl> urls;
  playlist->InsertUrls(urls << QUrl::fromLocalFile(filename));
}

void PlaylistManager::Save(int id, const QString& filename,
                           Playlist::Path path_type) {
  if (playlists_.contains(id)) {
    parser_->Save(playlist(id)->GetAllSongs(), filename, path_type);
  } else {
    // Playlist is not in the playlist manager: probably save action was
    // triggered
    // from the left side bar and the playlist isn't loaded.
    QFuture<QList<Song>> future = QtConcurrent::run(
        playlist_backend_, &PlaylistBackend::GetPlaylistSongs, id);
    NewClosure(future, this, SLOT(ItemsLoadedForSavePlaylist(
                                 QFuture<SongList>, QString, Playlist::Path)),
               future, filename, path_type);
  }
}

void PlaylistManager::ItemsLoadedForSavePlaylist(QFuture<SongList> future,
                                                 const QString& filename,
                                                 Playlist::Path path_type) {
  parser_->Save(future.result(), filename, path_type);
}

void PlaylistManager::SaveWithUI(int id, const QString& suggested_filename) {
  QSettings settings;
  settings.beginGroup(Playlist::kSettingsGroup);
  QString filename = settings.value("last_save_playlist").toString();
  QString extension = settings.value("last_save_extension",
                                     parser()->default_extension()).toString();
  QString filter =
      settings.value("last_save_filter", parser()->default_filter()).toString();

  qLog(Debug) << "Using extension:" << extension;

  // We want to use the playlist tab name as a default filename, but in the
  // same directory as the last saved file.

  // Strip off filename components until we find something that's a folder
  forever {
    QFileInfo fileinfo(filename);
    if (filename.isEmpty() || fileinfo.isDir()) break;

    filename = filename.section('/', 0, -2);
  }

  // Use the home directory as a fallback in case the path is empty.
  if (filename.isEmpty()) filename = QDir::homePath();

  // Add the suggested filename
  filename += "/" + suggested_filename + "." + extension;
  qLog(Debug) << "Suggested filename:" << filename;

  filename = QFileDialog::getSaveFileName(
      nullptr, tr("Save playlist", "Title of the playlist save dialog."),
      filename, parser()->filters(), &filter);

  if (filename.isNull()) {
    return;
  }

  // Check if the file extension is valid. Fallback to the default if not.
  QFileInfo info(filename);
  ParserBase* parser = parser_->ParserForExtension(info.suffix());
  if (!parser) {
      qLog(Warning) << "Unknown file extension:" << info.suffix();
      filename = info.absolutePath() + "/" + info.fileName()
        + "." + parser_->default_extension();
      info.setFile(filename);
      filter = info.suffix();
  }

  int p = settings.value(Playlist::kPathType, Playlist::Path_Automatic).toInt();
  Playlist::Path path = static_cast<Playlist::Path>(p);
  if (path == Playlist::Path_Ask_User) {
    PlaylistSaveOptionsDialog optionsDialog(nullptr);
    optionsDialog.setModal(true);
    if (optionsDialog.exec() != QDialog::Accepted) {
      return;
    }
    path = optionsDialog.path_type();
  }

  settings.setValue("last_save_playlist", filename);
  settings.setValue("last_save_filter", filter);
  settings.setValue("last_save_extension", info.suffix());

  Save(id == -1 ? current_id() : id, filename, path);
}

void PlaylistManager::Rename(int id, const QString& new_name) {
  Q_ASSERT(playlists_.contains(id));

  playlist_backend_->RenamePlaylist(id, new_name);
  playlists_[id].name = new_name;

  emit PlaylistRenamed(id, new_name);
}

void PlaylistManager::Favorite(int id, bool favorite) {
  if (playlists_.contains(id)) {
    // If playlists_ contains this playlist, its means it's opened: star or
    // unstar it.
    playlist_backend_->FavoritePlaylist(id, favorite);
    playlists_[id].p->set_favorite(favorite);
  } else {
    Q_ASSERT(!favorite);
    // Otherwise it means user wants to remove this playlist from the left
    // panel,
    // while it's not visible in the playlist tabbar either, because it has been
    // closed: delete it.
    playlist_backend_->RemovePlaylist(id);
  }
  emit PlaylistFavorited(id, favorite);
}

bool PlaylistManager::Close(int id) {
  // Won't allow removing the last playlist
  if (playlists_.count() <= 1 || !playlists_.contains(id)) return false;

  int next_id = -1;
  for (int possible_next_id : playlists_.keys()) {
    if (possible_next_id != id) {
      next_id = possible_next_id;
      break;
    }
  }
  if (next_id == -1) return false;

  if (id == active_) SetActivePlaylist(next_id);
  if (id == current_) SetCurrentPlaylist(next_id);

  Data data = playlists_.take(id);
  emit PlaylistClosed(id);

  if (!data.p->is_favorite()) {
    playlist_backend_->RemovePlaylist(id);
    emit PlaylistDeleted(id);
  }
  delete data.p;

  return true;
}

void PlaylistManager::Delete(int id) {
  if (!Close(id)) {
    return;
  }

  playlist_backend_->RemovePlaylist(id);
  emit PlaylistDeleted(id);
}

void PlaylistManager::OneOfPlaylistsChanged() {
  emit PlaylistChanged(qobject_cast<Playlist*>(sender()));
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
  if (active_ != -1 && active_ != id) active()->set_current_row(-1);

  active_ = id;
  emit ActiveChanged(active());

  sequence_->SetUsingDynamicPlaylist(active()->is_dynamic());
}

void PlaylistManager::SetActiveToCurrent() {
  // Check if we need to update the active playlist.
  // By calling SetActiveToCurrent, the playlist manager emits the signal
  // "ActiveChanged". This signal causes the network remote module to
  // send all playlists to the clients, even no change happend.
  if (current_id() != active_id()) {
    SetActivePlaylist(current_id());
  }
}

void PlaylistManager::ClearCurrent() { current()->Clear(); }

void PlaylistManager::ShuffleCurrent() { current()->Shuffle(); }

void PlaylistManager::RemoveDuplicatesCurrent() {
  current()->RemoveDuplicateSongs();
}

void PlaylistManager::RemoveUnavailableCurrent() {
  current()->RemoveUnavailableSongs();
}

void PlaylistManager::SetActivePlaying() { active()->Playing(); }

void PlaylistManager::SetActivePaused() { active()->Paused(); }

void PlaylistManager::SetActiveStopped() { active()->Stopped(); }

void PlaylistManager::SetActiveStreamMetadata(const QUrl& url,
                                              const Song& song) {
  active()->SetStreamMetadata(url, song);
}

void PlaylistManager::RateCurrentSong(double rating) {
  active()->RateSong(active()->current_index(), rating);
}

void PlaylistManager::RateCurrentSong(int rating) {
  RateCurrentSong(rating / 5.0);
}

void PlaylistManager::ChangePlaylistOrder(const QList<int>& ids) {
  playlist_backend_->SetPlaylistOrder(ids);
}

void PlaylistManager::UpdateSummaryText() {
  int tracks = current()->rowCount();
  quint64 nanoseconds = 0;
  int selected = 0;

  // Get the length of the selected tracks
  for (const QItemSelectionRange& range : playlists_[current_id()].selection) {
    if (!range.isValid()) continue;

    selected += range.bottom() - range.top() + 1;
    for (int i = range.top(); i <= range.bottom(); ++i) {
      qint64 length =
          range.model()->index(i, Playlist::Column_Length).data().toLongLong();
      if (length > 0) nanoseconds += length;
    }
  }

  QString summary;
  if (selected > 1) {
    summary += tr("%1 selected of").arg(selected) + " ";
  } else {
    nanoseconds = current()->GetTotalLength();
  }

  // TODO: Make the plurals translatable
  summary += tracks == 1 ? tr("1 track") : tr("%1 tracks").arg(tracks);

  if (nanoseconds)
    summary += " - [ " + Utilities::WordyTimeNanosec(nanoseconds) + " ]";

  emit SummaryTextChanged(summary);
}

void PlaylistManager::SelectionChanged(const QItemSelection& selection) {
  playlists_[current_id()].selection = selection;
  UpdateSummaryText();
}

void PlaylistManager::SongsDiscovered(const SongList& songs) {
  // Some songs might've changed in the library, let's update any playlist
  // items we have that match those songs

  for (const Song& song : songs) {
    for (const Data& data : playlists_) {
      PlaylistItemList items = data.p->library_items_by_id(song.id());
      for (PlaylistItemPtr item : items) {
        if (item->Metadata().directory_id() != song.directory_id()) continue;
        static_cast<LibraryPlaylistItem*>(item.get())->SetMetadata(song);
        data.p->ItemChanged(item);
      }
    }
  }
}

void PlaylistManager::PlaySmartPlaylist(GeneratorPtr generator, bool as_new,
                                        bool clear) {
  if (as_new) {
    New(generator->name());
  }

  if (clear) {
    current()->Clear();
  }

  current()->InsertSmartPlaylist(generator);
}

// When Player has processed the new song chosen by the user...
void PlaylistManager::SongChangeRequestProcessed(const QUrl& url, bool valid) {
  for (Playlist* playlist : GetAllPlaylists()) {
    if (playlist->ApplyValidityOnCurrentSong(url, valid)) {
      return;
    }
  }
}

void PlaylistManager::InsertUrls(int id, const QList<QUrl>& urls, int pos,
                                 bool play_now, bool enqueue) {
  Q_ASSERT(playlists_.contains(id));

  playlists_[id].p->InsertUrls(urls, pos, play_now, enqueue);
}

void PlaylistManager::InsertSongs(int id, const SongList& songs, int pos,
                                  bool play_now, bool enqueue) {
  Q_ASSERT(playlists_.contains(id));

  playlists_[id].p->InsertSongs(songs, pos, play_now, enqueue);
}

void PlaylistManager::RemoveItemsWithoutUndo(int id,
                                             const QList<int>& indices) {
  Q_ASSERT(playlists_.contains(id));

  playlists_[id].p->RemoveItemsWithoutUndo(indices);
}

void PlaylistManager::InvalidateDeletedSongs() {
  for (Playlist* playlist : GetAllPlaylists()) {
    playlist->InvalidateDeletedSongs();
  }
}

void PlaylistManager::RemoveDeletedSongs() {
  for (Playlist* playlist : GetAllPlaylists()) {
    playlist->RemoveDeletedSongs();
  }
}

QString PlaylistManager::GetNameForNewPlaylist(const SongList& songs) {
  if (songs.isEmpty()) {
    return tr("Playlist");
  }

  QSet<QString> artists;
  QSet<QString> albums;

  for (const Song& song : songs) {
    artists << (song.artist().isEmpty() ? tr("Unknown") : song.artist());
    albums << (song.album().isEmpty() ? tr("Unknown") : song.album());

    if (artists.size() > 1) {
      break;
    }
  }

  bool various_artists = artists.size() > 1;

  QString result;
  if (various_artists) {
    result = tr("Various artists");
  } else {
    result = artists.values().first();
  }

  if (!various_artists && albums.size() == 1) {
    result += " - " + albums.toList().first();
  }

  return result;
}

void PlaylistManager::Open(int id) {
  if (playlists_.contains(id)) {
    return;
  }

  const PlaylistBackend::Playlist& p = playlist_backend_->GetPlaylist(id);
  if (p.id != id) {
    return;
  }

  AddPlaylist(p.id, p.name, p.special_type, p.ui_path, p.favorite);
}

void PlaylistManager::SetCurrentOrOpen(int id) {
  Open(id);
  SetCurrentPlaylist(id);
}

bool PlaylistManager::IsPlaylistOpen(int id) { return playlists_.contains(id); }
