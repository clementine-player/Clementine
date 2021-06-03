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

#include "cddasongloader.h"

#include <gst/gst.h>
#include <gst/tag/tag.h>

#include <QtConcurrentRun>

#include "cddadevice.h"
#include "core/logging.h"
#include "core/timeconstants.h"

CddaSongLoader::CddaSongLoader(const QUrl& url, QObject* parent)
    : QObject(parent), url_(url), cdda_(nullptr), may_load_(true) {
  connect(this, SIGNAL(MusicBrainzDiscIdLoaded(const QString&)),
          SLOT(LoadAudioCDTags(const QString&)));
}

CddaSongLoader::~CddaSongLoader() {
  // The LoadSongsFromCdda methods runs concurrently in a thread and we need to
  // wait for it to terminate. There's no guarantee that it has terminated when
  // destructor is invoked.
  may_load_ = false;
  loading_future_.waitForFinished();
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

bool CddaSongLoader::IsActive() const { return loading_future_.isRunning(); }

void CddaSongLoader::LoadSongs() {
  // only dispatch a new thread for loading tracks if not already running.
  if (!IsActive()) {
    loading_future_ =
        QtConcurrent::run(this, &CddaSongLoader::LoadSongsFromCdda);
  }
}

void CddaSongLoader::LoadSongsFromCdda() {
  if (!may_load_) return;

  // Create gstreamer cdda element
  GError* error = nullptr;
  cdda_ = gst_element_make_from_uri(GST_URI_SRC, "cdda://", nullptr, &error);
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
  GstFormat fmt = gst_format_get_by_nick("track");
  GstFormat out_fmt = fmt;
  gint64 num_tracks = 0;
  if (!gst_element_query_duration(cdda_, out_fmt, &num_tracks) ||
      out_fmt != fmt) {
    qLog(Error) << "Error while querying cdda GstElement";
    gst_object_unref(GST_OBJECT(cdda_));
    return;
  }

  SongList songs;
  for (int track_number = 1; track_number <= num_tracks; track_number++) {
    // Init song
    Song song;
    song.set_id(track_number);
    song.set_valid(true);
    song.set_filetype(Song::Type_Cdda);
    song.set_url(GetUrlFromTrack(track_number));
    song.set_title(QString("Track %1").arg(track_number));
    song.set_track(track_number);
    songs << song;
  }
  emit SongsLoaded(songs);

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
        if (entries && songs.size() <= g_list_length(entries)) {
          int i = 0;
          for (GList* node = entries; node != nullptr; node = node->next) {
            GstTocEntry* entry = static_cast<GstTocEntry*>(node->data);
            quint64 duration = 0;
            gint64 start, stop;
            if (gst_toc_entry_get_start_stop_times(entry, &start, &stop))
              duration = stop - start;
            songs[i++].set_length_nanosec(duration);
          }
          emit SongsDurationLoaded(songs);
          msg_filter = static_cast<GstMessageType>(
              static_cast<int>(msg_filter) ^ GST_MESSAGE_TOC);
        }
        gst_toc_unref(toc);
      }
    } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_TAG) {
      // Handle TAG message: generate MusicBrainz DiscId

      GstTagList* tags = nullptr;
      gst_message_parse_tag(msg, &tags);
      char* string_mb = nullptr;
      if (gst_tag_list_get_string(tags, GST_TAG_CDDA_MUSICBRAINZ_DISCID,
                                  &string_mb)) {
        QString musicbrainz_discid = QString::fromUtf8(string_mb);
        g_free(string_mb);

        qLog(Info) << "MusicBrainz discid: " << musicbrainz_discid;
        emit MusicBrainzDiscIdLoaded(musicbrainz_discid);
        msg_filter = static_cast<GstMessageType>(static_cast<int>(msg_filter) ^
                                                 GST_MESSAGE_TAG);
      }
      gst_tag_list_free(tags);
    }
    gst_message_unref(msg);
  }

  gst_element_set_state(pipeline, GST_STATE_NULL);
  // This will also cause cdda_ to be unref'd.
  gst_object_unref(pipeline);
}

void CddaSongLoader::LoadAudioCDTags(const QString& musicbrainz_discid) const {
  MusicBrainzClient* musicbrainz_client = new MusicBrainzClient;
  connect(musicbrainz_client,
          SIGNAL(Finished(const QString&, const QString&,
                          MusicBrainzClient::ResultList)),
          SLOT(AudioCDTagsLoaded(const QString&, const QString&,
                                 MusicBrainzClient::ResultList)));

  musicbrainz_client->StartDiscIdRequest(musicbrainz_discid);
}

void CddaSongLoader::AudioCDTagsLoaded(
    const QString& artist, const QString& album,
    const MusicBrainzClient::ResultList& results) {
  MusicBrainzClient* musicbrainz_client =
      qobject_cast<MusicBrainzClient*>(sender());
  musicbrainz_client->deleteLater();
  SongList songs;
  if (results.empty()) return;
  int track_number = 1;
  for (const MusicBrainzClient::Result& ret : results) {
    Song song;
    song.set_artist(artist);
    song.set_album(album);
    song.set_title(ret.title_);
    song.set_length_nanosec(ret.duration_msec_ * kNsecPerMsec);
    song.set_track(track_number);
    song.set_year(ret.year_);
    song.set_id(track_number);
    song.set_filetype(Song::Type_Cdda);
    song.set_valid(true);
    // We need to set url: that's how playlist will find the correct item to
    // update
    song.set_url(GetUrlFromTrack(track_number++));
    songs << song;
  }
  emit SongsMetadataLoaded(songs);
}
