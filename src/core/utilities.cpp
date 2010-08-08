/* This file is part of Clementine.

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

#include "utilities.h"

#include <QCoreApplication>
#include <QDir>
#include <QStringList>
#include <QTemporaryFile>

#if defined(Q_OS_UNIX)
#  include <sys/statvfs.h>
#elif defined(Q_OS_WIN32)
#  include <windows.h>
#endif

namespace Utilities {

static QString tr(const char* str) {
  return QCoreApplication::translate("", str);
}

QString PrettyTime(int seconds) {
  // last.fm sometimes gets the track length wrong, so you end up with
  // negative times.
  seconds = qAbs(seconds);

  int hours = seconds / (60*60);
  int minutes = (seconds / 60) % 60;
  seconds %= 60;

  QString ret;
  if (hours)
    ret.sprintf("%d:%02d:%02d", hours, minutes, seconds);
  else
    ret.sprintf("%d:%02d", minutes, seconds);

  return ret;
}

QString WordyTime(quint64 seconds) {
  quint64 days = seconds / (60*60*24);

  // TODO: Make the plural rules translatable
  QStringList parts;

  if (days)
    parts << (days == 1 ? tr("1 day") : tr("%1 days").arg(days));
  parts << PrettyTime(seconds - days*60*60*24);

  return parts.join(" ");
}

QString PrettySize(quint64 bytes) {
  QString ret;

  if (bytes > 0) {
    if (bytes <= 1000)
      ret = QString::number(bytes) + " bytes";
    else if (bytes <= 1000*1000)
      ret.sprintf("%.1f KB", float(bytes) / 1000);
    else if (bytes <= 1000*1000*1000)
      ret.sprintf("%.1f MB", float(bytes) / (1000*1000));
    else
      ret.sprintf("%.1f GB", float(bytes) / (1000*1000*1000));
  }
  return ret;
}

quint64 FileSystemCapacity(const QString& path) {
#if defined(Q_OS_UNIX)
  struct statvfs fs_info;
  if (statvfs(path.toLocal8Bit().constData(), &fs_info) == 0)
    return quint64(fs_info.f_blocks) * quint64(fs_info.f_bsize);
#elif defined(Q_OS_WIN32)
  _ULARGE_INTEGER ret;
  if (GetDiskFreeSpaceEx(QDir::toNativeSeparators(path).toLocal8Bit().constData(),
                         NULL, &ret, NULL) != 0)
    return ret.QuadPart;
#endif

  return 0;
}

quint64 FileSystemFreeSpace(const QString& path) {
#if defined(Q_OS_UNIX)
  struct statvfs fs_info;
  if (statvfs(path.toLocal8Bit().constData(), &fs_info) == 0)
    return quint64(fs_info.f_bavail) * quint64(fs_info.f_bsize);
#elif defined(Q_OS_WIN32)
  _ULARGE_INTEGER ret;
  if (GetDiskFreeSpaceEx(QDir::toNativeSeparators(path).toLocal8Bit().constData(),
                         &ret, NULL, NULL) != 0)
    return ret.QuadPart;
#endif

  return 0;
}

QString MakeTempDir() {
  QString path;
  {
    QTemporaryFile tempfile;
    tempfile.open();
    path = tempfile.fileName();
  }

  QDir d;
  d.mkdir(path);

  return path;
}

void RemoveRecursive(const QString& path) {
  QDir dir(path);
  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs))
    RemoveRecursive(path + "/" + child);

  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Files))
    QFile::remove(path + "/" + child);

  dir.rmdir(path);
}

void CopyStr(const QString& str, char** dest_p) {
  Q_ASSERT(*dest_p == NULL);
  const QByteArray data = str.toUtf8();
  const int size = data.size() + 1;

  char* dest = new char[size];
  std::copy(data.constData(), data.constData() + size, dest);
  *dest_p = dest;
}

} // namespace
