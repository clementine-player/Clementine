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

#include <QMap>
#include <QObject>
#include <QXmlStreamReader>

class NetworkTimeouts;

class QNetworkAccessManager;
class QNetworkReply;

class MusicBrainzClient : public QObject {
  Q_OBJECT

  // Gets metadata for a particular PUID.
  // A PUID is created from a fingerprint using MusicDnsClient.
  // You can create one MusicBrainzClient and make multiple requests using it.
  // IDs are provided by the caller when a request is started and included in
  // the Finished signal - they have no meaning to MusicBrainzClient.

public:
  MusicBrainzClient(QObject* parent = 0);

  struct Result {
    Result() : duration_msec_(0), track_(0) {}

    QString title_;
    QString artist_;
    QString album_;
    int duration_msec_;
    int track_;
  };
  typedef QList<Result> ResultList;

  // Starts a request and returns immediately.  Finished() will be emitted
  // later with the same ID.
  void Start(int id, const QString& puid);
  void StartDiscIdRequest(const QString& puid);

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
  void RequestFinished();
  void DiscIdRequestFinished();

private:
  static Result ParseTrack(QXmlStreamReader* reader);
  static void ParseArtist(QXmlStreamReader* reader, QString* artist);
  static void ParseAlbum(QXmlStreamReader* reader, QString* album, int* track);

private:
  static const char* kTrackUrl;
  static const char* kDiscUrl;
  static const int kDefaultTimeout;

  QNetworkAccessManager* network_;
  NetworkTimeouts* timeouts_;
  QMap<QNetworkReply*, int> requests_;
};

#endif // MUSICBRAINZCLIENT_H
