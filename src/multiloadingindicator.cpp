#include "multiloadingindicator.h"

MultiLoadingIndicator::MultiLoadingIndicator(QWidget *parent)
  : QWidget(parent)
{
  ui_.setupUi(this);
}

void MultiLoadingIndicator::TaskStarted(TaskType type) {
  if (tasks_.contains(type))
    return;

  tasks_ << type;

  UpdateText();
  show();
}

void MultiLoadingIndicator::TaskFinished(TaskType type) {
  tasks_.removeAll(type);

  UpdateText();
  if (tasks_.count() == 0)
    hide();
}

void MultiLoadingIndicator::UpdateText() {
  QStringList strings;
  foreach (TaskType type, tasks_) {
    QString task(TaskTypeToString(type));
    task[0] = task[0].toLower();
    strings << task;
  }

  QString text(strings.join(", "));
  if (!text.isEmpty()) {
    text[0] = text[0].toUpper();
  }

  ui_.text->setText(text + "...");
}

QString MultiLoadingIndicator::TaskTypeToString(TaskType type) {
  switch (type) {
    case LoadingAudioEngine: return tr("Loading audio engine");
    case UpdatingLibrary:    return tr("Updating library");
    case GettingChannels:    return tr("Getting channels");
    case LoadingStream:      return tr("Loading stream");
    case LoadingLastFM:      return tr("Loading Last.fm radio");

    default: return QString::null;
  }
}
