#ifndef BACKGROUNDSTREAMS_H
#define BACKGROUNDSTREAMS_H

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QUrl>

#include "engines/engine_fwd.h"

class BackgroundStreams : public QObject {
  Q_OBJECT
 public:
  explicit BackgroundStreams(EngineBase* engine, QObject* parent = 0);
  ~BackgroundStreams();

  void LoadStreams();
  void SaveStreams();

  QStringList streams() const { return streams_.keys(); }

  void EnableStream(const QString& name, bool enable);
  void SetStreamVolume(const QString& name, int volume);

  int GetStreamVolume(const QString& name);
  bool IsPlaying(const QString& name);

 public slots:
  void MakeItRain(bool enable);
  void AllGloryToTheHypnotoad(bool enable);

 signals:
  void StreamStarted(const QString& name);
  void StreamStopped(const QString& name);

 private:
  struct Stream {
    QString name;
    QUrl url;
    int volume;
    int id;
  };

  void AddStream(
      const QString& name, const QUrl& url, int volume = 50, bool enabled = false);
  void PlayStream(Stream* stream);
  void StopStream(Stream* stream);

  EngineBase* engine_;

  QMap<QString, Stream*> streams_;

  static const char* kSettingsGroup;
  static const int kVersion = 1;

  static const char* kHypnotoadUrl;
  static const char* kRainUrl;
};

#endif
