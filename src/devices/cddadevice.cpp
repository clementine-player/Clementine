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

#include <QMutexLocker>
#include <gst/gst.h>
#include <gst/tag/tag.h>
#include <gst/audio/gstaudiocdsrc.h>

#include "core/logging.h"
#include "core/timeconstants.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"

#include "cddadevice.h"

CddaDevice::CddaDevice(const QUrl& url, DeviceLister* lister,
                       const QString& unique_id, DeviceManager* manager,
                       Application* app, int database_id, bool first_time)
    : ConnectedDevice(url, lister, unique_id, manager, app, database_id,
                      first_time),
      cdda_(nullptr),
      cdio_(nullptr) {}

CddaDevice::~CddaDevice() {
  if (cdio_) cdio_destroy(cdio_);
}

void CddaDevice::Init() {
  QMutexLocker locker(&mutex_init_);
  song_count_ = 0;  // Reset song count, in case it was already set
  cdio_ = cdio_open(url_.path().toLocal8Bit().constData(), DRIVER_DEVICE);
  if (cdio_ == nullptr) {
    return;
  }
  // Create gstreamer cdda element
  GError* error = nullptr;
  cdda_ = gst_element_make_from_uri(GST_URI_SRC, "cdda://", nullptr, &error);
  if (error) {
    qLog(Error) << error->code << error->message;
  }
  if (cdda_ == nullptr) {
    model_->Reset();
    return;
  }

  g_object_set(cdda_, "device", g_strdup(url_.path().toLocal8Bit().constData()),
               nullptr);
  if (g_object_class_find_property (G_OBJECT_GET_CLASS (cdda_), "paranoia-mode")) {
    g_object_set (cdda_, "paranoia-mode", 0, NULL);
  }

  // Change the element's state to ready and paused, to be able to query it
  if (gst_element_set_state(cdda_, GST_STATE_READY) ==
          GST_STATE_CHANGE_FAILURE ||
      gst_element_set_state(cdda_, GST_STATE_PAUSED) ==
          GST_STATE_CHANGE_FAILURE) {
    model_->Reset();
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
    model_->Reset();
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
    song.set_url(
        QUrl(QString("cdda://%1/%2").arg(url_.path()).arg(track_number)));
    song.set_title(QString("Track %1").arg(track_number));
    song.set_track(track_number);
    songs << song;
  }
  song_count_ = num_tracks;
  connect(this, SIGNAL(SongsDiscovered(const SongList&)), model_,
          SLOT(SongsDiscovered(const SongList&)));
  // Reset the model before emitting the SongsDiscovered signal. This
  // ensures that the model is updated properly even when a disc that
  // doesn't exist in MusicBrainz is inserted.
  model_->Reset();
  emit SongsDiscovered(songs);


  gst_tag_register_musicbrainz_tags();

  GstElement* pipeline = gst_pipeline_new("pipeline");
  GstElement* sink = gst_element_factory_make ("fakesink", NULL);
  gst_bin_add_many (GST_BIN (pipeline), cdda_, sink, NULL);
  gst_element_link (cdda_, sink);
  gst_element_set_state(pipeline, GST_STATE_READY);
  gst_element_set_state(pipeline, GST_STATE_PAUSED);

  // Get TOC and TAG messages
  GstMessage* msg = nullptr;
  GstMessage* msg_toc = nullptr;
  GstMessage* msg_tag = nullptr;
  while ((msg = gst_bus_timed_pop_filtered(GST_ELEMENT_BUS(pipeline),
      GST_SECOND, (GstMessageType)(GST_MESSAGE_TOC | GST_MESSAGE_TAG)))) {
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_TOC) {
      if (msg_toc) gst_message_unref(msg_toc); // Shouldn't happen, but just in case
      msg_toc = msg;
    } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_TAG) {
      if (msg_tag) gst_message_unref(msg_tag);
      msg_tag = msg;
    }
  }

  // Handle TOC message: get tracks duration
  if (msg_toc) {
    GstToc* toc;
    gst_message_parse_toc (msg_toc, &toc, nullptr);
    if (toc) {
      GList* entries = gst_toc_get_entries(toc);
      if (entries && songs.size() <= g_list_length (entries)) {
        int i = 0;
        for (GList* node = entries; node != nullptr; node = node->next) {
          GstTocEntry *entry = static_cast<GstTocEntry*>(node->data);
          quint64 duration = 0;
          gint64 start, stop;
          if (gst_toc_entry_get_start_stop_times (entry, &start, &stop))
            duration = stop - start;
          songs[i++].set_length_nanosec(duration);
        }
      }
    }
    gst_message_unref(msg_toc);
  }
  // Update songs with duration
  model_->Reset();
  emit SongsDiscovered(songs);
  song_count_ = songs.size();

  // Handle TAG message: generate MusicBrainz DiscId
  if (msg_tag) {
    GstTagList* tags = nullptr;
    gst_message_parse_tag(msg_tag, &tags);
    char* string_mb = nullptr;
    if (gst_tag_list_get_string(tags, GST_TAG_CDDA_MUSICBRAINZ_DISCID,
                                &string_mb)) {
      QString musicbrainz_discid(string_mb);
      qLog(Info) << "MusicBrainz discid: " << musicbrainz_discid;

      MusicBrainzClient* musicbrainz_client = new MusicBrainzClient(this);
      connect(musicbrainz_client, SIGNAL(Finished(const QString&, const QString&,
                                                  MusicBrainzClient::ResultList)),
              SLOT(AudioCDTagsLoaded(const QString&, const QString&,
                                     MusicBrainzClient::ResultList)));
      musicbrainz_client->StartDiscIdRequest(musicbrainz_discid);
      g_free(string_mb);
      gst_message_unref(msg_tag);
      gst_tag_list_free(tags);
    }
  }

  gst_element_set_state(pipeline, GST_STATE_NULL);
  // This will also cause cdda_ to be unref'd.
  gst_object_unref(pipeline);
}

void CddaDevice::AudioCDTagsLoaded(
    const QString& artist, const QString& album,
    const MusicBrainzClient::ResultList& results) {
  MusicBrainzClient* musicbrainz_client =
      qobject_cast<MusicBrainzClient*>(sender());
  musicbrainz_client->deleteLater();
  SongList songs;
  int track_number = 1;
  if (results.size() == 0) return;
  model_->Reset();
  for (const MusicBrainzClient::Result& ret : results) {
    Song song;
    song.set_artist(artist);
    song.set_album(album);
    song.set_title(ret.title_);
    song.set_length_nanosec(ret.duration_msec_ * kNsecPerMsec);
    song.set_track(track_number);
    song.set_year(ret.year_);
    song.set_id(track_number);
    // We need to set url: that's how playlist will find the correct item to
    // update
    song.set_url(
        QUrl(QString("cdda://%1/%2").arg(unique_id()).arg(track_number++)));
    songs << song;
  }
  connect(this, SIGNAL(SongsDiscovered(const SongList&)), model_,
          SLOT(SongsDiscovered(const SongList&)));
  emit SongsDiscovered(songs);
  song_count_ = songs.size();
}

void CddaDevice::Refresh() {
  if ((cdio_ && cdda_) && /* already init... */
      cdio_get_media_changed(cdio_) !=
          1 /* ...and hasn't change since last time */) {
    return;
  }
  // Check if mutex is already token (i.e. init is already taking place)
  if (!mutex_init_.tryLock()) {
    return;
  }
  mutex_init_.unlock();
  Init();
}
