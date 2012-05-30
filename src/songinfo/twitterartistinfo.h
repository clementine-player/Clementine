#ifndef TWITTERARTISTINFO_H
#define TWITTERARTISTINFO_H

#include "songinfoprovider.h"

#include "core/network.h"

class TwitterArtistInfo : public SongInfoProvider {
  Q_OBJECT
 public:
  TwitterArtistInfo();

  void FetchInfo(int id, const Song& metadata);

 private slots:
  void ArtistSearchFinished(QNetworkReply* reply, int id);
  void UserTimelineRequestFinished(
      QNetworkReply* reply, const QString& twitter_id, int id);

 private:
  void FetchUserTimeline(const QString& twitter_id, int id);

  NetworkAccessManager network_;

  static const char* kTwitterBucket;
  static const char* kTwitterTimelineUrl;
};

#endif
