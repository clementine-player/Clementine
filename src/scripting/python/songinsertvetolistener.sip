class SongInsertVetoListener : QObject {
%TypeHeaderCode
  #include "playlist/playlist.h"
%End

public:
  virtual SongList AboutToInsertSongs(const SongList& old_songs, const SongList& new_songs) = 0;
};
