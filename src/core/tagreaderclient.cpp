/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2013, Arnaud Bienner <arnaud.bienner@gmail.com>
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

#include "player.h"
#include "tagreaderclient.h"

#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QTcpServer>
#include <QThread>
#include <QUrl>

const char* TagReaderClient::kWorkerExecutableName = "clementine-tagreader";
TagReaderClient* TagReaderClient::sInstance = nullptr;

TagReaderClient::TagReaderClient(QObject* parent)
    : QObject(parent), worker_pool_(new WorkerPool<HandlerType>(this)) {
  sInstance = this;

  QSettings s;
  s.beginGroup(Player::kSettingsGroup);

  int max_workers = QThread::idealThreadCount();
  int num_workers = s.value("max_numprocs_tagclients", max_workers).toInt();

  worker_pool_->SetExecutableName(kWorkerExecutableName);
  worker_pool_->SetWorkerCount(num_workers);
  connect(worker_pool_, SIGNAL(WorkerFailedToStart()),
          SLOT(WorkerFailedToStart()));
}

void TagReaderClient::Start() { worker_pool_->Start(); }

void TagReaderClient::WorkerFailedToStart() {
  qLog(Error) << "The" << kWorkerExecutableName << "executable was not found"
              << "in the current directory or on the PATH.  Clementine will"
              << "not be able to read music file tags without it.";
}

TagReaderReply* TagReaderClient::ReadFile(const QString& filename) {
  pb::tagreader::Message message;
  pb::tagreader::ReadFileRequest* req = message.mutable_read_file_request();

  req->set_filename(DataCommaSizeFromQString(filename));

  return worker_pool_->SendMessageWithReply(&message);
}

TagReaderReply* TagReaderClient::SaveFile(const QString& filename,
                                          const Song& metadata) {
  pb::tagreader::Message message;
  pb::tagreader::SaveFileRequest* req = message.mutable_save_file_request();

  req->set_filename(DataCommaSizeFromQString(filename));
  metadata.ToProtobuf(req->mutable_metadata());

  return worker_pool_->SendMessageWithReply(&message);
}

TagReaderReply* TagReaderClient::UpdateSongStatistics(const Song& metadata) {
  pb::tagreader::Message message;
  pb::tagreader::SaveSongStatisticsToFileRequest* req =
      message.mutable_save_song_statistics_to_file_request();

  req->set_filename(DataCommaSizeFromQString(metadata.url().toLocalFile()));
  metadata.ToProtobuf(req->mutable_metadata());

  return worker_pool_->SendMessageWithReply(&message);
}

void TagReaderClient::UpdateSongsStatistics(const SongList& songs) {
  for (const Song& song : songs) {
    TagReaderReply* reply = UpdateSongStatistics(song);
    connect(reply, SIGNAL(Finished(bool)), reply, SLOT(deleteLater()));
  }
}

TagReaderReply* TagReaderClient::UpdateSongRating(const Song& metadata) {
  pb::tagreader::Message message;
  pb::tagreader::SaveSongRatingToFileRequest* req =
      message.mutable_save_song_rating_to_file_request();

  req->set_filename(DataCommaSizeFromQString(metadata.url().toLocalFile()));
  metadata.ToProtobuf(req->mutable_metadata());

  return worker_pool_->SendMessageWithReply(&message);
}

void TagReaderClient::UpdateSongsRating(const SongList& songs) {
  for (const Song& song : songs) {
    TagReaderReply* reply = UpdateSongRating(song);
    connect(reply, SIGNAL(Finished(bool)), reply, SLOT(deleteLater()));
  }
}

TagReaderReply* TagReaderClient::IsMediaFile(const QString& filename) {
  pb::tagreader::Message message;
  pb::tagreader::IsMediaFileRequest* req =
      message.mutable_is_media_file_request();

  req->set_filename(DataCommaSizeFromQString(filename));

  return worker_pool_->SendMessageWithReply(&message);
}

TagReaderReply* TagReaderClient::LoadEmbeddedArt(const QString& filename) {
  pb::tagreader::Message message;
  pb::tagreader::LoadEmbeddedArtRequest* req =
      message.mutable_load_embedded_art_request();

  req->set_filename(DataCommaSizeFromQString(filename));

  return worker_pool_->SendMessageWithReply(&message);
}

TagReaderReply* TagReaderClient::ReadCloudFile(
    const QUrl& download_url, const QString& title, int size,
    const QString& mime_type, const QString& authorisation_header) {
  pb::tagreader::Message message;
  pb::tagreader::ReadCloudFileRequest* req =
      message.mutable_read_cloud_file_request();

  const QString url_string = download_url.toEncoded();
  req->set_download_url(DataCommaSizeFromQString(url_string));
  req->set_title(DataCommaSizeFromQString(title));
  req->set_size(size);
  req->set_mime_type(DataCommaSizeFromQString(mime_type));
  req->set_authorisation_header(DataCommaSizeFromQString(authorisation_header));

  return worker_pool_->SendMessageWithReply(&message);
}

void TagReaderClient::ReadFileBlocking(const QString& filename, Song* song) {
  Q_ASSERT(QThread::currentThread() != thread());

  TagReaderReply* reply = ReadFile(filename);
  if (reply->WaitForFinished()) {
    song->InitFromProtobuf(reply->message().read_file_response().metadata());
  }
  reply->deleteLater();
}

bool TagReaderClient::SaveFileBlocking(const QString& filename,
                                       const Song& metadata) {
  Q_ASSERT(QThread::currentThread() != thread());

  bool ret = false;

  TagReaderReply* reply = SaveFile(filename, metadata);
  if (reply->WaitForFinished()) {
    ret = reply->message().save_file_response().success();
  }
  reply->deleteLater();

  return ret;
}

bool TagReaderClient::UpdateSongStatisticsBlocking(const Song& metadata) {
  Q_ASSERT(QThread::currentThread() != thread());

  bool ret = false;

  TagReaderReply* reply = UpdateSongStatistics(metadata);
  if (reply->WaitForFinished()) {
    ret = reply->message().save_song_statistics_to_file_response().success();
  }
  reply->deleteLater();

  return ret;
}

bool TagReaderClient::UpdateSongRatingBlocking(const Song& metadata) {
  Q_ASSERT(QThread::currentThread() != thread());

  bool ret = false;

  TagReaderReply* reply = UpdateSongRating(metadata);
  if (reply->WaitForFinished()) {
    ret = reply->message().save_song_rating_to_file_response().success();
  }
  reply->deleteLater();

  return ret;
}

bool TagReaderClient::IsMediaFileBlocking(const QString& filename) {
  Q_ASSERT(QThread::currentThread() != thread());

  bool ret = false;

  TagReaderReply* reply = IsMediaFile(filename);
  if (reply->WaitForFinished()) {
    ret = reply->message().is_media_file_response().success();
  }
  reply->deleteLater();

  return ret;
}

QImage TagReaderClient::LoadEmbeddedArtBlocking(const QString& filename) {
  Q_ASSERT(QThread::currentThread() != thread());

  QImage ret;

  TagReaderReply* reply = LoadEmbeddedArt(filename);
  if (reply->WaitForFinished()) {
    const std::string& data_str =
        reply->message().load_embedded_art_response().data();
    ret.loadFromData(QByteArray(data_str.data(), data_str.size()));
  }
  reply->deleteLater();

  return ret;
}
