/* This file is part of Clementine.
   Copyright 2010-2013, David Sansome <me@davidsansome.com>
   Copyright 2010-2014, John Maguire <john.maguire@gmail.com>

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

#include "backgroundstreams.h"

#include <QAction>
#include <QSettings>
#include <QtDebug>

#include "core/logging.h"
#include "engines/enginebase.h"

const char* BackgroundStreams::kSettingsGroup = "BackgroundStreams";
const char* BackgroundStreams::kHypnotoadUrl = "hypnotoad:///";
const char* BackgroundStreams::kRainUrl =
    "http://data.clementine-player.org/rainymood";
const char* BackgroundStreams::kEnterpriseUrl = "enterprise:///";

BackgroundStreams::BackgroundStreams(EngineBase* engine, QObject* parent)
    : QObject(parent), engine_(engine) {}

BackgroundStreams::~BackgroundStreams() { SaveStreams(); }

void BackgroundStreams::LoadStreams() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  int version = s.value("version", 0).toInt();
  if (version < 1) {
    AddStream(QT_TR_NOOP("Hypnotoad"), QUrl(kHypnotoadUrl));
    AddStream(QT_TR_NOOP("Rain"), QUrl(kRainUrl));
  }

  if (version < kVersion) {
    s.setValue("version", kVersion);
    AddStream(QT_TR_NOOP("Make it so!"), QUrl(kEnterpriseUrl));
  }

  int size = s.beginReadArray("streams");
  for (int i = 0; i < size; ++i) {
    s.setArrayIndex(i);
    AddStream(s.value("name").toString(), s.value("url").toUrl(),
              s.value("volume").toInt());
  }

  SaveStreams();
}

void BackgroundStreams::SaveStreams() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  QList<Stream*> values = streams_.values();
  s.beginWriteArray("streams");
  for (int i = 0; i < values.size(); ++i) {
    s.setArrayIndex(i);
    Stream* stream = values[i];
    s.setValue("name", stream->name);
    s.setValue("url", stream->url);
    s.setValue("volume", stream->volume);
  }
  s.endArray();
}

void BackgroundStreams::AddStream(const QString& name, const QUrl& url,
                                  int volume) {
  if (streams_.contains(name)) {
    return;
  }

  Stream* s = new Stream;
  s->name = name;
  s->url = url;
  s->volume = volume;
  s->id = -1;
  streams_[name] = s;
}

void BackgroundStreams::EnableStream(const QString& name, bool enable) {
  if (!streams_.contains(name)) {
    qLog(Warning) << "Tried to toggle a stream" << name << "which didn't exist";
    return;
  }

  Stream* stream = streams_[name];
  if (enable == (stream->id != -1)) {
    return;
  }
  if (enable) {
    PlayStream(stream);
  } else {
    StopStream(stream);
  }
  SaveStreams();
}

void BackgroundStreams::SetStreamVolume(const QString& name, int volume) {
  Stream* stream = streams_[name];
  stream->volume = volume;
  if (stream->id != -1) {
    engine_->SetBackgroundStreamVolume(stream->id, stream->volume);
  }
}

void BackgroundStreams::PlayStream(Stream* stream) {
  stream->id = engine_->AddBackgroundStream(stream->url);
  engine_->SetBackgroundStreamVolume(stream->id, stream->volume);
  emit StreamStarted(stream->name);

  if (stream->action) {
    stream->action->setChecked(true);
  }
}

void BackgroundStreams::StopStream(Stream* stream) {
  engine_->StopBackgroundStream(stream->id);
  stream->id = -1;
  emit StreamStopped(stream->name);

  if (stream->action) {
    stream->action->setChecked(false);
  }
}

int BackgroundStreams::GetStreamVolume(const QString& name) const {
  return streams_[name]->volume;
}

bool BackgroundStreams::IsPlaying(const QString& name) const {
  return streams_[name]->id != -1;
}

void BackgroundStreams::AddAction(const QString& name, QAction* action) {
  if (!streams_.contains(name)) {
    qLog(Error) << "Tried to add action for stream" << name
                << "which doesn't exist";
    return;
  }

  Stream* stream = streams_[name];
  if (stream->action) {
    qLog(Error) << "Tried to add multiple actions for stream" << name;
    return;
  }

  stream->action = action;
  action->setChecked(IsPlaying(name));
  connect(action, SIGNAL(toggled(bool)), SLOT(StreamActionToggled(bool)));
  connect(action, SIGNAL(destroyed()), SLOT(StreamActionDestroyed()));
}

void BackgroundStreams::StreamActionDestroyed() {
  QAction* action = static_cast<QAction*>(sender());
  if (!action) {
    return;
  }

  for (Stream* stream : streams_.values()) {
    if (stream->action == action) {
      stream->action = nullptr;
    }
  }
}

void BackgroundStreams::StreamActionToggled(bool checked) {
  QAction* action = static_cast<QAction*>(sender());
  if (!action) {
    return;
  }

  for (Stream* stream : streams_.values()) {
    if (stream->action == action) {
      EnableStream(stream->name, checked);
    }
  }
}
