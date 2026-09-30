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

#ifndef CORE_MACMEDIACONTROLS_H_
#define CORE_MACMEDIACONTROLS_H_

#include <QObject>
#include <memory>

#include "core/song.h"
#include "engines/engine_fwd.h"

class Application;
class QImage;

// Makes Clementine the Mac's Now Playing app: the song, its cover and its
// progress in Control Center, on the lock screen and on the Touch Bar, and
// the media keys, AirPods and other headphone buttons controlling it. It's the
// Mac's counterpart to WindowsMediaControls.
//
// macOS sends the media keys to whichever app last said it was playing, so
// they reach Clementine even when another app is in front.
class MacMediaControls : public QObject {
  Q_OBJECT

 public:
  explicit MacMediaControls(Application* app, QObject* parent = nullptr);
  ~MacMediaControls();

 private slots:
  void EngineStateChanged(Engine::State state);
  void CurrentSongChanged(const Song& song);
  void ArtLoaded(const Song& song, const QString& uri, const QImage& image);
  void Seeked(qlonglong microseconds);

 private:
  enum Command { Play, Pause, TogglePlayPause, Stop, Next, Previous };
  struct Private;

  // Called on the GUI thread via a queued invocation from the command center.
  Q_INVOKABLE void HandleCommand(int command);
  Q_INVOKABLE void SeekTo(double seconds);

  // Sends the song, cover and position as they are now.
  void UpdateNowPlaying();

  Application* app_;
  Song song_;
  std::unique_ptr<Private> d_;
};

#endif  // CORE_MACMEDIACONTROLS_H_
