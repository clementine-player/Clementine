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

#include "cddasongloader.h"

#include <gst/gst.h>
#include <gst/tag/tag.h>

#include <QtConcurrentRun>

#include "cddadevice.h"
#include "core/logging.h"
#include "core/timeconstants.h"

CddaSongLoader::CddaSongLoader(const QUrl& url, QObject* parent)
    : QObject(parent),
      url_(url),
      cdda_(nullptr),
      cdio_(nullptr),
      may_load_(true),
      disc_() {
  connect(this, SIGNAL(MusicBrainzDiscIdLoaded(const QString&)),
          SLOT(LoadAudioCDTags(const QString&)));
  connect(this, SIGNAL(SongsLoaded(const SongList&)),
          SLOT(SetDiscTracks(const SongList&)));
  connect(this, SIGNAL(SongsDurationLoaded(const SongList&)),
          SLOT(SetDiscTracks(const SongList&)));
  connect(this, SIGNAL(SongsMetadataLoaded(const SongList&)),
          SLOT(SetDiscTracks(const SongList&)));

  cdio_ = cdio_open(url_.path().toLocal8Bit().constData(), DRIVER_DEVICE);
  Q_ASSERT(cdio_ != nullptr);  // todo: assert for now, but we need to actually
                               // handle error cases here
}

CddaSongLoader::~CddaSongLoader() {
  // The LoadSongsFromCdda method runs concurrently in a thread and we need to
  // wait for it to terminate. There's no guarantee that it has terminated when
  // destructor is invoked.
  may_load_ = false;
  loading_future_.waitForFinished();
  Q_ASSERT(cdio_);
  cdio_destroy(cdio_);
}

QUrl CddaSongLoader::GetUrlFromTrack(int track_number) const {
  QString track;
  if (url_.isEmpty()) {
    track = QString("cdda://%1").arg(track_number);
  } else {
    track = QString("cdda://%1/%2").arg(url_.path()).arg(track_number);
  }
  return CddaDevice::TrackStrToUrl(track);
}

void CddaSongLoader::LoadSongs() {
  // first, make sure we are not already reading the disc,
  // in which case we do nothing and just wait for that
  // to finish and advertise results
  if (loading_future_.isRunning()) return;

  // if not currently reading, figure out if we need to or
  // can rely on cached information
  if (!disc_.has_been_read || HasChanged()) {
    disc_.has_been_read = true;
    emit DiscChanged();
    loading_future_ =
        QtConcurrent::run(this, &CddaSongLoader::LoadSongsFromCdda);
  } else
    emit SongsUpdated(disc_.tracks, false);
  // we have all information ready, no further updates follow
}

bool CddaSongLoader::ParseSongTags(SongList& songs, GstTagList* tags) {
  //// cdiocddasrc reads cd-text with following mapping from cdio
  ///
  /// DISC LEVEL :
  /// CDTEXT_FIELD_PERFORMER -> GST_TAG_ALBUM_ARTIST
  /// CDTEXT_FIELD_TITLE -> GST_TAG_ALBUM
  /// CDTEXT_FIELD_GENRE -> GST_TAG_GENRE
  ///
  /// TRACK LEVEL :
  /// CDTEXT_FIELD_PERFORMER -> GST_TAG_ARTIST
  /// CDTEXT_FIELD_TITLE -> GST_TAG_TITLE

  guint track_number;
  if (!gst_tag_list_get_uint(tags, GST_TAG_TRACK_NUMBER, &track_number)) {
    qLog(Error) << "Track tags do not contain track number!";
    return false;
  }

  Q_ASSERT(track_number != 0u);
  Q_ASSERT(static_cast<int>(track_number) <= songs.size());
  Song& song = songs[static_cast<int>(track_number - 1)];

  qLog(Debug) << gst_tag_list_to_string(tags);

  gchar* buffer = nullptr;
  if (gst_tag_list_get_string(tags, GST_TAG_ALBUM, &buffer)) {
    song.set_album(QString::fromUtf8(buffer));
    g_free(buffer);
  }

  if (gst_tag_list_get_string(tags, GST_TAG_ALBUM_ARTIST, &buffer)) {
    song.set_albumartist(QString::fromUtf8(buffer));
    g_free(buffer);
  }

  if (gst_tag_list_get_string(tags, GST_TAG_GENRE, &buffer)) {
    song.set_genre(QString::fromUtf8(buffer));
    g_free(buffer);
  }

  if (gst_tag_list_get_string(tags, GST_TAG_ARTIST, &buffer)) {
    song.set_artist(QString::fromUtf8(buffer));
    g_free(buffer);
  }

  if (gst_tag_list_get_string(tags, GST_TAG_TITLE, &buffer)) {
    song.set_title(QString::fromUtf8(buffer));
    g_free(buffer);
  }

  guint64 duration;
  if (gst_tag_list_get_uint64(tags, GST_TAG_DURATION, &duration)) {
    song.set_length_nanosec(duration);
  }

  song.set_track(track_number);
  song.set_id(track_number);
  song.set_filetype(Song::Type_Cdda);
  song.set_valid(true);
  song.set_url(GetUrlFromTrack(track_number));
  return true;
}

