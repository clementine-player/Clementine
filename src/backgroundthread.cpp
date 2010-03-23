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
