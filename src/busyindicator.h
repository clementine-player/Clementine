#ifndef BUSYINDICATOR_H
#define BUSYINDICATOR_H

#include <QLabel>

class QMovie;

class BusyIndicator : public QLabel {
  Q_OBJECT

 public:
  BusyIndicator(QWidget* parent = 0);
  ~BusyIndicator();

 protected:
  void showEvent(QShowEvent* event);
  void hideEvent(QHideEvent* event);

 private:
  QMovie* movie_;
};

#endif // BUSYINDICATOR_H
