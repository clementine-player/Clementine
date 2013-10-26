/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QColor>
#include <QLocale>
#include <QCryptographicHash>
#include <QSize>
#include <QString>
#include <QUrl>

#include <boost/scoped_array.hpp>

class QIODevice;
class QMouseEvent;
class QXmlStreamReader;
struct QMetaObject;

namespace Utilities {
  QString PrettyTime(int seconds);
  QString PrettyTimeDelta(int seconds);
  QString PrettyTimeNanosec(qint64 nanoseconds);
  QString PrettySize(quint64 bytes);
  QString PrettySize(const QSize& size);
  QString WordyTime(quint64 seconds);
  QString WordyTimeNanosec(qint64 nanoseconds);
  QString Ago(int seconds_since_epoch, const QLocale& locale);
  QString PrettyFutureDate(const QDate& date);

  QString ColorToRgba(const QColor& color);

  quint64 FileSystemCapacity(const QString& path);
  quint64 FileSystemFreeSpace(const QString& path);

  QString MakeTempDir(const QString template_name = QString());
  QString GetTemporaryFileName();
  void RemoveRecursive(const QString& path);
  bool CopyRecursive(const QString& source, const QString& destination);
  bool Copy(QIODevice* source, QIODevice* destination);

  void OpenInFileBrowser(const QList<QUrl>& filenames);

  enum HashFunction {
    Md5_Algo,
    Sha256_Algo,
    Sha1_Algo,
  };
  QByteArray Hmac(const QByteArray& key, const QByteArray& data, HashFunction algo);
  QByteArray HmacMd5(const QByteArray& key, const QByteArray& data);
  QByteArray HmacSha256(const QByteArray& key, const QByteArray& data);
  QByteArray HmacSha1(const QByteArray& key, const QByteArray& data);
  QByteArray Sha256(const QByteArray& data);


  // Picks an unused ephemeral port number.  Doesn't hold the port open so
  // there's the obvious race condition
  quint16 PickUnusedPort();


  // Forwards a mouse event to a different widget, remapping the event's widget
  // coordinates relative to those of the target widget.
  void ForwardMouseEvent(const QMouseEvent* e, QWidget* target);

  // Checks if the mouse event was inside the widget's rectangle.
  bool IsMouseEventInWidget(const QMouseEvent* e, const QWidget* widget);

  // Reads all children of the current element, and returns with the stream
  // reader either on the EndElement for the current element, or the end of the
  // file - whichever came first.
  void ConsumeCurrentElement(QXmlStreamReader* reader);

  // Advances the stream reader until it finds an element with the given name.
  // Returns false if the end of the document was reached before finding a
  // matching element.
  bool ParseUntilElement(QXmlStreamReader* reader, const QString& name);

  // Parses a string containing an RFC822 time and date.
  QDateTime ParseRFC822DateTime(const QString& text);

  // Replaces some HTML entities with their normal characters.
  QString DecodeHtmlEntities(const QString& text);

  // Shortcut for getting a Qt-aware enum value as a string.
  // Pass in the QMetaObject of the class that owns the enum, the string name of
  // the enum and a valid value from that enum.
  const char* EnumToString(const QMetaObject& meta, const char* name, int value);

  QStringList Prepend(const QString& text, const QStringList& list);
  QStringList Updateify(const QStringList& list);

  // Check if two urls are on the same drive (mainly for windows)
  bool UrlOnSameDriveAsClementine(const QUrl& url);

  // Get relative path to clementine binary
  QUrl GetRelativePathToClementineBin(const QUrl& url);


  enum ConfigPath {
    Path_Root,
    Path_AlbumCovers,
    Path_NetworkCache,
    Path_GstreamerRegistry,
    Path_DefaultMusicLibrary,
    Path_LocalSpotifyBlob,
    Path_MoodbarCache,
    Path_CacheRoot,
  };
  QString GetConfigPath(ConfigPath config);

  // Returns the minor version of OS X (ie. 6 for Snow Leopard, 7 for Lion).
  qint32 GetMacVersion();

  // Borrowed from schedutils
  enum IoPriority {
    IOPRIO_CLASS_NONE = 0,
    IOPRIO_CLASS_RT,
    IOPRIO_CLASS_BE,
    IOPRIO_CLASS_IDLE,
  };
  enum {
    IOPRIO_WHO_PROCESS = 1,
    IOPRIO_WHO_PGRP,
    IOPRIO_WHO_USER,
  };
  static const int IOPRIO_CLASS_SHIFT = 13;

  int SetThreadIOPriority(IoPriority priority);
  int GetThreadId();

  // Returns true if this machine has a battery.
  bool IsLaptop();

  QString SystemLanguageName();
}

class ScopedWCharArray {
public:
  ScopedWCharArray(const QString& str);

  QString ToString() const { return QString::fromWCharArray(data_.get()); }

  wchar_t* get() const { return data_.get(); }
  operator wchar_t*() const { return get(); }

  int characters() const { return chars_; }
  int bytes() const { return (chars_ + 1) * sizeof(wchar_t); }

private:
  Q_DISABLE_COPY(ScopedWCharArray);

  int chars_;
  boost::scoped_array<wchar_t> data_;
};

#endif // UTILITIES_H
