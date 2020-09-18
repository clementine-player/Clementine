/* This file is part of Clementine.
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#ifndef CDDASONGLOADER_H
#define CDDASONGLOADER_H

#include <QMutex>
#include <QObject>
#include <QUrl>

// These must come after Qt includes (issue 3247)
#include <cdio/cdio.h>
#include <gst/audio/gstaudiocdsrc.h>

#include "core/song.h"
#include "musicbrainz/musicbrainzclient.h"

// This class provides a (hopefully) nice, high level interface to get CD
// information and load tracks
class CddaSongLoader : public QObject {
  Q_OBJECT

 public:
  CddaSongLoader(
      // Url of the CD device. Will use the default device if empty
      const QUrl& url = QUrl(), QObject* parent = nullptr);
  ~CddaSongLoader();

  // Load songs.
  // Signals declared below will be emitted anytime new information will be
  // available.
  void LoadSongs();
  bool HasChanged();

 signals:
  void SongsLoaded(const SongList& songs);
  void SongsDurationLoaded(const SongList& songs);
  void SongsMetadataLoaded(const SongList& songs);
  void MusicBrainzDiscIdLoaded(const QString& musicbrainz_discid);

 private slots:
  void LoadAudioCDTags(const QString& musicbrainz_discid) const;
  void AudioCDTagsLoaded(const QString& artist, const QString& album,
                         const MusicBrainzClient::ResultList& results);

 private:
  QUrl GetUrlFromTrack(int track_number) const;
  void LoadSongsFromCdda();

  QUrl url_;
  GstElement* cdda_;
  CdIo_t* cdio_;
  QMutex mutex_load_;
};

#endif  // CDDASONGLOADER_H
