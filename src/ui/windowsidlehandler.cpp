/* This file is part of Clementine.
   Copyright 2015, John Maguire <john.maguire@gmail.com>
   Copyright 2015 - 2016, Arun Narayanankutty <n.arun.lifescience@gmail.com>

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

#include "windowsidlehandler.h"

#include "core/logging.h"

// TODO: use PowerCreateRequest on Win7+

WindowsIdleHandler::WindowsIdleHandler() : previous_state_(0), 
                                           is_inhibit_(false) {}

void WindowsIdleHandler::Inhibit(const char*) {
  switch (IdleHandler::inbtr_) {
    case IdleHandler::Screensaver :
      // resetting the display idle timer.
      previous_state_ =
          SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
      break;
    case IdleHandler::Suspend :
      // resetting the system idle timer.
      previous_state_ =
          SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
      break;
  }

  if (previous_state_ != 0) {
    is_inhibit_ = true;
  } else {
    qLog(Warning) << "Failed to inhibit screensaver/suspend";
  }
}

void WindowsIdleHandler::Uninhibit() {
  previous_state_ = 
      SetThreadExecutionState(ES_CONTINUOUS | previous_state_);
  if (previous_state_ != 0) {
    is_inhibit_ = false;
  } else {
    qLog(Warning) << "Failed to uninhibit screensaver/suspend";
  }
}

bool WindowsIdleHandler::Isinhibited() {
  return is_inhibit_;
}
