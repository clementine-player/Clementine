#ifndef BACKGROUNDSTREAMS_H
#define BACKGROUNDSTREAMS_H

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QUrl>

#include "engines/engine_fwd.h"

class QAction;

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

  int GetStreamVolume(const QString& name) const;
  bool IsPlaying(const QString& name) const;

  void AddAction(const QString& name, QAction* action);

signals:
  void StreamStarted(const QString& name);
  void StreamStopped(const QString& name);

 private slots:
  void StreamActionToggled(bool checked);
  void StreamActionDestroyed();

 private:
  struct Stream {
    Stream() : volume(0), id(0), action(NULL) {}

    QString name;
    QUrl url;
    int volume;
    int id;

    QAction* action;
  };

  void AddStream(const QString& name, const QUrl& url, int volume = 50);
  void PlayStream(Stream* stream);
  void StopStream(Stream* stream);

  EngineBase* engine_;

  QMap<QString, Stream*> streams_;

  static const char* kSettingsGroup;
  static const int kVersion = 2;

  static const char* kHypnotoadUrl;
  static const char* kRainUrl;
  static const char* kEnterpriseUrl;
};

#endif