void CddaSongLoader::LoadSongsFromCdda() {
  if (!may_load_) return;

  // Create gstreamer cdda element
  GError* error = nullptr;
  GstElement* cdda_ = gst_element_factory_make("cdiocddasrc", nullptr);
  if (error) {
    qLog(Error) << error->code << QString::fromLocal8Bit(error->message);
  }
  if (cdda_ == nullptr) {
    return;
  }

  if (!url_.isEmpty()) {
    g_object_set(cdda_, "device",
                 g_strdup(url_.path().toLocal8Bit().constData()), nullptr);
  }
  if (g_object_class_find_property(G_OBJECT_GET_CLASS(cdda_),
                                   "paranoia-mode")) {
    g_object_set(cdda_, "paranoia-mode", 0, NULL);
  }

  // Change the element's state to ready and paused, to be able to query it
  if (gst_element_set_state(cdda_, GST_STATE_READY) ==
          GST_STATE_CHANGE_FAILURE ||
      gst_element_set_state(cdda_, GST_STATE_PAUSED) ==
          GST_STATE_CHANGE_FAILURE) {
    gst_element_set_state(cdda_, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(cdda_));
    return;
  }

  // Get number of tracks
  GstFormat track_fmt = gst_format_get_by_nick("track");
  gint64 num_tracks = 0;
  if (!gst_element_query_duration(cdda_, track_fmt, &num_tracks)) {
    qLog(Error) << "Error while querying cdda GstElement for track count";
    gst_object_unref(GST_OBJECT(cdda_));
    return;
  }

  SongList initialSongList;
  for (int track_number = 1; track_number <= num_tracks; track_number++) {
    // Init song
    Song song;
    song.set_id(track_number);
    song.set_valid(true);
    song.set_filetype(Song::Type_Cdda);
    song.set_url(GetUrlFromTrack(track_number));
    song.set_title(QString("Track %1").arg(track_number));
    song.set_track(track_number);
    initialSongList << song;
  }
  emit SongsLoaded(initialSongList);

  SongList taggedSongList(initialSongList);

  gst_tag_register_musicbrainz_tags();

  GstElement* pipeline = gst_pipeline_new("pipeline");
  GstElement* sink = gst_element_factory_make("fakesink", NULL);
  gst_bin_add_many(GST_BIN(pipeline), cdda_, sink, NULL);
  gst_element_link(cdda_, sink);
  gst_element_set_state(pipeline, GST_STATE_READY);
  gst_element_set_state(pipeline, GST_STATE_PAUSED);

  // Get TOC and TAG messages
  GstMessage* msg = nullptr;
  GstMessageType msg_filter =
      static_cast<GstMessageType>(GST_MESSAGE_TOC | GST_MESSAGE_TAG);
  QString musicbrainz_discid;
  while (may_load_ && msg_filter &&
         (msg = gst_bus_timed_pop_filtered(GST_ELEMENT_BUS(pipeline),
                                           10 * GST_SECOND, msg_filter))) {
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_TOC) {
      // Handle TOC message: get tracks duration
      GstToc* toc;
      gst_message_parse_toc(msg, &toc, nullptr);
      if (toc) {
        GList* entries = gst_toc_get_entries(toc);
        if (entries && initialSongList.size() <= g_list_length(entries)) {
          int i = 0;
          for (GList* node = entries; node != nullptr; node = node->next) {
            GstTocEntry* entry = static_cast<GstTocEntry*>(node->data);
            quint64 duration = 0;
            gint64 start, stop;
            if (gst_toc_entry_get_start_stop_times(entry, &start, &stop))
              duration = stop - start;
            initialSongList[i++].set_length_nanosec(duration);
          }
          emit SongsDurationLoaded(initialSongList);
          msg_filter = static_cast<GstMessageType>(
              static_cast<int>(msg_filter) ^ GST_MESSAGE_TOC);
        }
        gst_toc_unref(toc);
      }
    } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_TAG) {
      // Handle TAG message: generate MusicBrainz DiscId and read CD-TEXT if
      // present

      GstTagList* tags = nullptr;
      gst_message_parse_tag(msg, &tags);
      char* string_mb = nullptr;
      if (musicbrainz_discid.isEmpty() &&
          gst_tag_list_get_string(tags, GST_TAG_CDDA_MUSICBRAINZ_DISCID,
                                  &string_mb)) {
        musicbrainz_discid = QString::fromUtf8(string_mb);
        g_free(string_mb);
        qLog(Info) << "MusicBrainz discid: " << musicbrainz_discid;
        // emit MusicBrainzDiscIdLoaded(musicbrainz_discid);
        // for now, we'll invoke musicbrainz only after having read all CD-TEXT
        // tags and emitted a message for it
      }

      ParseSongTags(taggedSongList, tags);
      gst_tag_list_free(tags);

      gint64 track_number;
      gst_element_query_position(cdda_, track_fmt, &track_number);
      track_number += 1;
      if (track_number < num_tracks) {
        // more to go, seek to next track to get a tag message for it
        gst_element_seek_simple(
            pipeline, track_fmt,
            static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH |
                                      GST_SEEK_FLAG_TRICKMODE),
            track_number);
      } else  // we are done with reading track tags: do no longer filter
        msg_filter = static_cast<GstMessageType>(static_cast<int>(msg_filter) ^
                                                 GST_MESSAGE_TAG);
    }
    gst_message_unref(msg);
  }
  emit SongsMetadataLoaded(taggedSongList);

  if (!musicbrainz_discid.isEmpty())
    emit MusicBrainzDiscIdLoaded(musicbrainz_discid);

  // cleanup
  gst_element_set_state(pipeline, GST_STATE_NULL);
  // This will also cause cdda_ to be unref'd.
  gst_object_unref(pipeline);
}

