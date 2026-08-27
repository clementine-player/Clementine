/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "lastfmscrobblecache.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/logging.h"
#include "core/utilities.h"

LastFmScrobbleCache::LastFmScrobbleCache(const QString& username)
    : username_(username) {
  Load();
}

QString LastFmScrobbleCache::FilePath() const {
  return Utilities::GetConfigPath(Utilities::Path_Root) +
         "/lastfmscrobblecache_" + username_ + ".json";
}

void LastFmScrobbleCache::Load() {
  tracks_.clear();

  QFile file(FilePath());
  if (!file.open(QIODevice::ReadOnly)) return;

  const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  for (const QJsonValue& value : doc.array()) {
    const QJsonObject obj = value.toObject();
    LastFmTrack track;
    track.artist = obj["artist"].toString();
    track.album_artist = obj["album_artist"].toString();
    track.album = obj["album"].toString();
    track.title = obj["title"].toString();
    track.track_number = obj["track_number"].toInt();
    track.duration_secs = obj["duration_secs"].toInt();
    track.timestamp =
        QDateTime::fromSecsSinceEpoch(obj["timestamp"].toInteger());
    track.source =
        obj["source"].toInt() == LastFmTrack::NonPersonalisedBroadcast
            ? LastFmTrack::NonPersonalisedBroadcast
            : LastFmTrack::Player;
    tracks_ << track;
  }
}

void LastFmScrobbleCache::Save() const {
  QJsonArray array;
  for (const LastFmTrack& track : tracks_) {
    QJsonObject obj;
    obj["artist"] = track.artist;
    obj["album_artist"] = track.album_artist;
    obj["album"] = track.album;
    obj["title"] = track.title;
    obj["track_number"] = track.track_number;
    obj["duration_secs"] = track.duration_secs;
    obj["timestamp"] = track.timestamp.toSecsSinceEpoch();
    obj["source"] = static_cast<int>(track.source);
    array << obj;
  }

  const QString path = FilePath();
  QDir().mkpath(Utilities::GetConfigPath(Utilities::Path_Root));

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qLog(Warning) << "Couldn't write Last.fm scrobble cache to" << path;
    return;
  }
  file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

void LastFmScrobbleCache::Add(const LastFmTrack& track) {
  tracks_ << track;
  Save();
}

void LastFmScrobbleCache::RemoveFirst(int count) {
  tracks_.erase(tracks_.begin(),
                tracks_.begin() + qMin(count, tracks_.count()));
  Save();
}
