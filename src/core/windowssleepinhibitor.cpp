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

#include "core/windowssleepinhibitor.h"

#include <string>

#include "core/logging.h"

WindowsSleepInhibitor::~WindowsSleepInhibitor() {
  for (HANDLE request : requests_) {
    CloseHandle(request);
  }
}

void WindowsSleepInhibitor::Inhibit(SleepInhibitor::Keep keep,
                                    const QString& reason) {
  std::wstring reason_string = reason.toStdWString();
  REASON_CONTEXT context;
  context.Version = POWER_REQUEST_CONTEXT_VERSION;
  context.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
  context.Reason.SimpleReasonString = &reason_string[0];

  HANDLE request = PowerCreateRequest(&context);
  if (request == INVALID_HANDLE_VALUE) {
    qLog(Warning) << "Couldn't keep the computer awake:" << GetLastError();
    return;
  }
  PowerSetRequest(request, PowerRequestSystemRequired);
  if (keep == SleepInhibitor::Keep::DisplayOn) {
    PowerSetRequest(request, PowerRequestDisplayRequired);
  }
  requests_[keep] = request;
}

void WindowsSleepInhibitor::Uninhibit(SleepInhibitor::Keep keep) {
  if (!requests_.contains(keep)) return;
  HANDLE request = requests_.take(keep);
  PowerClearRequest(request, PowerRequestSystemRequired);
  if (keep == SleepInhibitor::Keep::DisplayOn) {
    PowerClearRequest(request, PowerRequestDisplayRequired);
  }
  CloseHandle(request);
}
