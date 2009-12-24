#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QSystemTrayIcon>

class SystemTrayIcon : public QSystemTrayIcon {
  Q_OBJECT

 public:
  SystemTrayIcon(QObject* parent = 0);

  bool event(QEvent* event);

 signals:
  void WheelEvent(int delta);
};

#endif // SYSTEMTRAYICON_H
