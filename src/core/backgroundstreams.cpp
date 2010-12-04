#include "backgroundstreams.h"

#include <QSettings>
#include <QtDebug>

#include "engines/enginebase.h"

const char* BackgroundStreams::kSettingsGroup = "BackgroundStreams";
const char* BackgroundStreams::kHypnotoadUrl = "hypnotoad:///";
const char* BackgroundStreams::kRainUrl = "http://data.clementine-player.org/rainymood";

BackgroundStreams::BackgroundStreams(EngineBase* engine, QObject* parent)
    : QObject(parent),
      engine_(engine) {
}

BackgroundStreams::~BackgroundStreams() {
  SaveStreams();
}

void BackgroundStreams::LoadStreams() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  int version = s.value("version", 0).toInt();
  if (version < kVersion) {
    s.setValue("version", kVersion);
    AddStream(QT_TR_NOOP("Hypnotoad"), QUrl(kHypnotoadUrl));
    AddStream(QT_TR_NOOP("Rain"), QUrl(kRainUrl));
    SaveStreams();
    return;
  }

  int size = s.beginReadArray("streams");
  for (int i = 0; i < size; ++i) {
    s.setArrayIndex(i);
    AddStream(s.value("name").toString(),
              s.value("url").toUrl(),
              s.value("volume").toInt(),
              s.value("enabled").toBool());
  }
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
    s.setValue("enabled", stream->id != -1);
  }
  s.endArray();
}

void BackgroundStreams::AddStream(const QString& name,
                                  const QUrl& url,
                                  int volume,
                                  bool enabled) {
  Stream* s = new Stream;
  s->name = name;
  s->url = url;
  s->volume = volume;
  s->id = -1;
  streams_[name] = s;
  if (enabled) {
    PlayStream(s);
  }
}

void BackgroundStreams::EnableStream(const QString& name, bool enable) {
  Stream* stream = streams_[name];
  if (enable == (stream->id != -1)) {
    return;
  }
  if (enable) {
    PlayStream(stream);
  } else {
    StopStream(stream);
  }
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
}

void BackgroundStreams::StopStream(Stream* stream) {
  engine_->StopBackgroundStream(stream->id);
  stream->id = -1;
  emit StreamStopped(stream->name);
}

int BackgroundStreams::GetStreamVolume(const QString& name) {
  return streams_[name]->volume;
}

bool BackgroundStreams::IsPlaying(const QString& name) {
  return streams_[name]->id != -1;
}

void BackgroundStreams::MakeItRain(bool enable) {
  if (!streams_.contains("Rain")) {
    AddStream("Rain", QUrl(kRainUrl));
  }
  EnableStream("Rain", enable);
}

void BackgroundStreams::AllGloryToTheHypnotoad(bool enable) {
  if (!streams_.contains("Hypnotoad")) {
    AddStream("Hypnotoad", QUrl(kHypnotoadUrl));
  }
  EnableStream("Hypnotoad", enable);
}
