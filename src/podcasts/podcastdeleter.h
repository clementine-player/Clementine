/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef PODCASTS_PODCASTDELETER_H_
#define PODCASTS_PODCASTDELETER_H_

#include "core/network.h"
#include "podcast.h"
#include "podcastepisode.h"

#include <QFile>
#include <QList>
#include <QObject>
#include <QQueue>
#include <QRegExp>
#include <QSet>

#ifdef Q_OS_WIN
#include <time.h>
#else
#include <sys/time.h>
#endif

class Application;
class PodcastBackend;

class QNetworkAccessManager;

class PodcastDeleter : public QObject {
  Q_OBJECT

 public:
  explicit PodcastDeleter(Application* app, QObject* parent = nullptr);
  static const char* kSettingsGroup;
  static const int kAutoDeleteCheckIntervalMsec;

 public slots:
  // Deletes downloaded data for this episode
  void DeleteEpisode(const PodcastEpisode& episode);
  void AutoDelete();
  void ReloadSettings();

 private:
  Application* app_;
  PodcastBackend* backend_;
  int delete_after_secs_;
  QTimer* auto_delete_timer_;
};

#endif  // PODCASTS_PODCASTDELETER_H_
