/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#ifndef GROOVESHARKURLHANDLER_H
#define GROOVESHARKURLHANDLER_H

#include "core/urlhandler.h"

class GroovesharkService;
class QTimer;

class GroovesharkUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  GroovesharkUrlHandler(GroovesharkService* service, QObject* parent);

  QString scheme() const { return "grooveshark"; }
  QIcon icon() const { return QIcon(":providers/grooveshark.png"); }
  LoadResult StartLoading(const QUrl& url);
  void TrackAboutToEnd();
  void TrackSkipped();

 private slots:
  void MarkStreamKeyOver30Secs();

 private:
  GroovesharkService* service_;
  QTimer* timer_mark_stream_key_;
  QString last_artist_id_;
  QString last_album_id_;
  QString last_song_id_;
  QString last_server_id_;
  QString last_stream_key_;
};

#endif  // GROOVESHARKURLHANDLER_H
