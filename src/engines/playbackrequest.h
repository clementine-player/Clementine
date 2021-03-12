/* This file is part of Clementine.
   Copyright 2020, Jim Broadus <jbroadus@gmail.com>

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

#ifndef ENGINES_PLAYBACKREQUEST_H_
#define ENGINES_PLAYBACKREQUEST_H_

#include <QMap>
#include <QUrl>

class MediaPlaybackRequest {
 public:
  // For local songs and raw streams, the request and media URLs are the same.
  MediaPlaybackRequest(const QUrl& url) : request_url_(url), url_(url) {}
  MediaPlaybackRequest(const QUrl& request_url, const QUrl& media_url)
      : request_url_(request_url), url_(media_url) {}
  MediaPlaybackRequest() {}

  const QUrl& RequestUrl() const { return request_url_; }
  const QUrl& MediaUrl() const { return url_; }
  void SetMediaUrl(const QUrl& url) { url_ = url; }

  QUrl request_url_;
  QUrl url_;

  typedef QMap<QByteArray, QByteArray> HeaderList;
  HeaderList headers_;
};

#endif  // ENGINES_PLAYBACKREQUEST_H_
