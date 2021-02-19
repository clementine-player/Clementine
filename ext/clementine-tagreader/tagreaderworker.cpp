/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "tagreaderworker.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QTextCodec>
#include <QUrl>

TagReaderWorker::TagReaderWorker(QIODevice* socket, QObject* parent)
    : AbstractMessageHandler<pb::tagreader::Message>(socket, parent) {}

void TagReaderWorker::MessageArrived(const pb::tagreader::Message& message) {
  pb::tagreader::Message reply;

#if 0
  // Crash every few requests
  if (qrand() % 10 == 0) {
    qLog(Debug) << "Crashing on request ID" << message.id();
    abort();
  }
#endif

  if (message.has_read_file_request()) {
    tag_reader_.ReadFile(
        QStringFromStdString(message.read_file_request().filename()),
        reply.mutable_read_file_response()->mutable_metadata());
  } else if (message.has_save_file_request()) {
    reply.mutable_save_file_response()->set_success(tag_reader_.SaveFile(
        QStringFromStdString(message.save_file_request().filename()),
        message.save_file_request().metadata()));
  } else if (message.has_save_song_tag_to_file_request()) {
    reply.mutable_save_song_tag_to_file_response()->set_success(
        tag_reader_.UpdateSongTag(
            QStringFromStdString(
                message.save_song_tag_to_file_request().filename()),
            QStringFromStdString(
                message.save_song_tag_to_file_request().tagname()),
            QStringFromStdString(
                message.save_song_tag_to_file_request().tagvalue())));
  } else if (message.has_save_song_statistics_to_file_request()) {
    reply.mutable_save_song_statistics_to_file_response()->set_success(
        tag_reader_.SaveSongStatisticsToFile(
            QStringFromStdString(
                message.save_song_statistics_to_file_request().filename()),
            message.save_song_statistics_to_file_request().metadata()));
  } else if (message.has_save_song_rating_to_file_request()) {
    reply.mutable_save_song_rating_to_file_response()->set_success(
        tag_reader_.SaveSongRatingToFile(
            QStringFromStdString(
                message.save_song_rating_to_file_request().filename()),
            message.save_song_rating_to_file_request().metadata()));
  } else if (message.has_is_media_file_request()) {
    reply.mutable_is_media_file_response()->set_success(tag_reader_.IsMediaFile(
        QStringFromStdString(message.is_media_file_request().filename())));
  } else if (message.has_load_embedded_art_request()) {
    QByteArray data = tag_reader_.LoadEmbeddedArt(
        QStringFromStdString(message.load_embedded_art_request().filename()));
    reply.mutable_load_embedded_art_response()->set_data(data.constData(),
                                                         data.size());
  } else if (message.has_read_cloud_file_request()) {
#ifdef HAVE_GOOGLE_DRIVE
    const pb::tagreader::ReadCloudFileRequest& req =
        message.read_cloud_file_request();
    if (!tag_reader_.ReadCloudFile(
            QUrl::fromEncoded(QByteArray(req.download_url().data(),
                                         req.download_url().size())),
            QStringFromStdString(req.title()), req.size(),
            QStringFromStdString(req.mime_type()),
            QStringFromStdString(req.authorisation_header()),
            reply.mutable_read_cloud_file_response()->mutable_metadata())) {
      reply.mutable_read_cloud_file_response()->clear_metadata();
    }
#endif
  }

  SendReply(message, &reply);
}

void TagReaderWorker::DeviceClosed() {
  AbstractMessageHandler<pb::tagreader::Message>::DeviceClosed();

  qApp->exit();
}
