/* This file is part of Clementine.
   Copyright 2026, David Sansome <me@davidsansome.com>

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

#ifndef CORE_WINDOWSMEDIACONTROLS_H_
#define CORE_WINDOWSMEDIACONTROLS_H_

#include <QObject>
#include <memory>

#include "engines/engine_fwd.h"

class Application;
class QImage;
class Song;

// Integrates with the Windows System Media Transport Controls (SMTC), the
// media session shown in the volume flyout and lock screen.  Media buttons on
// Bluetooth headphones (AVRCP) are delivered through SMTC, so without this
// they don't reach Clementine.
//
// SMTC is bound to a window handle.  We create our own hidden window for it
// rather than using the main window's, because Qt can destroy and recreate a
// top-level window's native handle, which would silently disconnect SMTC.
class WindowsMediaControls : public QObject {
  Q_OBJECT

 public:
  explicit WindowsMediaControls(Application* app, QObject* parent = nullptr);
  ~WindowsMediaControls();

 private slots:
  void EngineStateChanged(Engine::State state);
  void CurrentSongChanged(const Song& song);
  void ArtLoaded(const Song& song, const QString& uri, const QImage& image);

 private:
  class ButtonPressedHandler;
  struct Private;

  // Called on the GUI thread via a queued invocation from the SMTC thread.
  Q_INVOKABLE void ButtonPressed(int button);

  void UpdateMetadata(const Song& song, const QImage& image);

  Application* app_;
  std::unique_ptr<Private> d_;
};

#endif  // CORE_WINDOWSMEDIACONTROLS_H_
