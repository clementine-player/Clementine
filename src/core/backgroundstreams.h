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

#ifndef CORE_BACKGROUNDSTREAMS_H_
#define CORE_BACKGROUNDSTREAMS_H_

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QUrl>

#include "engines/engine_fwd.h"

class QAction;

class BackgroundStreams : public QObject {
  Q_OBJECT

 public:
  BackgroundStreams(EngineBase* engine, QObject* parent = nullptr);
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
    Stream() : volume(0), id(0), action(nullptr) {}

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

#endif  // CORE_BACKGROUNDSTREAMS_H_
