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

#ifndef UI_SCREENSHOTTAKER_H_
#define UI_SCREENSHOTTAKER_H_

#include <QDir>
#include <QImage>
#include <QList>
#include <QObject>
#include <QUrl>
#include <functional>

class Application;
class MainWindow;
class QWidget;

// Takes the screenshots CI posts on pull requests (--screenshots <dir>): the
// main window on each of its tabs and every settings page, in the theme
// --screenshot-theme (light or dark) started it in.
// The music given on the command line is added to the library as well as the
// playlist, and the playlist's first song is paused a third of the way in.
//
// It changes the library and settings of the profile it runs in, so it's
// meant for CI runners and throwaway profiles. The files are named for the
// order they were taken in; the dark theme's start with dark_. One theme a
// run, from the start, is what people see: switching themes while Clementine
// runs doesn't yet repaint everything.
class ScreenshotTaker : public QObject {
  Q_OBJECT

 public:
  ScreenshotTaker(Application* app, MainWindow* window, const QString& dir,
                  const QList<QUrl>& music, QObject* parent = nullptr);

  // Plays through a sink that keeps time without a sound card, so the song
  // stays where it's paused. Call before the player starts.
  static void UseSilentSink();

 public slots:
  // Exits the application when it's done: 0 if every screenshot was saved,
  // 1 if some weren't, and 2 if the music never loaded.
  void Run();

 private:
  void Wait(int msec);
  bool WaitFor(const char* what, int timeout_msec,
               const std::function<bool()>& done);
  // Waits until |value| has stayed the same, and passed |ready|, for a while.
  bool WaitUntilSettled(const char* what, int timeout_msec,
                        const std::function<int()>& value,
                        const std::function<bool(int)>& ready);

  bool LoadMusic();
  void PausePartWay();
  void TakeAll(const QString& prefix);
  void TakeMainWindow(const QString& prefix);
  void TakeSettings(const QString& prefix);
  // Sizes |widget| as close to |size| as fits on its screen, frame and all,
  // and puts it in the top left corner.
  void Place(QWidget* widget, const QSize& size);
  void Save(QWidget* widget, const QString& name);
  // The window with its frame, or a null image where that can't be had.
  QImage CaptureFrame(QWidget* widget);

  static QString Slug(const QString& text);

  Application* app_;
  MainWindow* window_;
  QDir dir_;
  QList<QUrl> music_;
  int failures_;
};

#endif  // UI_SCREENSHOTTAKER_H_
