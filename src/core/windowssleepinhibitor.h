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

#ifndef CORE_WINDOWSSLEEPINHIBITOR_H_
#define CORE_WINDOWSSLEEPINHIBITOR_H_

#include <windows.h>

#include <QMap>

#include "core/sleepinhibitor.h"

// Makes a power request, which `powercfg /requests` lists. Unlike
// SetThreadExecutionState, a request doesn't belong to the thread that made
// it.
class WindowsSleepInhibitor : public SleepInhibitor::Backend {
 public:
  ~WindowsSleepInhibitor();

  void Inhibit(SleepInhibitor::Keep keep, const QString& reason) override;
  void Uninhibit(SleepInhibitor::Keep keep) override;

 private:
  QMap<SleepInhibitor::Keep, HANDLE> requests_;
};

#endif  // CORE_WINDOWSSLEEPINHIBITOR_H_
