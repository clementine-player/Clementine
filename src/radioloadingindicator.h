#ifndef RADIOLOADINGINDICATOR_H
#define RADIOLOADINGINDICATOR_H

#include <QWidget>

#include "ui_radioloadingindicator.h"

class RadioLoadingIndicator : public QWidget {
  Q_OBJECT
 public:
  RadioLoadingIndicator(QWidget *parent = 0);

  void paintEvent(QPaintEvent *);

 private:
  Ui::RadioLoadingIndicator ui_;
};

#endif // RADIOLOADINGINDICATOR_H
