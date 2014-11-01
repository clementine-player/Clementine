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

#ifndef CORE_URLHANDLER_H_
#define CORE_URLHANDLER_H_

#include <QIcon>
#include <QObject>
#include <QUrl>

class UrlHandler : public QObject {
  Q_OBJECT

 public:
  explicit UrlHandler(QObject* parent = nullptr);

  // The URL scheme that this handler handles.
  virtual QString scheme() const = 0;
  virtual QIcon icon() const;

  // Returned by StartLoading() and LoadNext(), indicates what the player
  // should do when it wants to load a URL.
  struct LoadResult {
    enum Type {
      // There wasn't a track available, and the player should move on to the
      // next playlist item.
      NoMoreTracks,

      // There might be another track available but the handler needs to do some
      // work (eg. fetching a remote playlist) to find out.  AsyncLoadComplete
      // will be emitted later with the same original_url.
      WillLoadAsynchronously,

      // There was a track available.  Its url is in media_url.
      TrackAvailable,
    };

    LoadResult(const QUrl& original_url = QUrl(), Type type = NoMoreTracks,
               const QUrl& media_url = QUrl(), qint64 length_nanosec_ = -1);

    // The url that the playlist item has in Url().
    // Might be something unplayable like lastfm://...
    QUrl original_url_;

    Type type_;

    // The actual url to something that gstreamer can play.
    QUrl media_url_;

    // Track length, if we are able to get it only now
    qint64 length_nanosec_;
  };

  // Called by the Player when a song starts loading - gives the handler
  // a chance to do something clever to get a playable track.
  virtual LoadResult StartLoading(const QUrl& url) { return LoadResult(url); }

  // Called by the player when a song finishes - gives the handler a chance to
  // get another track to play.
  virtual LoadResult LoadNext(const QUrl& url) { return LoadResult(url); }

  // Functions to be warned when something happen to a track handled by
  // UrlHandler.
  virtual void TrackAboutToEnd() {}
  virtual void TrackSkipped() {}

 signals:
  void AsyncLoadComplete(const UrlHandler::LoadResult& result);
};

#endif  // CORE_URLHANDLER_H_
