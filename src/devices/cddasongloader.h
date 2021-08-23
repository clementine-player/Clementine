/* This file is part of Clementine.
   Copyright 2014, David Sansome <me@davidsansome.com>
   Copyright 2021, Lukas Prediger <lumip@lumip.de>

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

#include <QFuture>
#include <QObject>
#include <QUrl>

// These must come after Qt includes (issue 3247)
#include <gst/audio/gstaudiocdsrc.h>

#include <atomic>

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
  // Signals declared below will be emitted anytime new information becomes
  // available.
  void LoadSongs();
  bool IsActive() const;

  // The list of currently cached tracks. This gets updated when
  // LoadSongs() is called.
  SongList cached_tracks() const;

 signals:
  // Emitted whenever information about tracks were updated.
  // Guarantees consistency with previous updates, i.e., consumers can rely
  // entirely on the updated list and do not have to merge metadata. May be
  // emitted multiple times during reading the disc as long as the Finished
  // signal was not emitted.
  void SongsUpdated(const SongList& songs);
  // Emitted when no further updates will follow; guaranteed to be emitted
  // at some point.
  void Finished();

  // The following signals are mostly for internal processing; other classes
  // can get all relevant updates by just connecting to SongsUpdated. However,
  // the more specialised remain available.

  // Emitted when the number of tracks has been initially loaded from the disc.
  // This is a specialised signal; subscribe to SongsUpdated for general updates
  // information about tracks is updated.
  void SongsLoaded(const SongList& songs);
  // Emitted when track durations have been loaded from the disc.
  // This is a specialised signal; subscribe to SongsUpdated for general updates
  // information about tracks is updated.
  void SongsDurationLoaded(const SongList& songs);
  // Emitted when metadata has been loaded from the disc.
  // This is a specialised signal; subscribe to SongsUpdated for general updates
  // information about tracks is updated.
  void SongsMetadataLoaded(const SongList& songs);
  // Emitted when the MusicBrainz disc id has been determined.
  // This is a specialised signal; subscribe to SongsUpdated for general updates
  // information about tracks is updated.
  void MusicBrainzDiscIdLoaded(const QString& musicbrainz_discid);

 private slots:
  void LoadAudioCDTags(const QString& musicbrainz_discid) const;
  void ProcessMusicBrainzResponse(const QString& artist, const QString& album,
                                  const MusicBrainzClient::ResultList& results);
  void SetDiscTracks(const SongList& songs, bool has_titles);

 private:
  QUrl GetUrlFromTrack(int track_number) const;
  void LoadSongsFromCdda();
  // Parse gstreamer taglist for a song
  // Returns true if any tags were read, updates the Song object in songs
  // accordingly, and returns the zero-based track index via the track_no
  // argument.
  bool ParseSongTags(SongList& songs, GstTagList* tags, gint* track_no);

  struct Disc {
    SongList tracks;
    bool has_titles;  // indicates that titles have been read and are not
                      // defaulted
  };

  QUrl url_;
  GstElement* cdda_;
  QFuture<void> loading_future_;
  std::atomic<bool> may_load_;
  Disc disc_;
  mutable QMutex disc_mutex_;
};

#endif  // CDDASONGLOADER_H
