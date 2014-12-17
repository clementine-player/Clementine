/* This file is part of Clementine.
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "groovesharkurlhandler.h"

#include <QTimer>

#include "groovesharkservice.h"
#include "core/logging.h"

GroovesharkUrlHandler::GroovesharkUrlHandler(GroovesharkService* service,
                                             QObject* parent)
    : UrlHandler(parent),
      service_(service),
      timer_mark_stream_key_(new QTimer(this)) {
  // We have to warn Grooveshark when user has listened for more than 30
  // seconds of a song, and when it ends. I guess this is used by Grooveshark
  // for statistics and user history.
  // To do this, we have TrackAboutToEnd method, and timer_mark_stream_key_
  // timer.
  // It is not perfect, as we may call Grooveshark MarkStreamKeyOver30Secs even
  // if user hasn't actually listen to 30 seconds (e.g. stream set to pause
  // state) but this is not a big deal and it should be accurate enough anyway.
  timer_mark_stream_key_->setInterval(30000);
  timer_mark_stream_key_->setSingleShot(true);
  connect(timer_mark_stream_key_, SIGNAL(timeout()),
          SLOT(MarkStreamKeyOver30Secs()));
}

UrlHandler::LoadResult GroovesharkUrlHandler::StartLoading(const QUrl& url) {
  qint64 length_nanosec = 0;
  QUrl streaming_url;
  QStringList ids = url.toString().remove("grooveshark://").split("/");
  if (ids.size() < 3) {
    qLog(Error) << "Invalid grooveshark URL: " << url.toString();
    qLog(Error) << "Should be grooveshark://artist_id/album_id/song_id";
  } else {
    last_artist_id_ = ids[0];
    last_album_id_ = ids[1];
    last_song_id_ = ids[2];

    streaming_url = service_->GetStreamingUrlFromSongId(
        last_song_id_, last_artist_id_, &last_server_id_, &last_stream_key_,
        &length_nanosec);
    qLog(Debug) << "Grooveshark Streaming URL: " << streaming_url;

    timer_mark_stream_key_->start();
  }

  return LoadResult(url, LoadResult::TrackAvailable, streaming_url,
                    length_nanosec);
}

void GroovesharkUrlHandler::TrackAboutToEnd() {
  if (timer_mark_stream_key_->isActive()) {
    timer_mark_stream_key_->stop();
    return;
  }
  service_->MarkSongComplete(last_song_id_, last_stream_key_, last_server_id_);
}

void GroovesharkUrlHandler::TrackSkipped() { timer_mark_stream_key_->stop(); }

void GroovesharkUrlHandler::MarkStreamKeyOver30Secs() {
  service_->MarkStreamKeyOver30Secs(last_stream_key_, last_server_id_);
}
