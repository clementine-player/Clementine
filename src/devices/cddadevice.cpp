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
  cdda_ = gst_element_make_from_uri(GST_URI_SRC, "cdda://", nullptr);
  if (cdda_ == nullptr) {
    model_->Reset();
    return;
  }

  GST_CDDA_BASE_SRC(cdda_)->device =
      g_strdup(url_.path().toLocal8Bit().constData());

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
  if (!gst_element_query_duration(cdda_, &out_fmt, &num_tracks) ||
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
    guint64 duration = 0;
    // quint64 == ulonglong and guint64 == ulong, therefore we must cast
    if (gst_tag_list_get_uint64(
            GST_CDDA_BASE_SRC(cdda_)->tracks[track_number - 1].tags,
            GST_TAG_DURATION, &duration)) {
      song.set_length_nanosec((quint64)duration);
    }
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
  emit SongsDiscovered(songs);

  // Generate MusicBrainz DiscId
  gst_tag_register_musicbrainz_tags();
  GstElement* pipe = gst_pipeline_new("pipeline");
  gst_bin_add(GST_BIN(pipe), cdda_);
  gst_element_set_state(pipe, GST_STATE_READY);
  gst_element_set_state(pipe, GST_STATE_PAUSED);
  GstMessage* msg = gst_bus_timed_pop_filtered(
      GST_ELEMENT_BUS(pipe), GST_CLOCK_TIME_NONE, GST_MESSAGE_TAG);
  GstTagList* tags = nullptr;
  gst_message_parse_tag(msg, &tags);
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
  }

  // Clean all the Gstreamer objects we have used: we don't need them anymore
  gst_element_set_state(pipe, GST_STATE_NULL);
  // This will also cause cdda_ to be unref'd.
  gst_object_unref(GST_OBJECT(pipe));
  gst_object_unref(GST_OBJECT(msg));
  gst_tag_list_free(tags);
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
