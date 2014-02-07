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

#ifndef ACOUSTIDCLIENT_H
#define ACOUSTIDCLIENT_H

#include <QMap>
#include <QObject>

class NetworkTimeouts;

class QNetworkAccessManager;
class QNetworkReply;

class AcoustidClient : public QObject {
  Q_OBJECT

  // Gets a MBID from a Chromaprint fingerprint.
  // A fingerprint identifies one particular encoding of a song and is created
  // by Fingerprinter.  An MBID identifies the actual song and can be passed to
  // Musicbrainz to get metadata.
  // You can create one AcoustidClient and make multiple requests using it.
  // IDs are provided by the caller when a request is started and included in
  // the Finished signal - they have no meaning to AcoustidClient.

 public:
  AcoustidClient(QObject* parent = 0);

  // Network requests will be aborted after this interval.
  void SetTimeout(int msec);

  // Starts a request and returns immediately.  Finished() will be emitted
  // later with the same ID.
  void Start(int id, const QString& fingerprint, int duration_msec);

  // Cancels the request with the given ID.  Finished() will never be emitted
  // for that ID.  Does nothing if there is no request with the given ID.
  void Cancel(int id);

  // Cancels all requests.  Finished() will never be emitted for any pending
  // requests.
  void CancelAll();

signals:
  void Finished(int id, const QString& mbid);

 private slots:
  void RequestFinished(QNetworkReply* reply, int id);

 private:
  static const char* kClientId;
  static const char* kUrl;
  static const int kDefaultTimeout;

  QNetworkAccessManager* network_;
  NetworkTimeouts* timeouts_;
  QMap<int, QNetworkReply*> requests_;
};

#endif  // ACOUSTIDCLIENT_H
