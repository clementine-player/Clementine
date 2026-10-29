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

#include "core/sleepinhibitor.h"

#include <QMetaObject>
#include <QSettings>
#include <QTimer>
#include <QtGlobal>

#include "config.h"
#include "core/logging.h"

#if defined(Q_OS_DARWIN)
#include "core/macsleepinhibitor.h"
#elif defined(Q_OS_WIN32)
#include "core/windowssleepinhibitor.h"
#elif defined(HAVE_DBUS)
#include "core/dbussleepinhibitor.h"
#endif

namespace {

// In the order their reasons are preferred, when more than one needs the
// same thing.
const SleepInhibitor::Activity kActivities[] = {
    SleepInhibitor::Activity::FullScreenVisualizations,
    SleepInhibitor::Activity::RemoteTransfer,
    SleepInhibitor::Activity::PlaybackOnOtherDevice,
    SleepInhibitor::Activity::LocalPlayback,
    SleepInhibitor::Activity::WhileOpen,
};

const SleepInhibitor::Keep kKeeps[] = {
    SleepInhibitor::Keep::Awake,
    SleepInhibitor::Keep::DisplayOn,
};

}  // namespace

const char* SleepInhibitor::kSettingsGroup = "SleepInhibitor";
const int SleepInhibitor::kDefaultGraceMsec = 60000;

SleepInhibitor::Hold::Hold(SleepInhibitor* inhibitor, Activity activity)
    : inhibitor_(inhibitor), activity_(activity) {}

SleepInhibitor::Hold::~Hold() {
  SleepInhibitor* inhibitor = inhibitor_;
  Activity activity = activity_;
  // Runs straight away on the inhibitor's thread, and is queued from others.
  QMetaObject::invokeMethod(
      inhibitor, [inhibitor, activity]() { inhibitor->RemoveHold(activity); });
}

SleepInhibitor::SleepInhibitor(QObject* parent)
    : SleepInhibitor(CreatePlatformBackend(), parent) {}

SleepInhibitor::SleepInhibitor(std::unique_ptr<Backend> backend,
                               QObject* parent)
    : QObject(parent),
      backend_(std::move(backend)),
      grace_msec_(kDefaultGraceMsec) {
  // Held for as long as this exists, which is as long as Clementine runs.
  holds_[Activity::WhileOpen] = 1;
  ReloadSettings();
}

SleepInhibitor::~SleepInhibitor() {
  if (!backend_) return;
  for (Keep keep : kKeeps) {
    if (inhibited_.contains(keep)) backend_->Uninhibit(keep);
  }
}

std::unique_ptr<SleepInhibitor::Backend>
SleepInhibitor::CreatePlatformBackend() {
#if defined(Q_OS_DARWIN)
  return std::make_unique<MacSleepInhibitor>();
#elif defined(Q_OS_WIN32)
  return std::make_unique<WindowsSleepInhibitor>();
#elif defined(HAVE_DBUS)
  return std::make_unique<DBusSleepInhibitor>();
#else
  return nullptr;
#endif
}

std::unique_ptr<SleepInhibitor::Hold> SleepInhibitor::Acquire(
    Activity activity) {
  QMetaObject::invokeMethod(this, [this, activity]() { AddHold(activity); });
  return std::unique_ptr<Hold>(new Hold(this, activity));
}

SleepInhibitor::Keep SleepInhibitor::KeepFor(Activity activity) {
  switch (activity) {
    case Activity::FullScreenVisualizations:
      // They're watched rather than used, so nothing else keeps the display
      // on.
      return Keep::DisplayOn;
    case Activity::WhileOpen:
    case Activity::LocalPlayback:
    case Activity::PlaybackOnOtherDevice:
    case Activity::RemoteTransfer:
      return Keep::Awake;
  }
  return Keep::Awake;
}

bool SleepInhibitor::EnabledByDefault(Activity activity) {
  // Keeping the computer awake whenever Clementine's open is for those who
  // ask.
  return activity != Activity::WhileOpen;
}

QString SleepInhibitor::SettingsKey(Activity activity) {
  switch (activity) {
    case Activity::FullScreenVisualizations:
      return "full_screen_visualizations";
    case Activity::WhileOpen:
      return "while_open";
    case Activity::LocalPlayback:
      return "local_playback";
    case Activity::PlaybackOnOtherDevice:
      return "playback_on_other_device";
    case Activity::RemoteTransfer:
      return "remote_transfer";
  }
  return QString();
}

QString SleepInhibitor::Reason(Activity activity) {
  switch (activity) {
    case Activity::FullScreenVisualizations:
      // TRANSLATORS: Why Clementine is keeping the computer awake, which
      // the system shows in its list of what's stopping it from sleeping.
      return tr("Showing visualizations full screen");
    case Activity::WhileOpen:
      // TRANSLATORS: Why Clementine is keeping the computer awake, which
      // the system shows in its list of what's stopping it from sleeping.
      return tr("Keeping the computer awake while Clementine is open");
    case Activity::LocalPlayback:
      // TRANSLATORS: Why Clementine is keeping the computer awake, which
      // the system shows in its list of what's stopping it from sleeping.
      return tr("Playing music");
    case Activity::PlaybackOnOtherDevice:
      // TRANSLATORS: Why Clementine is keeping the computer awake, which
      // the system shows in its list of what's stopping it from sleeping.
      // The music is playing on a phone, a Chromecast or the like, which
      // gets it from this computer.
      return tr("Playing music on another device");
    case Activity::RemoteTransfer:
      // TRANSLATORS: Why Clementine is keeping the computer awake, which
      // the system shows in its list of what's stopping it from sleeping.
      // A phone running Clementine Remote is downloading songs from this
      // computer.
      return tr("Sending songs to a remote");
  }
  return QString();
}

void SleepInhibitor::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  for (Activity activity : kActivities) {
    enabled_[activity] =
        s.value(SettingsKey(activity), EnabledByDefault(activity)).toBool();
  }
  Update();
}

void SleepInhibitor::AddHold(Activity activity) {
  ++holds_[activity];
  Update();
}

void SleepInhibitor::RemoveHold(Activity activity) {
  if (KeepFor(activity) == Keep::Awake && grace_msec_ > 0) {
    // The computer only sleeps after minutes without anything to do, so a
    // gap between songs, or between files a remote downloads, needn't let it
    // go and ask again.
    QTimer::singleShot(grace_msec_, this, [this, activity]() {
      --holds_[activity];
      Update();
    });
    return;
  }
  --holds_[activity];
  Update();
}

void SleepInhibitor::Update() {
  QMap<Keep, QString> needed;
  for (Activity activity : kActivities) {
    if (holds_.value(activity) <= 0 || !enabled_.value(activity)) continue;
    Keep keep = KeepFor(activity);
    if (!needed.contains(keep)) needed[keep] = Reason(activity);
  }

  for (Keep keep : kKeeps) {
    const QString reason = needed.value(keep);
    if (reason == inhibited_.value(keep)) continue;

    // Let go first if the reason's changed, so the system shows the new one.
    if (inhibited_.contains(keep)) {
      qLog(Debug) << "No longer"
                  << (keep == Keep::DisplayOn ? "keeping the display on:"
                                              : "keeping awake:")
                  << inhibited_.value(keep);
      if (backend_) backend_->Uninhibit(keep);
      inhibited_.remove(keep);
    }
    if (!reason.isEmpty()) {
      qLog(Debug) << (keep == Keep::DisplayOn ? "Keeping the display on:"
                                              : "Keeping awake:")
                  << reason;
      if (backend_) backend_->Inhibit(keep, reason);
      inhibited_[keep] = reason;
    }
  }
}
