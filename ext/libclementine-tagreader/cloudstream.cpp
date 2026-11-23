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

#include "cloudstream.h"

#include <taglib/id3v2framefactory.h>
#include <taglib/mpegfile.h>

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <limits>

#include "core/logging.h"

namespace {
static const int kTaglibPrefixCacheBytes = 64 * 1024;  // Should be enough.
static const int kTaglibSuffixCacheBytes = 8 * 1024;
}  // namespace

CloudStream::CloudStream(const QUrl& url, const QString& filename,
                         const long length, const QString& auth)
    : url_(url),
      filename_(filename),
      encoded_filename_(filename_.toUtf8()),
      length_(length),
      auth_(auth),
      cursor_(0),
      network_(new QNetworkAccessManager),
      cache_(length),
      num_requests_(0) {}

TagLib::FileName CloudStream::name() const { return encoded_filename_.data(); }

bool CloudStream::CheckCache(int start, int end) {
  for (int i = start; i <= end; ++i) {
    if (!cache_.test(i)) {
      return false;
    }
  }
  return true;
}

void CloudStream::FillCache(int start, TagLib::ByteVector data) {
  // Clamp to length_ so an out-of-range start or oversized reply never writes
  // past the end of the fixed-size cache_ sparsetable.
  if (start < 0 || static_cast<ulong>(start) >= length_) {
    return;
  }
  const int max_count = static_cast<int>(
      qMin<ulong>(data.size(), length_ - static_cast<ulong>(start)));
  for (int i = 0; i < max_count; ++i) {
    cache_.set(start + i, data[i]);
  }
}

TagLib::ByteVector CloudStream::GetCached(int start, int end) {
  const int size = end - start + 1;
  // Cast: taglib 2 also has a ByteVector(char) overload, so an int is
  // ambiguous.
  TagLib::ByteVector ret(static_cast<unsigned int>(size));
  for (int i = 0; i < size; ++i) {
    ret[i] = cache_.get(start + i);
  }
  return ret;
}

void CloudStream::Precache() {
  // For reading the tags of an MP3, TagLib tends to request:
  // 1. The first 1024 bytes
  // 2. Somewhere between the first 2KB and first 60KB
  // 3. The last KB or two.
  // 4. Somewhere in the first 64KB again
  //
  // OGG Vorbis may read the last 4KB.
  //
  // So, if we precache the first 64KB and the last 8KB we should be sorted :-)
  // Ideally, we would use bytes=0-655364,-8096 but Google Drive does not seem
  // to support multipart byte ranges yet so we have to make do with two
  // requests.

  seek(0, TagLib::IOStream::Beginning);
  readBlock(kTaglibPrefixCacheBytes);
  seek(kTaglibSuffixCacheBytes, TagLib::IOStream::End);
  readBlock(kTaglibSuffixCacheBytes);
  clear();
}

