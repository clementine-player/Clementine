class PlaylistManagerInterface : QObject {

%TypeHeaderCode
#include "playlist/playlistmanager.h"
%End

public:
  int current_id() const;
  int active_id() const;

  Playlist* playlist(int id) const;
  Playlist* current() const;
  Playlist* active() const;

  const QList<Playlist*> GetAllPlaylists() const;

  const QItemSelection& selection(int id) const;
  const QItemSelection& current_selection() const;
  const QItemSelection& active_selection() const;

  QString name(int index) const;

  LibraryBackend* library_backend() const;
  // PlaylistBackend* playlist_backend() const;
  PlaylistSequence* sequence() const;
  PlaylistParser* parser() const;

public slots:
  void New(const QString& name, const SongList& songs = SongList());
  void Load(const QString& filename);
  void Save(int id, const QString& filename);
  void Rename(int id, const QString& new_name);
  void Remove(int id);
  void ChangePlaylistOrder(const QList<int>& ids);

  void SetCurrentPlaylist(int id);
  void SetActivePlaylist(int id);
  void SetActiveToCurrent();

  void SelectionChanged(const QItemSelection& selection);

  // Convenience slots that defer to either current() or active()
  void ClearCurrent();
  void ShuffleCurrent();
  void SetActivePlaying();
  void SetActivePaused();
  void SetActiveStopped();
  void SetActiveStreamMetadata(const QUrl& url, const Song& song);
  // Rate current song using 0.0 - 1.0 scale.
  void RateCurrentSong(double rating);

  // void PlaySmartPlaylist(smart_playlists::GeneratorPtr generator, bool as_new, bool clear);

signals:
  void PlaylistManagerInitialized();

  void PlaylistAdded(int id, const QString& name);
  void PlaylistRemoved(int id);
  void PlaylistRenamed(int id, const QString& new_name);
  void CurrentChanged(Playlist* new_playlist);
  void ActiveChanged(Playlist* new_playlist);

  void Error(const QString& message);
  void SummaryTextChanged(const QString& summary);

  // Forwarded from individual playlists
  void CurrentSongChanged(const Song& song);

  // Signals that one of manager's playlists has changed (new items, new
  // ordering etc.) - the argument shows which.
  void PlaylistChanged(Playlist* playlist);
  void EditingFinished(const QModelIndex& index);
  void PlayRequested(const QModelIndex& index);

private:
  PlaylistManagerInterface();
};
