/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "moodbarloader.h"

#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QNetworkDiskCache>
#include <QTimer>
#include <QThread>
#include <QUrl>

#include "moodbarpipeline.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/qhash_qurl.h"
#include "core/utilities.h"

MoodbarLoader::MoodbarLoader(Application* app, QObject* parent)
    : QObject(parent),
      cache_(new QNetworkDiskCache(this)),
      thread_(new QThread(this)),
      kMaxActiveRequests(qMax(1, QThread::idealThreadCount() / 2)),
      save_alongside_originals_(false),
      disable_moodbar_calculation_(false) {
  cache_->setCacheDirectory(
      Utilities::GetConfigPath(Utilities::Path_MoodbarCache));
  cache_->setMaximumCacheSize(60 * 1024 *
                              1024);  // 60MB - enough for 20,000 moodbars

  connect(app, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));
  ReloadSettings();
}

MoodbarLoader::~MoodbarLoader() {
  thread_->quit();
  thread_->wait(1000);
}

void MoodbarLoader::ReloadSettings() {
  QSettings s;
  s.beginGroup("Moodbar");
  save_alongside_originals_ =
      s.value("save_alongside_originals", false).toBool();

  disable_moodbar_calculation_ = !s.value("calculate", true).toBool();
  MaybeTakeNextRequest();
}

QStringList MoodbarLoader::MoodFilenames(const QString& song_filename) {
  const QFileInfo file_info(song_filename);
  const QString dir_path(file_info.dir().path());

  QStringList parts(file_info.fileName().split('.'));
  parts.removeLast();
  parts.append("mood");
  const QString mood_filename(parts.join("."));

  return QStringList() << dir_path + "/." + mood_filename
                       << dir_path + "/" + mood_filename;
}

MoodbarLoader::Result MoodbarLoader::Load(const QUrl& url, QByteArray* data,
                                          MoodbarPipeline** async_pipeline) {
  if (url.scheme() != "file") {
    return CannotLoad;
  }

  // Are we in the middle of loading this moodbar already?
  if (requests_.contains(url)) {
    *async_pipeline = requests_[url];
    return WillLoadAsync;
  }

  // Check if a mood file exists for this file already
  const QString filename(url.toLocalFile());

  for (const QString& possible_mood_file : MoodFilenames(filename)) {
    QFile f(possible_mood_file);
    if (f.open(QIODevice::ReadOnly)) {
      qLog(Info) << "Loading moodbar data from" << possible_mood_file;
      *data = f.readAll();
      return Loaded;
    }
  }

  // Maybe it exists in the cache?
  std::unique_ptr<QIODevice> cache_device(cache_->data(url));
  if (cache_device) {
    qLog(Info) << "Loading cached moodbar data for" << filename;
    *data = cache_device->readAll();
    if (!data->isEmpty()) {
      return Loaded;
    }
  }

  if (!thread_->isRunning()) thread_->start(QThread::IdlePriority);

  // There was no existing file, analyze the audio file and create one.
  MoodbarPipeline* pipeline = new MoodbarPipeline(url);
  pipeline->moveToThread(thread_);
  NewClosure(pipeline, SIGNAL(Finished(bool)), this,
             SLOT(RequestFinished(MoodbarPipeline*, QUrl)), pipeline, url);

  requests_[url] = pipeline;
  queued_requests_ << url;

  MaybeTakeNextRequest();

  *async_pipeline = pipeline;
  return WillLoadAsync;
}

void MoodbarLoader::MaybeTakeNextRequest() {
  Q_ASSERT(QThread::currentThread() == qApp->thread());

  if (active_requests_.count() >= kMaxActiveRequests ||
      queued_requests_.isEmpty() || disable_moodbar_calculation_) {
    return;
  }

  const QUrl url = queued_requests_.takeFirst();
  active_requests_ << url;

  qLog(Info) << "Creating moodbar data for" << url.toLocalFile();
  QMetaObject::invokeMethod(requests_[url], "Start", Qt::QueuedConnection);
}

void MoodbarLoader::RequestFinished(MoodbarPipeline* request, const QUrl& url) {
  Q_ASSERT(QThread::currentThread() == qApp->thread());

  if (request->success()) {
    qLog(Info) << "Moodbar data generated successfully for"
               << url.toLocalFile();

    // Save the data in the cache
    QNetworkCacheMetaData metadata;
    metadata.setUrl(url);

    QIODevice* cache_file = cache_->prepare(metadata);
    if (cache_file) {
      cache_file->write(request->data());
      cache_->insert(cache_file);
    }

    // Save the data alongside the original as well if we're configured to.
    if (save_alongside_originals_) {
      const QString mood_filename(MoodFilenames(url.toLocalFile())[0]);
      QFile mood_file(mood_filename);
      if (mood_file.open(QIODevice::WriteOnly)) {
        mood_file.write(request->data());

#ifdef Q_OS_WIN32
        if (!SetFileAttributes((LPCTSTR)mood_filename.utf16(),
                               FILE_ATTRIBUTE_HIDDEN)) {
          qLog(Warning) << "Error changing hidden attribute for file"
                        << mood_filename;
        }
#endif

      } else {
        qLog(Warning) << "Error opening mood file for writing" << mood_filename;
      }
    }
  }

  // Remove the request from the active list and delete it
  requests_.remove(url);
  active_requests_.remove(url);

  QTimer::singleShot(1000, request, SLOT(deleteLater()));

  MaybeTakeNextRequest();
}
