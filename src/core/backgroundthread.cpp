/* This file is part of Clementine.

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

#include "backgroundthread.h"

int BackgroundThreadBase::SetIOPriority() {
#ifdef Q_OS_LINUX
  return syscall(SYS_ioprio_set, IOPRIO_WHO_PROCESS, gettid(),
                 4 | io_priority_ << IOPRIO_CLASS_SHIFT);
#elif defined(Q_OS_DARWIN)
  return setpriority(PRIO_DARWIN_THREAD, 0,
                     io_priority_ == IOPRIO_CLASS_IDLE ? PRIO_DARWIN_BG : 0);
#else
  return 0;
#endif
}

int BackgroundThreadBase::gettid() {
#ifdef Q_OS_LINUX
  return syscall(SYS_gettid);
#else
  return 0;
#endif
}
