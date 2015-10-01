#ifndef CORE_SIGNALHANDLER_H
#define CORE_SIGNALHANDLER_H

#include <QObject>

class QSocketNotifier;

class SignalHandler : public QObject {
  Q_OBJECT
 public:
  explicit SignalHandler(QObject* parent = 0);

 signals:
  void SIG_USR1();

 private slots:
  void SignalReceived();

 private:
  QSocketNotifier* notifier_;
};

#endif  // CORE_SIGNALHANDLER_H
