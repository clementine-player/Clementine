#ifndef SPOTIFYRESOLVER_H
#define SPOTIFYRESOLVER_H

#include <QMap>

#include "resolver.h"

namespace spotify_pb {
  class SearchResponse;
}

class SpotifyService;
class SpotifyServer;

class SpotifyResolver : public Resolver {
  Q_OBJECT
 public:
  SpotifyResolver(SpotifyService* service, QObject* parent = 0);
  int ResolveSong(const Song& song);

 signals:
  void ResolveFinished(int id, SongList songs);

 private slots:
  void SearchFinished(const spotify_pb::SearchResponse& response);
  void ServerDestroyed();

 private:
  SpotifyServer* server();

 private:
  SpotifyService* service_;
  SpotifyServer* server_;
  QMap<QString, int> queries_;
  int next_id_;
};

#endif  // SPOTIFYRESOLVER_H
