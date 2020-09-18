/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MUSICBRAINZCLIENT_H
#define MUSICBRAINZCLIENT_H

#include <QHash>
#include <QMultiMap>
#include <QObject>
#include <QXmlStreamReader>

class NetworkTimeouts;

class QNetworkAccessManager;
class QNetworkReply;

class MusicBrainzClient : public QObject {
  Q_OBJECT

  // Gets metadata for a particular MBID.
  // An MBID is created from a fingerprint using MusicDnsClient.
  // You can create one MusicBrainzClient and make multiple requests using it.
  // IDs are provided by the caller when a request is started and included in
  // the Finished signal - they have no meaning to MusicBrainzClient.

 public:
  // The second argument allows for specifying a custom network access
  // manager. It is used in tests. The ownership of network
  // is not transferred.
  MusicBrainzClient(QObject* parent = nullptr,
                    QNetworkAccessManager* network = nullptr);

  struct Result {
    Result() : duration_msec_(0), track_(0), year_(-1) {}

    bool operator<(const Result& other) const {
#define cmp(field)                      \
  if (field < other.field) return true; \
  if (field > other.field) return false;

      cmp(track_);
      cmp(year_);
      cmp(title_);
      cmp(artist_);
      return false;

#undef cmp
    }

    bool operator==(const Result& other) const {
      return title_ == other.title_ && artist_ == other.artist_ &&
             album_ == other.album_ && duration_msec_ == other.duration_msec_ &&
             track_ == other.track_ && year_ == other.year_;
    }

    QString title_;
    QString artist_;
    QString album_;
    int duration_msec_;
    int track_;
    int year_;
  };
  typedef QList<Result> ResultList;

  // Starts a request and returns immediately.  Finished() will be emitted
  // later with the same ID.
  void Start(int id, const QStringList& mbid);
  void StartDiscIdRequest(const QString& discid);

  // Cancels the request with the given ID.  Finished() will never be emitted
  // for that ID.  Does nothing if there is no request with the given ID.
  void Cancel(int id);

  // Cancels all requests.  Finished() will never be emitted for any pending
  // requests.
  void CancelAll();

 signals:
  // Finished signal emitted when fechting songs tags
  void Finished(int id, const MusicBrainzClient::ResultList& result);
  // Finished signal emitted when fechting album's songs tags using DiscId
  void Finished(const QString& artist, const QString album,
                const MusicBrainzClient::ResultList& result);

 private slots:
  // id identifies the track, and request_number means it's the
  // 'request_number'th request for this track
  void RequestFinished(QNetworkReply* reply, int id, int request_number);
  void DiscIdRequestFinished(const QString& discid, QNetworkReply* reply);

 private:
  // Used as parameter for UniqueResults
  enum UniqueResultsSortOption { SortResults = 0, KeepOriginalOrder };

  struct Release {
    enum Status {
      Status_Unknown = 0,
      Status_PseudoRelease,
      Status_Bootleg,
      Status_Promotional,
      Status_Official
    };

    Release() : track_(0), year_(0), status_(Status_Unknown) {}

    Result CopyAndMergeInto(const Result& orig) const {
      Result ret(orig);
      ret.album_ = album_;
      ret.track_ = track_;
      ret.year_ = year_;
      return ret;
    }

    void SetStatusFromString(const QString& s) {
      if (s.compare("Official", Qt::CaseInsensitive) == 0) {
        status_ = Status_Official;
      } else if (s.compare("Promotion", Qt::CaseInsensitive) == 0) {
        status_ = Status_Promotional;
      } else if (s.compare("Bootleg", Qt::CaseInsensitive) == 0) {
        status_ = Status_Bootleg;
      } else if (s.compare("Pseudo-release", Qt::CaseInsensitive) == 0) {
        status_ = Status_PseudoRelease;
      } else {
        status_ = Status_Unknown;
      }
    }

    bool operator<(const Release& other) const {
      // Compare status so that "best" status (e.g. Official) will be first
      // when sorting a list of releases.
      return status_ > other.status_;
    }

    QString album_;
    int track_;
    int year_;
    Status status_;
  };

  struct PendingResults {
    PendingResults(int sort_id, const ResultList& results)
        : sort_id_(sort_id), results_(results) {}

    bool operator<(const PendingResults& other) const {
      return sort_id_ < other.sort_id_;
    }

    int sort_id_;
    ResultList results_;
  };

  static bool MediumHasDiscid(const QString& discid, QXmlStreamReader* reader);
  static ResultList ParseMedium(QXmlStreamReader* reader);
  static Result ParseTrackFromDisc(QXmlStreamReader* reader);
  static ResultList ParseTrack(QXmlStreamReader* reader);
  static void ParseArtist(QXmlStreamReader* reader, QString* artist);
  static Release ParseRelease(QXmlStreamReader* reader);
  static ResultList UniqueResults(const ResultList& results,
                                  UniqueResultsSortOption opt = SortResults);

 private:
  static const char* kTrackUrl;
  static const char* kDiscUrl;
  static const char* kDateRegex;
  static const int kDefaultTimeout;
  static const int kMaxRequestPerTrack;

  QNetworkAccessManager* network_;
  NetworkTimeouts* timeouts_;
  QMultiMap<int, QNetworkReply*> requests_;
  // Results we received so far, kept here until all the replies are finished
  QMap<int, QList<PendingResults>> pending_results_;
};

inline uint qHash(const MusicBrainzClient::Result& result) {
  return qHash(result.album_) ^ qHash(result.artist_) ^ result.duration_msec_ ^
         qHash(result.title_) ^ result.track_ ^ result.year_;
}
#endif  // MUSICBRAINZCLIENT_H
