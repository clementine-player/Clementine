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

#include "tagreaderclient.h"

#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QTcpServer>


const char* TagReaderClient::kWorkerExecutableName = "clementine-tagreader";

TagReaderClient::TagReaderClient(QObject* parent)
  : QObject(parent),
    worker_pool_(new WorkerPool<HandlerType>(this))
{
}

void TagReaderClient::Start() {
  worker_pool_->Start();
}

TagReaderReply* TagReaderClient::ReadFile(const QString& filename) {
  pb::tagreader::Message message;
  pb::tagreader::ReadFileRequest* req = message.mutable_read_file_request();

  req->set_filename(DataCommaSizeFromQString(filename));

  return handler_->SendMessageWithReply(&message);
}

TagReaderReply* TagReaderClient::SaveFile(const QString& filename, const Song& metadata) {
  pb::tagreader::Message message;
  pb::tagreader::SaveFileRequest* req = message.mutable_save_file_request();

  req->set_filename(DataCommaSizeFromQString(filename));
  metadata.ToProtobuf(req->mutable_metadata());

  return handler_->SendMessageWithReply(&message);
}

TagReaderReply* TagReaderClient::IsMediaFile(const QString& filename) {
  pb::tagreader::Message message;
  pb::tagreader::IsMediaFileRequest* req = message.mutable_is_media_file_request();

  req->set_filename(DataCommaSizeFromQString(filename));

  return handler_->SendMessageWithReply(&message);
}

TagReaderReply* TagReaderClient::LoadEmbeddedArt(const QString& filename) {
  pb::tagreader::Message message;
  pb::tagreader::LoadEmbeddedArtRequest* req = message.mutable_load_embedded_art_request();

  req->set_filename(DataCommaSizeFromQString(filename));

  return handler_->SendMessageWithReply(&message);
}
