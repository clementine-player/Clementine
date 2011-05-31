#ifndef SPOTIFYRESOLVER_H
#define SPOTIFYRESOLVER_H

#include <QMap>

#include "resolver.h"

namespace protobuf {
class SearchResponse;
}

class SpotifyServer;

class SpotifyResolver : public Resolver {
  Q_OBJECT
 public:
  SpotifyResolver(SpotifyServer* service, QObject* parent = 0);
  int ResolveSong(const Song& song);

 signals:
  void ResolveFinished(int id, SongList songs);

 private slots:
  void SearchFinished(const protobuf::SearchResponse& response);

 private:
  SpotifyServer* spotify_;
  QMap<QString, int> queries_;
  int next_id_;
};

#endif  // SPOTIFYRESOLVER_H
