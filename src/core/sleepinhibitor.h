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

#ifndef CORE_SLEEPINHIBITOR_H_
#define CORE_SLEEPINHIBITOR_H_

#include <QMap>
#include <QObject>
#include <QString>
#include <memory>

// Keeps the computer from going to sleep, or its display from turning off,
// while something Clementine is doing needs it. Each activity can be turned
// off in the settings. Only idle sleep is prevented: the user can still
// suspend the computer themselves.
class SleepInhibitor : public QObject {
  Q_OBJECT

 public:
  static const char* kSettingsGroup;
  // How long the computer stays awake for an activity after its last hold
  // goes, unless that was keeping the display on.
  static const int kDefaultGraceMsec;

  // What Clementine is doing that the computer should stay awake for.
  enum class Activity {
    // Held all the time, so the computer never sleeps while Clementine's
    // open. Off unless the user turns it on.
    WhileOpen,
    // Playing through this computer's own output.
    LocalPlayback,
    // Playing on a phone, Chromecast or UPnP renderer, which gets the music
    // from this computer.
    PlaybackOnOtherDevice,
    // A remote downloading songs, for syncing or otherwise.
    RemoteTransfer,
    FullScreenVisualizations,
  };

  // How much of the computer to keep awake.
  enum class Keep {
    // The computer stays awake, but the display can turn off.
    Awake,
    // The display stays on, and with it the computer.
    DisplayOn,
  };

  // Asks the system. There's one for each platform.
  class Backend {
   public:
    virtual ~Backend() {}

    // |reason| is shown by the system's tools that list what's keeping the
    // computer awake. Each Keep is inhibited and uninhibited separately, and
    // Inhibit is called at most once before each Uninhibit.
    virtual void Inhibit(Keep keep, const QString& reason) = 0;
    virtual void Uninhibit(Keep keep) = 0;
  };

  // Keeps the computer awake for an activity until it's deleted. It can be
  // deleted on any thread, but not after the SleepInhibitor.
  class Hold {
   public:
    ~Hold();

   private:
    friend class SleepInhibitor;
    Hold(SleepInhibitor* inhibitor, Activity activity);

    SleepInhibitor* inhibitor_;
    Activity activity_;

    Q_DISABLE_COPY(Hold)
  };

  // Uses this platform's backend. Where there isn't one, holds do nothing.
  explicit SleepInhibitor(QObject* parent = nullptr);
  // Takes ownership of |backend|, which can be null.
  explicit SleepInhibitor(std::unique_ptr<Backend> backend,
                          QObject* parent = nullptr);
  ~SleepInhibitor();

  // Can be called on any thread.
  std::unique_ptr<Hold> Acquire(Activity activity);

  // For tests.
  void SetGraceMsec(int msec) { grace_msec_ = msec; }

  static Keep KeepFor(Activity activity);
  static bool EnabledByDefault(Activity activity);
  static QString SettingsKey(Activity activity);
  static QString Reason(Activity activity);

 public slots:
  void ReloadSettings();

 private:
  void AddHold(Activity activity);
  void RemoveHold(Activity activity);
  // Asks the backend for what the enabled activities that are held need, and
  // lets go of what they don't.
  void Update();

  static std::unique_ptr<Backend> CreatePlatformBackend();

  std::unique_ptr<Backend> backend_;
  int grace_msec_;
  QMap<Activity, int> holds_;
  QMap<Activity, bool> enabled_;
  // Why each Keep the backend's been asked for was asked for.
  QMap<Keep, QString> inhibited_;
};

#endif  // CORE_SLEEPINHIBITOR_H_
