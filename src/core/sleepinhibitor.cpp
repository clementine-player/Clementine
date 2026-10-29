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

const SleepInhibitor::Activity kActivities[] = {
    SleepInhibitor::Activity::FullScreenVisualizations,
};

const SleepInhibitor::Keep kKeeps[] = {
    SleepInhibitor::Keep::Awake,
    SleepInhibitor::Keep::DisplayOn,
};

}  // namespace

const char* SleepInhibitor::kSettingsGroup = "SleepInhibitor";

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
    : QObject(parent), backend_(std::move(backend)) {
  ReloadSettings();
}

SleepInhibitor::~SleepInhibitor() {
  if (!backend_) return;
  for (Keep keep : kKeeps) {
    if (inhibited_.value(keep)) backend_->Uninhibit(keep);
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
  }
  return Keep::Awake;
}

QString SleepInhibitor::SettingsKey(Activity activity) {
  switch (activity) {
    case Activity::FullScreenVisualizations:
      return "full_screen_visualizations";
  }
  return QString();
}

QString SleepInhibitor::Reason(Activity activity) {
  switch (activity) {
    case Activity::FullScreenVisualizations:
      return tr("Showing visualizations full screen");
  }
  return QString();
}

void SleepInhibitor::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  for (Activity activity : kActivities) {
    enabled_[activity] = s.value(SettingsKey(activity), true).toBool();
  }
  Update();
}

void SleepInhibitor::AddHold(Activity activity) {
  ++holds_[activity];
  Update();
}

void SleepInhibitor::RemoveHold(Activity activity) {
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
    bool wanted = needed.contains(keep);
    if (wanted == inhibited_.value(keep)) continue;
    inhibited_[keep] = wanted;

    qLog(Debug) << (wanted ? "Keeping" : "No longer keeping")
                << (keep == Keep::DisplayOn ? "the display on" : "awake");
    if (!backend_) continue;
    if (wanted) {
      backend_->Inhibit(keep, needed[keep]);
    } else {
      backend_->Uninhibit(keep);
    }
  }
}
