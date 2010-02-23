#ifndef MULTILOADINGINDICATOR_H
#define MULTILOADINGINDICATOR_H

#include <QWidget>

#include "ui_multiloadingindicator.h"

class MultiLoadingIndicator : public QWidget {
  Q_OBJECT

 public:
  MultiLoadingIndicator(QWidget* parent = 0);

  enum TaskType {
    LoadingAudioEngine,
    UpdatingLibrary,
    GettingChannels,
    LoadingStream,
    LoadingLastFM,
  };

 public slots:
  void TaskStarted(MultiLoadingIndicator::TaskType type);
  void TaskFinished(MultiLoadingIndicator::TaskType type);

 private:
  void UpdateText();
  static QString TaskTypeToString(TaskType type);

 private:
  Ui::MultiLoadingIndicator ui_;

  QList<TaskType> tasks_;
};

#endif // MULTILOADINGINDICATOR_H
