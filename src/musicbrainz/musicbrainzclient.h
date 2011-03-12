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

class QNetworkAccessManager;
class QNetworkReply;
class QXmlStreamReader;

class MusicBrainzClient : public QObject {
  Q_OBJECT

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

  void Start(int id, const QString& puid);
  void Cancel(int id);
  void CancelAll();

signals:
  void Finished(int id, const MusicBrainzClient::ResultList& result);

private slots:
  void RequestFinished();

private:
  static Result ParseTrack(QXmlStreamReader* reader);
  static void ParseArtist(QXmlStreamReader* reader, QString* artist);
  static void ParseAlbum(QXmlStreamReader* reader, QString* album, int* track);

private:
  static const char* kUrl;

  QNetworkAccessManager* network_;
  QMap<QNetworkReply*, int> requests_;
};

#endif // MUSICBRAINZCLIENT_H
