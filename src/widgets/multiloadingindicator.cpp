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

MultiLoadingIndicator::MultiLoadingIndicator(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_MultiLoadingIndicator)
{
  ui_->setupUi(this);
}

MultiLoadingIndicator::~MultiLoadingIndicator() {
  delete ui_;
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

  ui_->text->setText(text + "...");
}

QString MultiLoadingIndicator::TaskTypeToString(TaskType type) {
  switch (type) {
    case LoadingAudioEngine: return tr("Loading audio engine");
    case UpdatingLibrary:    return tr("Updating library");
    case GettingChannels:    return tr("Getting channels");
    case LoadingStream:      return tr("Loading stream");
    case LoadingLastFM:      return tr("Loading Last.fm radio");
    case LoadingMagnatune:   return tr("Downloading Magnatune catalogue");
    case LoadingTracks:      return tr("Loading tracks");

    default: return QString::null;
  }
}
