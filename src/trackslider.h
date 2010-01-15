#ifndef TRACKSLIDER_H
#define TRACKSLIDER_H

#include <QWidget>

#include "ui_trackslider.h"

class TrackSlider : public QWidget {
  Q_OBJECT

 public:
  TrackSlider(QWidget* parent = 0);

  // QWidget
  QSize sizeHint() const;

  static QString PrettyTime(int seconds);

 public slots:
  void SetValue(int elapsed, int total);
  void SetStopped();
  void SetCanSeek(bool can_seek);

 signals:
  void ValueChanged(int value);

 private:
  Ui::TrackSlider ui_;
};

#endif // TRACKSLIDER_H
