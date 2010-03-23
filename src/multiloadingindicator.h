/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

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