void CddaSongLoader::LoadAudioCDTags(const QString& musicbrainz_discid) const {
  MusicBrainzClient* musicbrainz_client = new MusicBrainzClient;
  connect(musicbrainz_client,
          SIGNAL(Finished(const QString&, const QString&,
                          MusicBrainzClient::ResultList)),
          SLOT(ProcessMusicBrainzResponse(const QString&, const QString&,
                                          MusicBrainzClient::ResultList)));

  musicbrainz_client->StartDiscIdRequest(musicbrainz_discid);
}

void CddaSongLoader::ProcessMusicBrainzResponse(
    const QString& artist, const QString& album,
    const MusicBrainzClient::ResultList& results) {
  MusicBrainzClient* musicbrainz_client =
      qobject_cast<MusicBrainzClient*>(sender());
  musicbrainz_client->deleteLater();
  if (results.empty()) {
    // no real update, but we have to signal that
    // no further updates will follow now
    emit SongsUpdated(disc_.tracks, false);
    return;
  }

  if (disc_.tracks.length() != results.length()) {
    qLog(Warning) << "Number of tracks in metadata does not match number of "
                     "songs on disc!";
    return;  // no idea how to recover; just do nothing
  }

  for (int i = 0; i < results.length(); ++i) {
    const MusicBrainzClient::Result& new_song_info = results[i];
    Song& song = disc_.tracks[i];

    if (!disc_.has_titles) song.set_title(new_song_info.title_);
    if (song.album().isEmpty()) song.set_album(album);
    if (song.artist().isEmpty()) song.set_artist(artist);

    if (song.length_nanosec() == -1)
      song.set_length_nanosec(new_song_info.duration_msec_ * kNsecPerMsec);
    if (song.track() < 1) song.set_track(new_song_info.track_);
    if (song.year() == -1) song.set_year(new_song_info.year_);
  }

  emit SongsUpdated(disc_.tracks, false);  // musicbrainz response is our last
                                           // step, no further updates follow
}

void CddaSongLoader::SetDiscTracks(const SongList& songs) {
  disc_.tracks = songs;
  disc_.has_been_read = true;
  emit SongsUpdated(disc_.tracks, true);  // further updates may follow
}

bool CddaSongLoader::HasChanged() {
  Q_ASSERT(cdio_);
  if (cdio_get_media_changed(cdio_) != 1) {
    return false;
  }
  disc_ = Disc();
  return true;
}
