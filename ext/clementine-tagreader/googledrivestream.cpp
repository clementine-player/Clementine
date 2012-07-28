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

#include "googledrivestream.h"
#include "core/logging.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <taglib/id3v2framefactory.h>
#include <taglib/mpegfile.h>
using TagLib::ByteVector;

GoogleDriveStream::GoogleDriveStream(
    const QUrl& url, const QString& filename, const long length,
    const QString& auth, QNetworkAccessManager* network)
    : url_(url),
      filename_(filename),
      encoded_filename_(filename_.toUtf8()),
      length_(length),
      auth_(auth),
      cursor_(0),
      network_(network),
      cache_(length) {
}

TagLib::FileName GoogleDriveStream::name() const {
  return encoded_filename_.data();
}

bool GoogleDriveStream::CheckCache(int start, int end) {
  for (int i = start; i <= end; ++i) {
    if (!cache_.test(i)) {
      return false;
    }
  }
  return true;
}

void GoogleDriveStream::FillCache(int start, TagLib::ByteVector data) {
  for (int i = 0; i < data.size(); ++i) {
    cache_.set(start + i, data[i]);
  }
}

TagLib::ByteVector GoogleDriveStream::GetCached(int start, int end) {
  const uint size = end - start + 1;
  TagLib::ByteVector ret(size);
  for (int i = 0; i < size; ++i) {
    ret[i] = cache_.get(start + i);
  }
  return ret;
}

TagLib::ByteVector GoogleDriveStream::readBlock(ulong length) {
  const uint start = cursor_;
  const uint end = qMin(cursor_ + length - 1, length_ - 1);

  if (end <= start) {
    return TagLib::ByteVector();
  }

  if (CheckCache(start, end)) {
    TagLib::ByteVector cached = GetCached(start, end);
    cursor_ += cached.size();
    return cached;
  }

  QNetworkRequest request = QNetworkRequest(url_);
  request.setRawHeader(
      "Authorization", QString("Bearer %1").arg(auth_).toUtf8());
  request.setRawHeader(
      "Range", QString("bytes=%1-%2").arg(start).arg(end).toUtf8());

  QNetworkReply* reply = network_->get(request);

  QEventLoop loop;
  QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();
  reply->deleteLater();

  QByteArray data = reply->readAll();
  TagLib::ByteVector bytes(data.data(), data.size());
  cursor_ += data.size();

  FillCache(start, bytes);
  return bytes;
}

void GoogleDriveStream::writeBlock(const ByteVector&) {
  qLog(Debug) << Q_FUNC_INFO << "not implemented";
}

void GoogleDriveStream::insert(const ByteVector&, ulong, ulong) {
  qLog(Debug) << Q_FUNC_INFO << "not implemented";
}

void GoogleDriveStream::removeBlock(ulong, ulong) {
  qLog(Debug) << Q_FUNC_INFO << "not implemented";
}

bool GoogleDriveStream::readOnly() const {
  qLog(Debug) << Q_FUNC_INFO;
  return true;
}

bool GoogleDriveStream::isOpen() const {
  return true;
}

void GoogleDriveStream::seek(long offset, TagLib::IOStream::Position p) {
  switch (p) {
    case TagLib::IOStream::Beginning:
      cursor_ = offset;
      break;

    case TagLib::IOStream::Current:
      cursor_ = qMin(ulong(cursor_ + offset), length_);
      break;

    case TagLib::IOStream::End:
      cursor_ = qMax(0UL, length_ - offset);
      break;
  }
}

void GoogleDriveStream::clear() {
  cursor_ = 0;
}

long GoogleDriveStream::tell() const {
  return cursor_;
}

long GoogleDriveStream::length() {
  return length_;
}

void GoogleDriveStream::truncate(long) {
  qLog(Debug) << Q_FUNC_INFO << "not implemented";
}
