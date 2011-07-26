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
#include <QSize>
#include <QString>

#include <boost/scoped_array.hpp>

class QIODevice;

namespace Utilities {
  QString PrettyTime(int seconds);
  QString PrettyTimeDelta(int seconds);
  QString PrettyTimeNanosec(qint64 nanoseconds);
  QString PrettySize(quint64 bytes);
  QString PrettySize(const QSize& size);
  QString WordyTime(quint64 seconds);
  QString WordyTimeNanosec(qint64 nanoseconds);
  QString Ago(int seconds_since_epoch, const QLocale& locale);

  QString ColorToRgba(const QColor& color);

  quint64 FileSystemCapacity(const QString& path);
  quint64 FileSystemFreeSpace(const QString& path);

  QString MakeTempDir(const QString template_name = QString());
  void RemoveRecursive(const QString& path);
  bool CopyRecursive(const QString& source, const QString& destination);
  bool Copy(QIODevice* source, QIODevice* destination);

  void OpenInFileBrowser(const QStringList& filenames);

  QByteArray HmacSha256(const QByteArray& key, const QByteArray& data);
  QByteArray Sha256(const QByteArray& data);


  enum ConfigPath {
    Path_Root,
    Path_AlbumCovers,
    Path_NetworkCache,
    Path_GstreamerRegistry,
    Path_DefaultMusicLibrary,
    Path_LocalSpotifyBlob,
  };
  QString GetConfigPath(ConfigPath config);
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
