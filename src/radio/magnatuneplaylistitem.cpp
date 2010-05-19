#include "magnatuneplaylistitem.h"

MagnatunePlaylistItem::MagnatunePlaylistItem(const QString& type)
  : LibraryPlaylistItem(type)
{
}

MagnatunePlaylistItem::MagnatunePlaylistItem(const Song& song)
  : LibraryPlaylistItem("Magnatune")
{
  song_ = song;
}

bool MagnatunePlaylistItem::InitFromQuery(const QSqlQuery &query) {
  // Rows from the songs tables come first
  song_.InitFromQuery(query, Song::kColumns.count() + 1);

  return song_.is_valid();
}

QUrl MagnatunePlaylistItem::Url() const {
  return QUrl::fromEncoded(song_.filename().toAscii());
}
