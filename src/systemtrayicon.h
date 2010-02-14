#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QSystemTrayIcon>

class SystemTrayIcon : public QSystemTrayIcon {
  Q_OBJECT

 public:
  SystemTrayIcon(QObject* parent = 0);
  virtual ~SystemTrayIcon();

  virtual bool event(QEvent* event);

 public slots:
  void SetProgress(int percentage);

 signals:
  void WheelEvent(int delta);

 private:
  QPixmap icon_;
  QPixmap grey_icon_;
};

#endif // SYSTEMTRAYICON_H
