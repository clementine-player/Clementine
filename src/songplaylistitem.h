#ifndef SONGPLAYLISTITEM_H
#define SONGPLAYLISTITEM_H

#include "playlistitem.h"
#include "song.h"

class SongPlaylistItem : public PlaylistItem {
 public:
  SongPlaylistItem();
  SongPlaylistItem(const Song& song);

  Type type() const { return Type_Song; }

  void Save(QSettings& settings) const;
  void Restore(const QSettings& settings);
  void Reload();

  Song Metadata() const { return song_; }

  QUrl Url();

 private:
  void SaveFile(QSettings& settings) const;
  void SaveStream(QSettings& settings) const;

  void RestoreFile(const QSettings& settings);
  void RestoreStream(const QSettings& settings);
  Song song_;
};

#endif // SONGPLAYLISTITEM_H
