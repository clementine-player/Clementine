#include "spotifyresolver.h"

#include "core/logging.h"
#include "core/timeconstants.h"
#include "radio/spotifyserver.h"
#include "spotifyblob/common/spotifymessages.pb.h"

SpotifyResolver::SpotifyResolver(SpotifyServer* spotify, QObject* parent)
    : Resolver(parent),
      spotify_(spotify),
      next_id_(0) {
  connect(spotify_, SIGNAL(SearchResults(const protobuf::SearchResponse&)),
          SLOT(SearchFinished(const protobuf::SearchResponse&)));
}

int SpotifyResolver::ResolveSong(const Song& song) {
  QString query_string;
  query_string += "artist:\"" + song.artist() + "\"";
  query_string += " title:\"" + song.title() + "\"";
  query_string += " album:\"" + song.album() + "\"";

  qLog(Debug) << query_string;

  spotify_->Search(query_string, 25);

  int id = next_id_++;
  queries_[query_string] = id;
  return id;
}

void SpotifyResolver::SearchFinished(const protobuf::SearchResponse& response) {
  QString query_string = QString::fromUtf8(response.request().query().c_str());
  qLog(Debug) << query_string;
  QMap<QString, int>::iterator it = queries_.find(query_string);
  if (it == queries_.end()) {
    return;
  }

  int id = it.value();
  queries_.erase(it);

  SongList songs;
  for (int i = 0; i < response.result_size(); ++i) {
    const protobuf::Track& track = response.result(i);
    Song song;
    song.set_title(QString::fromUtf8(track.title().c_str()));
    if (track.artist_size() > 0) {
      song.set_artist(QString::fromUtf8(track.artist(0).c_str()));
    }
    song.set_album(QString::fromUtf8(track.album().c_str()));
    song.set_length_nanosec(track.duration_msec() * kNsecPerMsec);
    song.set_disc(track.disc());
    song.set_track(track.track());
    song.set_year(track.year());
    song.set_url(QUrl(QString::fromUtf8(track.uri().c_str())));
    song.set_valid(true);
    songs << song;
  }
  qLog(Debug) << "Resolved from spotify:" << songs.length();
  if (!songs.isEmpty()) {
    qLog(Debug) << songs[0].title() << songs[0].artist();
  }

  emit ResolveFinished(id, songs);
}
