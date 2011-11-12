#include "spotifyresolver.h"

#include "core/logging.h"
#include "core/timeconstants.h"
#include "internet/spotifyserver.h"
#include "internet/spotifyservice.h"
#include "spotifyblob/common/spotifymessages.pb.h"

SpotifyResolver::SpotifyResolver(SpotifyService* service, QObject* parent)
    : Resolver(parent),
      service_(service),
      server_(NULL),
      next_id_(0)
{
}

SpotifyServer* SpotifyResolver::server() {
  if (server_)
    return server_;

  if (service_->login_state() != SpotifyService::LoginState_LoggedIn)
    return NULL;

  server_ = service_->server();
  connect(server_, SIGNAL(SearchResults(spotify_pb::SearchResponse)),
          SLOT(SearchFinished(spotify_pb::SearchResponse)));
  connect(server_, SIGNAL(destroyed()), SLOT(ServerDestroyed()));

  return server_;
}

void SpotifyResolver::ServerDestroyed() {
  server_ = NULL;
}

int SpotifyResolver::ResolveSong(const Song& song) {
  SpotifyServer* s = server();
  if (!s) {
    return -1;
  }

  QString query_string;
  query_string += "artist:\"" + song.artist() + "\"";
  query_string += " title:\"" + song.title() + "\"";
  query_string += " album:\"" + song.album() + "\"";

  qLog(Debug) << query_string;

  s->Search(query_string, 25);

  int id = next_id_++;
  queries_[query_string] = id;
  return id;
}

void SpotifyResolver::SearchFinished(const spotify_pb::SearchResponse& response) {
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
    const spotify_pb::Track& track = response.result(i);
    Song song;
    SpotifyService::SongFromProtobuf(track, &song);
    songs << song;
  }
  qLog(Debug) << "Resolved from spotify:" << songs.length();
  if (!songs.isEmpty()) {
    qLog(Debug) << songs[0].title() << songs[0].artist();
  }

  emit ResolveFinished(id, songs);
}
