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

  // QObject
  bool event(QEvent *);

  static QString PrettyTime(int seconds);

 public slots:
  void SetValue(int elapsed, int total);
  void SetStopped();
  void SetCanSeek(bool can_seek);

 signals:
  void ValueChanged(int value);

 private slots:
  void ValueMaybeChanged(int value);

 private:
  void UpdateTimes(int elapsed);
  void UpdateLabelWidth();

 private:
  Ui::TrackSlider ui_;

  bool setting_value_;
};

#endif // TRACKSLIDER_H
