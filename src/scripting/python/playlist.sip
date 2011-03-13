class Playlist : QAbstractListModel {

%TypeHeaderCode
#include "playlist/playlist.h"
#include "scripting/python/pythonengine.h"
%End

public:
  enum Column {
    Column_Title,
    Column_Artist,
    Column_Album,
    Column_AlbumArtist,
    Column_Composer,
    Column_Length,
    Column_Track,
    Column_Disc,
    Column_Year,
    Column_Genre,

    Column_BPM,
    Column_Bitrate,
    Column_Samplerate,
    Column_Filename,
    Column_BaseFilename,
    Column_Filesize,
    Column_Filetype,
    Column_DateCreated,
    Column_DateModified,

    Column_Rating,
    Column_PlayCount,
    Column_SkipCount,
    Column_LastPlayed,
    Column_Score,

    Column_Comment,

    ColumnCount
  };

  enum Role {
    Role_IsCurrent,
    Role_IsPaused,
    Role_StopAfter,
    Role_QueuePosition,
    Role_CanSetRating,
  };

  static const char* kRowsMimetype;
  static const char* kPlayNowMimetype;

  static bool CompareItems(int column, Qt::SortOrder order,
                           PlaylistItemPtr a, PlaylistItemPtr b);

  static QString column_name(Column column);
  static bool column_is_editable(Column column);
  static bool set_column_value(Song& song, Column column, const QVariant& value);

  // Accessors
  QSortFilterProxyModel* proxy() const;
  Queue* queue() const;

  int id() const;

  int current_row() const;
  int last_played_row() const;
  int next_row() const;
  int previous_row() const;

  const QModelIndex current_index() const;

  bool stop_after_current() const;
  bool is_dynamic() const;

  const PlaylistItemPtr& item_at(int index) const;
  const bool has_item_at(int index) const;

  PlaylistItemPtr current_item() const;

  PlaylistItem::Options current_item_options() const;
  Song current_item_metadata() const;

  PlaylistItemList library_items_by_id(int id) const;

  SongList GetAllSongs() const;
  PlaylistItemList GetAllItems() const;
  quint64 GetTotalLength() const; // in seconds

  PlaylistSequence* sequence() const;

  QUndoStack* undo_stack() const;

  // ColumnAlignmentMap column_alignments() const { return column_alignments_; }
  // void set_column_alignments(const ColumnAlignmentMap& column_alignments);
  void set_column_align_left(int column);
  void set_column_align_center(int column);
  void set_column_align_right(int column);

  // Scrobbling
  int scrobble_point() const;
  bool has_scrobbled() const;
  void set_scrobbled(bool v);

  // Changing the playlist
  void InsertItems              (const PlaylistItemList& items,     int pos = -1, bool play_now = false, bool enqueue = false);
  void InsertLibraryItems       (const SongList& items,             int pos = -1, bool play_now = false, bool enqueue = false);
  void InsertSongs              (const SongList& items,             int pos = -1, bool play_now = false, bool enqueue = false);
  void InsertSongsOrLibraryItems(const SongList& items,             int pos = -1, bool play_now = false, bool enqueue = false);
  // void InsertSmartPlaylist      (smart_playlists::GeneratorPtr gen, int pos = -1, bool play_now = false, bool enqueue = false);
  void InsertUrls               (const QList<QUrl>& urls,           int pos = -1, bool play_now = false, bool enqueue = false);
  void RemoveItemsWithoutUndo   (const QList<int>& indicesIn);

  void StopAfter(int row);
  void ReloadItems(const QList<int>& rows);

  // Changes rating of a song to the given value asynchronously
  void RateSong(const QModelIndex& index, double rating);

  void AddSongInsertVetoListener(SongInsertVetoListener* listener /Transfer/);
%MethodCode
  sipCpp->AddSongInsertVetoListener(a0);
  PythonEngine::instance()->RegisterNativeObject(a0);
%End
  void RemoveSongInsertVetoListener(SongInsertVetoListener* listener);

public slots:
  void set_current_row(int index);

  void Clear();
  void Shuffle();

  void RepopulateDynamicPlaylist();
  void TurnOffDynamicPlaylist();

signals:
  void CurrentSongChanged(const Song& metadata);
  void EditingFinished(const QModelIndex& index);
  void PlayRequested(const QModelIndex& index);

  // Signals that the underlying list of items was changed, meaning that
  // something was added to it, removed from it or the ordering changed.
  void PlaylistChanged();
  void DynamicModeChanged(bool dynamic);

  void LoadTracksError(const QString& message);

private:
  Playlist();
};
