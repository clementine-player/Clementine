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

#ifndef INTERNET_PLEX_PLEXPLAYBACKREPORTER_H_
#define INTERNET_PLEX_PLEXPLAYBACKREPORTER_H_

#include <QObject>
#include <QString>

class QTimer;
class Application;
class PlexService;
class Song;

// Tells the Plex server what we're playing, so it shows up under Now Playing,
// and marks tracks as played once they pass the play count point.
class PlexPlaybackReporter : public QObject {
  Q_OBJECT

 public:
  PlexPlaybackReporter(PlexService* service, Application* app,
                       QObject* parent = nullptr);

 private slots:
  void Playing();
  void Paused();
  void Stopped();
  void CurrentSongChanged(const Song& song);
  void Tick();

 private:
  // Works out which Plex track is current and reports state for it.
  void Update(const QString& state);
  QString CurrentRatingKey() const;
  void SendTimeline(const QString& state);
  void MaybeScrobble(qint64 position_nanosec);

  PlexService* service_;
  Application* app_;
  QTimer* timer_;

  QString rating_key_;  // Empty if the current track isn't from Plex.
  qint64 time_msec_;
  qint64 duration_msec_;
  bool scrobbled_;
};

#endif  // INTERNET_PLEX_PLEXPLAYBACKREPORTER_H_
