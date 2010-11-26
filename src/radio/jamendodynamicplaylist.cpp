#include "jamendodynamicplaylist.h"

#include <QEventLoop>
#include <QtDebug>

#include "core/network.h"
#include "core/songloader.h"
#include "library/librarybackend.h"
#include "radio/jamendoplaylistitem.h"

const char* JamendoDynamicPlaylist::kTopTracksMonthUrl =
    "http://api.jamendo.com/get2/id+name+url+stream+album_name+"
    "album_url+album_id+artist_id+artist_name/track/xspf/"
    "track_album+album_artist/?order=ratingmonth_desc&streamencoding=ogg2";

JamendoDynamicPlaylist::JamendoDynamicPlaylist()
  : network_(new NetworkAccessManager(this)),
    current_page_(0),
    current_index_(0) {

}

void JamendoDynamicPlaylist::Load(const QByteArray& data) {

}

QByteArray JamendoDynamicPlaylist::Save() const {
  return QByteArray();
}

PlaylistItemList JamendoDynamicPlaylist::Generate() {
  return GenerateMore(20);
}

PlaylistItemList JamendoDynamicPlaylist::GenerateMore(int count) {
  PlaylistItemList items;
  while (items.size() < count) {
    // Add items from current list.
    if (current_index_ < current_items_.size()) {
      PlaylistItemList more_items = current_items_.mid(current_index_, count);
      items << more_items;
      current_index_ += more_items.size();
    } else {
      // We need more songs!
      Fetch();
    }
  }

  return items;
}

void JamendoDynamicPlaylist::Fetch() {
  QUrl url = QUrl(kTopTracksMonthUrl);
  url.addQueryItem("pn", QString::number(current_page_++));
  url.addQueryItem("n", QString::number(kPageSize));

  SongLoader loader(backend_);
  SongLoader::Result result = loader.Load(url);

  if (result != SongLoader::WillLoadAsync) {
    qWarning() << "Jamendo dynamic playlist fetch failed with:"
               << url;
    return;
  }

  // Blocking wait for reply.
  {
    QEventLoop event_loop;
    connect(&loader, SIGNAL(LoadFinished(bool)), &event_loop, SLOT(quit()));
    event_loop.exec();
  }

  const SongList& songs = loader.songs();
  if (songs.empty()) {
    qWarning() << "No songs returned from Jamendo:"
               << url;
    return;
  }

  current_items_.clear();
  foreach (const Song& song, songs) {
    current_items_ << PlaylistItemPtr(new JamendoPlaylistItem(song));
  }
  current_index_ = 0;
}
