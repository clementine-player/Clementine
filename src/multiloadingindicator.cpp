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
  ui_.text->setText(tasks_.join(", ") + "...");
}
