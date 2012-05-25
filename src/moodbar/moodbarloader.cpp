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
#include "moodbarpipeline.h"
#include "core/closure.h"
#include "core/utilities.h"

#include <QDir>
#include <QFileInfo>
#include <QNetworkDiskCache>
#include <QUrl>

MoodbarLoader::MoodbarLoader(QObject* parent)
  : QObject(parent),
    cache_(new QNetworkDiskCache(this))
{
  cache_->setCacheDirectory(Utilities::GetConfigPath(Utilities::Path_MoodbarCache));
  cache_->setMaximumCacheSize(1024 * 1024); // 1MB - enough for 333 moodbars
}

MoodbarLoader::~MoodbarLoader() {
}

MoodbarLoader::Result MoodbarLoader::Load(
        const QUrl& url, QByteArray* data, MoodbarPipeline** async_pipeline) {
  if (url.scheme() != "file") {
    return CannotLoad;
  }
  
  // Are we in the middle of loading this moodbar already?
  if (active_requests_.contains(url)) {
    *async_pipeline = active_requests_[url];
    return WillLoadAsync;
  }
  
  // Check if a mood file exists for this file already
  const QString filename(url.toLocalFile());
  const QFileInfo file_info(filename);
  const QString dir_path(file_info.dir().path());
  
  QStringList parts(file_info.fileName().split('.'));
  parts.removeLast();
  parts.append("mood");
  const QString mood_filename(parts.join("."));
  
  QStringList possible_mood_files;
  possible_mood_files << dir_path + "/." + mood_filename
                      << dir_path + "/" + mood_filename;
  
  foreach (const QString& possible_mood_file, possible_mood_files) {
    QFile f(possible_mood_file);
    if (f.open(QIODevice::ReadOnly)) {
      qLog(Info) << "Loading moodbar data from" << possible_mood_file;
      *data = f.readAll();
      return Loaded;
    }
  }
  
  // Maybe it exists in the cache?
  QIODevice* cache_device = cache_->data(url);
  if (cache_device) {
    qLog(Info) << "Loading cached moodbar data for" << filename;
    *data = cache_device->readAll();
    delete cache_device;
    return Loaded;
  }
  
  // There was no existing file, analyze the audio file and create one.
  MoodbarPipeline* pipeline = new MoodbarPipeline(filename);
  if (!pipeline->Start()) {
    delete pipeline;
    return CannotLoad;
  }
  
  qLog(Info) << "Creating moodbar data for" << filename;
  
  active_requests_[filename] = pipeline;
  NewClosure(pipeline, SIGNAL(Finished(bool)),
             this, SLOT(RequestFinished(MoodbarPipeline*,QUrl)),
             pipeline, url);
  
  *async_pipeline = pipeline;
  return WillLoadAsync;
}

void MoodbarLoader::RequestFinished(MoodbarPipeline* request, const QUrl& url) {
  if (request->success()) {
    qLog(Info) << "Moodbar data generated successfully for" << url.toLocalFile();
    
    // Save the data in the cache
    QNetworkCacheMetaData metadata;
    metadata.setUrl(url);
    
    QIODevice* cache_file = cache_->prepare(metadata);
    cache_file->write(request->data());
    cache_->insert(cache_file);
  }
  
  // Remove the request from the active list and delete it
  active_requests_.take(url);
  request->deleteLater();
}
