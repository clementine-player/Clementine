#include "magnatuneplaylistitem.h"
#include "magnatuneservice.h"
#include "radiomodel.h"

MagnatunePlaylistItem::MagnatunePlaylistItem(const QString& type)
  : LibraryPlaylistItem(type)
{
}

MagnatunePlaylistItem::MagnatunePlaylistItem(const Song& song)
  : LibraryPlaylistItem("Magnatune")
{
  song_ = song;
}

bool MagnatunePlaylistItem::InitFromQuery(const SqlRow& query) {
  // Rows from the songs tables come first
  song_.InitFromQuery(query, Song::kColumns.count() + 1);

  return song_.is_valid();
}

PlaylistItem::Options MagnatunePlaylistItem::options() const {
  return SpecialPlayBehaviour;
}

QUrl MagnatunePlaylistItem::Url() const {
  return QUrl::fromEncoded(song_.filename().toAscii());
}

PlaylistItem::SpecialLoadResult MagnatunePlaylistItem::StartLoading() {
  MagnatuneService* service = RadioModel::Service<MagnatuneService>();
  QUrl url(Url());

  return SpecialLoadResult(PlaylistItem::SpecialLoadResult::TrackAvailable,
                           url, service->ModifyUrl(url));
}
