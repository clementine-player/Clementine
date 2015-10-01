#include "core/signalhandler.h"

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include <QByteArray>
#include <QSocketNotifier>

#include "core/logging.h"

SignalHandler::SignalHandler(QObject* parent)
    : QObject(parent) {
  sigset_t mask;
  memset(&mask, 0, sizeof(mask));
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  // Listen to the handled signals in a file descriptor.
  const int fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
  if (fd == -1) {
    perror(Q_FUNC_INFO);
    qLog(Fatal) << "Failed to register signal handler with signalfd()";
  }
  notifier_ = new QSocketNotifier(fd, QSocketNotifier::Read, this);
  connect(notifier_, SIGNAL(activated(int)), SLOT(SignalReceived()));
}

void SignalHandler::SignalReceived() {
  qLog(Debug) << Q_FUNC_INFO;
  QByteArray buffer(sizeof(signalfd_siginfo), 0);
  // read() should fail with EAGAIN if there are no more signals.
  forever {
    int bytes_read = read(notifier_->socket(), buffer.data(), buffer.size());
    if (bytes_read == buffer.size()) {
      signalfd_siginfo* siginfo =
          reinterpret_cast<signalfd_siginfo*>(buffer.data());
      qLog(Debug) << Q_FUNC_INFO
                  << siginfo->ssi_signo << strsignal(siginfo->ssi_signo);;
      switch (siginfo->ssi_signo) {
        case SIGUSR1:
          emit SIG_USR1();
          break;
        // Add more signals here.
        default:
          break;
      }
    } else if (bytes_read <= 0) {
      break;
    }
  }
}
