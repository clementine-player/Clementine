#ifndef MULTILOADINGINDICATOR_H
#define MULTILOADINGINDICATOR_H

#include <QWidget>

#include "ui_multiloadingindicator.h"

class MultiLoadingIndicator : public QWidget {
  Q_OBJECT

 public:
  MultiLoadingIndicator(QWidget* parent = 0);

 public slots:
  void TaskStarted(const QString& name);
  void TaskFinished(const QString& name);

 private:
  void UpdateText();

 private:
  Ui::MultiLoadingIndicator ui_;

  QStringList tasks_;
};

#endif // MULTILOADINGINDICATOR_H
