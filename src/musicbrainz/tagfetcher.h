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

#ifndef TAGFETCHER_H
#define TAGFETCHER_H

#include "musicbrainzclient.h"
#include "core/song.h"

#include <QFutureWatcher>
#include <QObject>

class AcoustidClient;

class TagFetcher : public QObject {
  Q_OBJECT

  // High level interface to Fingerprinter, AcoustidClient and
  // MusicBrainzClient.

 public:
  TagFetcher(QObject* parent = 0);

  void StartFetch(const SongList& songs);

 public slots:
  void Cancel();

signals:
  void Progress(const Song& original_song, const QString& stage);
  void ResultAvailable(const Song& original_song,
                       const SongList& songs_guessed);

 private slots:
  void FingerprintFound(int index);
  void PuidFound(int index, const QString& puid);
  void TagsFetched(int index, const MusicBrainzClient::ResultList& result);

 private:
  static QString GetFingerprint(const Song& song);

  QFutureWatcher<QString>* fingerprint_watcher_;
  AcoustidClient* acoustid_client_;
  MusicBrainzClient* musicbrainz_client_;

  SongList songs_;
};

#endif  // TAGFETCHER_H
