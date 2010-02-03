#include "multiloadingindicator.h"

MultiLoadingIndicator::MultiLoadingIndicator(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);
}

void MultiLoadingIndicator::TaskStarted(const QString &name) {
  if (tasks_.contains(name))
    return;

  tasks_ << name;

  UpdateText();
  show();
}

void MultiLoadingIndicator::TaskFinished(const QString &name) {
  tasks_.removeAll(name);

  UpdateText();
  if (tasks_.count() == 0)
    hide();
}

void MultiLoadingIndicator::UpdateText() {
  QStringList strings;
  foreach (QString task, tasks_) {
    if (task.isEmpty())
      continue;

    task[0] = task[0].toLower();
    strings << task;
  }

  QString text(strings.join(", "));
  if (!text.isEmpty()) {
    text[0] = text[0].toUpper();
  }

  ui_.text->setText(text + "...");
}
