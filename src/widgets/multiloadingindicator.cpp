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

#include "multiloadingindicator.h"
#include "ui_multiloadingindicator.h"
#include "core/taskmanager.h"

MultiLoadingIndicator::MultiLoadingIndicator(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_MultiLoadingIndicator)
{
  ui_->setupUi(this);
}

MultiLoadingIndicator::~MultiLoadingIndicator() {
  delete ui_;
}

void MultiLoadingIndicator::SetTaskManager(TaskManager* task_manager) {
  task_manager_ = task_manager;
  connect(task_manager_, SIGNAL(TasksChanged()), SLOT(UpdateText()));
}

void MultiLoadingIndicator::UpdateText() {
  QList<TaskManager::Task> tasks = task_manager_->GetTasks();

  QStringList strings;
  foreach (const TaskManager::Task& task, tasks) {
    QString name(task.name);
    name[0] = name[0].toLower();
    strings << name;
  }

  QString text(strings.join(", "));
  if (!text.isEmpty()) {
    text[0] = text[0].toUpper();
  }

  ui_->text->setText(text + "...");
  emit TaskCountChange(tasks.count());
}