#if (TAGLIB_MAJOR_VERSION == 2)
TagLib::ByteVector CloudStream::readBlock(size_t length) {
#else
TagLib::ByteVector CloudStream::readBlock(ulong length) {
#endif
  // Bail out before computing end = start + clamped_len - 1 so an empty
  // stream, zero-length read, or EOF cursor cannot underflow size_t.
  if (length_ == 0 || length == 0 || cursor_ < 0 ||
      static_cast<ulong>(cursor_) >= length_) {
    return TagLib::ByteVector();
  }
  const size_t start = static_cast<size_t>(cursor_);
  const size_t avail = static_cast<size_t>(length_) - start;
  const size_t clamped_len = qMin(static_cast<size_t>(length), avail);
  const size_t end = start + clamped_len - 1;

  if (end < start) {
    return TagLib::ByteVector();
  }

  if (CheckCache(start, end)) {
    TagLib::ByteVector cached = GetCached(start, end);
    cursor_ += cached.size();
    return cached;
  }

  QNetworkRequest request = QNetworkRequest(url_);
  if (!auth_.isEmpty()) {
    request.setRawHeader("Authorization", auth_.toUtf8());
  }
  request.setRawHeader("Range",
                       QString("bytes=%1-%2").arg(start).arg(end).toUtf8());
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                       QNetworkRequest::AlwaysNetwork);
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       QNetworkRequest::NoLessSafeRedirectPolicy);

  QNetworkReply* reply = network_->get(request);
  connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
          SLOT(SSLErrors(QList<QSslError>)));
  ++num_requests_;

  QEventLoop loop;
  QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();
  reply->deleteLater();

  int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  // Require 206 Partial Content for nonzero offsets: servers that ignore Range
  // return 200 OK starting at byte 0, which would corrupt cursor_ and cache_.
  if (reply->error() != QNetworkReply::NoError || code >= 400 ||
      (start > 0 && code != 206)) {
    qLog(Debug) << "Error retrieving url to tag:" << url_ << "HTTP" << code;
    return TagLib::ByteVector();
  }

  QByteArray data = reply->readAll();
  // Truncate oversized bodies to the requested byte window before advancing
  // cursor_ or populating cache_.
  const qint64 max_bytes = static_cast<qint64>(end - start + 1);
  if (data.size() > max_bytes) {
    data.truncate(static_cast<int>(max_bytes));
  }
  TagLib::ByteVector bytes(data.data(), data.size());
  cursor_ += data.size();

  FillCache(static_cast<int>(start), bytes);
  return bytes;
}

void CloudStream::writeBlock(const TagLib::ByteVector&) {
  qLog(Debug) << Q_FUNC_INFO << "not implemented";
}

#if (TAGLIB_MAJOR_VERSION == 2)
void CloudStream::insert(const TagLib::ByteVector&, TagLib::offset_t, size_t) {
#else
void CloudStream::insert(const TagLib::ByteVector&, ulong, ulong) {
#endif
  qLog(Debug) << Q_FUNC_INFO << "not implemented";
}

#if (TAGLIB_MAJOR_VERSION == 2)
void CloudStream::removeBlock(TagLib::offset_t, size_t) {
#else
void CloudStream::removeBlock(ulong, ulong) {
#endif
  qLog(Debug) << Q_FUNC_INFO << "not implemented";
}

bool CloudStream::readOnly() const {
  qLog(Debug) << Q_FUNC_INFO;
  return true;
}

bool CloudStream::isOpen() const { return true; }

#if (TAGLIB_MAJOR_VERSION == 2)
void CloudStream::seek(TagLib::offset_t offset, TagLib::IOStream::Position p) {
#else
void CloudStream::seek(long offset, TagLib::IOStream::Position p) {
#endif
  qint64 target = 0;
  switch (p) {
    case TagLib::IOStream::Beginning:
      target = static_cast<qint64>(offset);
      break;

    case TagLib::IOStream::Current:
      target = static_cast<qint64>(cursor_) + static_cast<qint64>(offset);
      break;

    case TagLib::IOStream::End:
      // This should really not have qAbs(), but OGG reading needs it.
      target = static_cast<qint64>(length_) - qAbs(static_cast<qint64>(offset));
      break;
  }
  // Compute in signed 64-bit math and clamp to [0, min(length_, INT_MAX)] so
  // seeking from End on files smaller than offset cannot underflow unsigned.
  const qint64 max_cursor = qMin<qint64>(static_cast<qint64>(length_),
                                         std::numeric_limits<int>::max());
  cursor_ = static_cast<int>(qBound<qint64>(0, target, max_cursor));
}

void CloudStream::clear() { cursor_ = 0; }

#if (TAGLIB_MAJOR_VERSION == 2)
TagLib::offset_t CloudStream::tell() const { return cursor_; }

TagLib::offset_t CloudStream::length() { return length_; }

void CloudStream::truncate(TagLib::offset_t) {
#else
long CloudStream::tell() const { return cursor_; }

long CloudStream::length() { return length_; }

void CloudStream::truncate(long) {
#endif
  qLog(Debug) << Q_FUNC_INFO << "not implemented";
}

void CloudStream::SSLErrors(const QList<QSslError>& errors) {
  for (const QSslError& error : errors) {
    qLog(Debug) << error.error() << error.errorString();
    qLog(Debug) << error.certificate();
  }
}
