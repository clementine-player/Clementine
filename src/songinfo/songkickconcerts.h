#ifndef SONGKICKCONCERTS_H
#define SONGKICKCONCERTS_H

#include "songinfoprovider.h"

class QNetworkReply;

class SongkickConcerts : public SongInfoProvider {
  Q_OBJECT

 public:
  SongkickConcerts();
  void FetchInfo(int id, const Song& metadata);

 private slots:
  void ArtistSearchFinished(QNetworkReply* reply, int id);
  void CalendarRequestFinished();

 private:
  void FetchSongkickCalendar(const QString& artist_id, int id);

  static const char* kSongkickArtistBucket;
  static const char* kSongkickArtistCalendarUrl;
};

#endif
