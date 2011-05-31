#ifndef LASTFMSUGGESTER_H
#define LASTFMSUGGESTER_H

#include <QMap>
#include <QObject>

#include <lastfm/ws.h>

#include "core/song.h"

class LastFMSuggester : public QObject {
  Q_OBJECT
 public:
  LastFMSuggester(QObject* parent = 0);

 public slots:
  int SuggestSongs(const Song& song);

 signals:
  void SuggestSongsFinished(int id, const SongList& songs);

 private slots:
  void RequestFinished();

 private:
  QMap<QNetworkReply*, int> replies_;
  int next_id_;
};

#endif  // LASTFMSUGGESTER_H
