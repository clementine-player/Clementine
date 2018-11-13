/* This file is part of Clementine.
   Copyright 2010-2014, David Sansome <me@davidsansome.com>
   Copyright 2010-2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, 2014, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2012, Alan Briolat <alan.briolat@gmail.com>
   Copyright 2012, Veniamin Gvozdikov <G.Veniamin@gmail.com>
   Copyright 2013-2014, Andreas <asfa194@gmail.com>
   Copyright 2013, Glad Olus <gladolus@gmx.com>
   Copyright 2013, graehl <graehl@gmail.com>
   Copyright 2014, vkrishtal <krishtalhost@gmail.com>
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

#include "utilities.h"

#include <stdlib.h>

#include <memory>

#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QStringList>
#include <QTcpServer>
#include <QtDebug>
#include <QTemporaryFile>
#include <QtGlobal>
#include <QUrl>
#include <QWidget>
#include <QXmlStreamReader>

#include "core/application.h"
#include "core/logging.h"
#include "config.h"
#include "timeconstants.h"

#include "sha2.h"

#if defined(Q_OS_UNIX)
#include <sys/statvfs.h>
#elif defined(Q_OS_WIN32)
#include <windows.h>
#include <QProcess>
#endif

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <sys/syscall.h>
#endif
#ifdef Q_OS_DARWIN
#include <sys/resource.h>
#endif

#ifdef Q_OS_DARWIN
#include "core/mac_startup.h"
#include "core/mac_utilities.h"
#include "core/scoped_cftyperef.h"
#include "CoreServices/CoreServices.h"
#include "IOKit/ps/IOPowerSources.h"
#include "IOKit/ps/IOPSKeys.h"
#include <QProcess>
#endif

namespace Utilities {

static QString tr(const char* str) {
  return QCoreApplication::translate("", str);
}

QString PrettyTimeDelta(int seconds) {
  return (seconds >= 0 ? "+" : "-") + PrettyTime(seconds);
}

QString PrettyTime(int seconds, bool always_show_hours) {
  // last.fm sometimes gets the track length wrong, so you end up with
  // negative times.
  seconds = qAbs(seconds);

  int hours = seconds / (60 * 60);
  int minutes = (seconds / 60) % 60;
  seconds %= 60;

  QString ret;
  if (hours || always_show_hours)
    ret.sprintf("%d:%02d:%02d", hours, minutes,
                seconds);  // NOLINT(runtime/printf)
  else
    ret.sprintf("%d:%02d", minutes, seconds);  // NOLINT(runtime/printf)

  return ret;
}

QString PrettyTimeNanosec(qint64 nanoseconds, bool always_show_hours) {
  return PrettyTime(nanoseconds / kNsecPerSec, always_show_hours);
}

QString WordyTime(quint64 seconds) {
  quint64 days = seconds / (kSecsPerDay);
  quint64 remaining_hours = (seconds - days * kSecsPerDay) / (60 * 60);

  // TODO(David Sansome): Make the plural rules translatable
  QStringList parts;

  if (days) parts << (days == 1 ? tr("1 day") : tr("%1 days").arg(days));

  // Since PrettyTime does not return the hour if it is 0, we need to add it
  // explicitly for durations longer than 1 day.
  parts << (days && !remaining_hours ? QString("0:") : QString()) +
               PrettyTime(seconds - days * kSecsPerDay);

  return parts.join(" ");
}

QString WordyTimeNanosec(qint64 nanoseconds) {
  return WordyTime(nanoseconds / kNsecPerSec);
}

QString Ago(int seconds_since_epoch, const QLocale& locale) {
  const QDateTime now = QDateTime::currentDateTime();
  const QDateTime then = QDateTime::fromTime_t(seconds_since_epoch);
  const int days_ago = then.date().daysTo(now.date());
  const QString time =
      then.time().toString(locale.timeFormat(QLocale::ShortFormat));

  if (days_ago == 0) return tr("Today") + " " + time;
  if (days_ago == 1) return tr("Yesterday") + " " + time;
  if (days_ago <= 7) return tr("%1 days ago").arg(days_ago);

  return then.date().toString(locale.dateFormat(QLocale::ShortFormat));
}

QString PrettyFutureDate(const QDate& date) {
  const QDate now = QDate::currentDate();
  const int delta_days = now.daysTo(date);

  if (delta_days < 0) return QString();
  if (delta_days == 0) return tr("Today");
  if (delta_days == 1) return tr("Tomorrow");
  if (delta_days <= 7) return tr("In %1 days").arg(delta_days);
  if (delta_days <= 14) return tr("Next week");

  return tr("In %1 weeks").arg(delta_days / 7);
}

QString PrettySize(quint64 bytes) {
  QString ret;

  if (bytes > 0) {
    if (bytes <= 1000)
      ret = QString::number(bytes) + " bytes";
    else if (bytes <= 1000 * 1000)
      ret.sprintf("%.1f KB",
                  static_cast<float>(bytes) / 1000);  // NOLINT(runtime/printf)
    else if (bytes <= 1000 * 1000 * 1000)
      ret.sprintf("%.1f MB", static_cast<float>(bytes) /
                                 (1000 * 1000));  // NOLINT(runtime/printf)
    else
      ret.sprintf("%.1f GB",
                  static_cast<float>(bytes) /
                      (1000 * 1000 * 1000));  // NOLINT(runtime/printf)
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
  if (GetDiskFreeSpaceEx(
          QDir::toNativeSeparators(path).toLocal8Bit().constData(), nullptr,
          &ret, nullptr) != 0)
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
  if (GetDiskFreeSpaceEx(
          QDir::toNativeSeparators(path).toLocal8Bit().constData(), &ret,
          nullptr, nullptr) != 0)
    return ret.QuadPart;
#endif

  return 0;
}

QString MakeTempDir(const QString template_name) {
  QString path;
  {
    QTemporaryFile tempfile;
    if (!template_name.isEmpty()) tempfile.setFileTemplate(template_name);

    tempfile.open();
    path = tempfile.fileName();
  }

  QDir d;
  d.mkdir(path);

  return path;
}

QString GetTemporaryFileName() {
  QString file;
  {
    QTemporaryFile tempfile;
    // Do not delete the file, we want to do something with it
    tempfile.setAutoRemove(false);
    tempfile.open();
    file = tempfile.fileName();
  }

  return file;
}

QString SaveToTemporaryFile(const QByteArray& data) {
  QTemporaryFile tempfile;
  tempfile.setAutoRemove(false);

  if (!tempfile.open()) {
    return QString();
  }

  if (tempfile.write(data) != data.size()) {
    tempfile.remove();
    return QString();
  }

  tempfile.close();
  return tempfile.fileName();
}

bool RemoveRecursive(const QString& path) {
  QDir dir(path);
  for (const QString& child :
       dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden)) {
    if (!RemoveRecursive(path + "/" + child)) return false;
  }

  for (const QString& child :
       dir.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden)) {
    if (!QFile::remove(path + "/" + child)) return false;
  }

  if (!dir.rmdir(path)) return false;

  return true;
}

bool CopyRecursive(const QString& source, const QString& destination) {
  // Make the destination directory
  QString dir_name = source.section('/', -1, -1);
  QString dest_path = destination + "/" + dir_name;
  QDir().mkpath(dest_path);

  QDir dir(source);
  for (const QString& child :
       dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs)) {
    if (!CopyRecursive(source + "/" + child, dest_path)) {
      qLog(Warning) << "Failed to copy dir" << source + "/" + child << "to"
                    << dest_path;
      return false;
    }
  }

  for (const QString& child :
       dir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
    if (!QFile::copy(source + "/" + child, dest_path + "/" + child)) {
      qLog(Warning) << "Failed to copy file" << source + "/" + child << "to"
                    << dest_path;
      return false;
    }
  }
  return true;
}

bool Copy(QIODevice* source, QIODevice* destination) {
  if (!source->open(QIODevice::ReadOnly)) return false;

  if (!destination->open(QIODevice::WriteOnly)) return false;

  const qint64 bytes = source->size();
  std::unique_ptr<char[]> data(new char[bytes]);
  qint64 pos = 0;

  qint64 bytes_read;
  do {
    bytes_read = source->read(data.get() + pos, bytes - pos);
    if (bytes_read == -1) return false;

    pos += bytes_read;
  } while (bytes_read > 0 && pos != bytes);

  pos = 0;
  qint64 bytes_written;
  do {
    bytes_written = destination->write(data.get() + pos, bytes - pos);
    if (bytes_written == -1) return false;

    pos += bytes_written;
  } while (bytes_written > 0 && pos != bytes);

  return true;
}

QString ColorToRgba(const QColor& c) {
  return QString("rgba(%1, %2, %3, %4)")
      .arg(c.red())
      .arg(c.green())
      .arg(c.blue())
      .arg(c.alpha());
}

QString GetConfigPath(ConfigPath config) {
  switch (config) {
    case Path_Root: {
      if (Application::kIsPortable) {
        return QString("%1/data").arg(QCoreApplication::applicationDirPath());
      }
#ifdef Q_OS_DARWIN
      return mac::GetApplicationSupportPath() + "/" +
             QCoreApplication::organizationName();
#else
      return QString("%1/.config/%2")
          .arg(QDir::homePath(), QCoreApplication::organizationName());
#endif
    } break;

    case Path_CacheRoot: {
      if (Application::kIsPortable) {
        return GetConfigPath(Path_Root) + "/cache";
      }
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
      char* xdg = getenv("XDG_CACHE_HOME");
      if (!xdg || !*xdg) {
        return QString("%1/.cache/%2")
            .arg(QDir::homePath(), QCoreApplication::organizationName());
      } else {
        return QString("%1/%2").arg(xdg, QCoreApplication::organizationName());
      }
#else
      return GetConfigPath(Path_Root);
#endif
    } break;

    case Path_Icons:
      return GetConfigPath(Path_Root) + "/customiconset";

    case Path_AlbumCovers:
      return GetConfigPath(Path_Root) + "/albumcovers";

    case Path_NetworkCache:
      return GetConfigPath(Path_CacheRoot) + "/networkcache";

    case Path_MoodbarCache:
      return GetConfigPath(Path_CacheRoot) + "/moodbarcache";

    case Path_GstreamerRegistry:
      return GetConfigPath(Path_Root) +
             QString("/gst-registry-%1-bin")
                 .arg(QCoreApplication::applicationVersion());

    case Path_DefaultMusicLibrary:
#ifdef Q_OS_DARWIN
      return mac::GetMusicDirectory();
#else
      return QDir::homePath();
#endif

    case Path_LocalSpotifyBlob:
      return GetConfigPath(Path_Root) + "/spotifyblob";

    default:
      qFatal("%s", Q_FUNC_INFO);
      return QString::null;
  }
}

#ifdef Q_OS_DARWIN
// Better than openUrl(dirname(path)) - also highlights file at path
void RevealFileInFinder(QString const& path) {
  QProcess::execute("/usr/bin/open", QStringList() << "-R" << path);
}
#endif  // Q_OS_DARWIN

#ifdef Q_OS_WIN
void ShowFileInExplorer(QString const& path) {
  QProcess::execute("explorer.exe", QStringList()
                                        << "/select,"
                                        << QDir::toNativeSeparators(path));
}
#endif

void OpenInFileBrowser(const QList<QUrl>& urls) {
  QSet<QString> dirs;

  for (const QUrl& url : urls) {
    if (url.scheme() != "file") {
      continue;
    }
    QString path = url.toLocalFile();

    if (!QFile::exists(path)) continue;

    const QString directory = QFileInfo(path).dir().path();
    if (dirs.contains(directory)) continue;
    dirs.insert(directory);
    qLog(Debug) << path;
#ifdef Q_OS_DARWIN
    // revealing multiple files in the finder only opens one window,
    // so it also makes sense to reveal at most one per directory
    RevealFileInFinder(path);
#elif defined(Q_OS_WIN32)
    ShowFileInExplorer(path);
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
#endif
  }
}

QByteArray Hmac(const QByteArray& key, const QByteArray& data,
                HashFunction method) {
  const int kBlockSize = 64;  // bytes
  Q_ASSERT(key.length() <= kBlockSize);

  QByteArray inner_padding(kBlockSize, static_cast<char>(0x36));
  QByteArray outer_padding(kBlockSize, static_cast<char>(0x5c));

  for (int i = 0; i < key.length(); ++i) {
    inner_padding[i] = inner_padding[i] ^ key[i];
    outer_padding[i] = outer_padding[i] ^ key[i];
  }
  if (Md5_Algo == method) {
    return QCryptographicHash::hash(
        outer_padding + QCryptographicHash::hash(inner_padding + data,
                                                 QCryptographicHash::Md5),
        QCryptographicHash::Md5);
  } else if (Sha1_Algo == method) {
    return QCryptographicHash::hash(
        outer_padding + QCryptographicHash::hash(inner_padding + data,
                                                 QCryptographicHash::Sha1),
        QCryptographicHash::Sha1);
  } else {  // Sha256_Algo, currently default
    return Sha256(outer_padding + Sha256(inner_padding + data));
  }
}

QByteArray HmacSha256(const QByteArray& key, const QByteArray& data) {
  return Hmac(key, data, Sha256_Algo);
}

QByteArray HmacMd5(const QByteArray& key, const QByteArray& data) {
  return Hmac(key, data, Md5_Algo);
}

QByteArray HmacSha1(const QByteArray& key, const QByteArray& data) {
  return Hmac(key, data, Sha1_Algo);
}

QByteArray Sha256(const QByteArray& data) {
#ifndef USE_SYSTEM_SHA2
  using clementine_sha2::SHA256_CTX;
  using clementine_sha2::SHA256_Init;
  using clementine_sha2::SHA256_Update;
  using clementine_sha2::SHA256_Final;
  using clementine_sha2::SHA256_DIGEST_LENGTH;
#endif

  SHA256_CTX context;
  SHA256_Init(&context);
  SHA256_Update(&context, reinterpret_cast<const quint8*>(data.constData()),
                data.length());

  QByteArray ret(SHA256_DIGEST_LENGTH, '\0');
  SHA256_Final(reinterpret_cast<quint8*>(ret.data()), &context);

  return ret;
}

// File must not be open and will be closed afterwards!
QByteArray Sha1File(QFile& file) {
  file.open(QIODevice::ReadOnly);
  QCryptographicHash hash(QCryptographicHash::Sha1);
  QByteArray data;

  while (!file.atEnd()) {
    data = file.read(1000000);  // 1 mib
    hash.addData(data.data(), data.length());
    data.clear();
  }

  file.close();

  return hash.result();
}

QByteArray Sha1CoverHash(const QString& artist, const QString& album) {
  QCryptographicHash hash(QCryptographicHash::Sha1);
  hash.addData(artist.toLower().toUtf8().constData());
  hash.addData(album.toLower().toUtf8().constData());

  return hash.result();
}

QString PrettySize(const QSize& size) {
  return QString::number(size.width()) + "x" + QString::number(size.height());
}

void ForwardMouseEvent(const QMouseEvent* e, QWidget* target) {
  QMouseEvent c(e->type(), target->mapFromGlobal(e->globalPos()),
                e->globalPos(), e->button(), e->buttons(), e->modifiers());

  QApplication::sendEvent(target, &c);
}

bool IsMouseEventInWidget(const QMouseEvent* e, const QWidget* widget) {
  return widget->rect().contains(widget->mapFromGlobal(e->globalPos()));
}

quint16 PickUnusedPort() {
  forever {
    const quint16 port = 49152 + qrand() % 16384;

    QTcpServer server;
    if (server.listen(QHostAddress::Any, port)) {
      return port;
    }
  }
}

void ConsumeCurrentElement(QXmlStreamReader* reader) {
  int level = 1;
  while (level != 0 && !reader->atEnd()) {
    switch (reader->readNext()) {
      case QXmlStreamReader::StartElement:
        ++level;
        break;
      case QXmlStreamReader::EndElement:
        --level;
        break;
      default:
        break;
    }
  }
}

bool ParseUntilElement(QXmlStreamReader* reader, const QString& name) {
  while (!reader->atEnd()) {
    QXmlStreamReader::TokenType type = reader->readNext();
    switch (type) {
      case QXmlStreamReader::StartElement:
        if (reader->name() == name) {
          return true;
        }
        break;
      default:
        break;
    }
  }
  return false;
}

QDateTime ParseRFC822DateTime(const QString& text) {
  QRegExp regexp(
      "(\\d{1,2}) (\\w{3,12}) (\\d+) (\\d{1,2}):(\\d{1,2}):(\\d{1,2})");
  if (regexp.indexIn(text) == -1) {
    return QDateTime();
  }

  enum class MatchNames { DAYS = 1, MONTHS, YEARS, HOURS, MINUTES, SECONDS };

  QMap<QString, int> monthmap;
  monthmap["Jan"] = 1;
  monthmap["Feb"] = 2;
  monthmap["Mar"] = 3;
  monthmap["Apr"] = 4;
  monthmap["May"] = 5;
  monthmap["Jun"] = 6;
  monthmap["Jul"] = 7;
  monthmap["Aug"] = 8;
  monthmap["Sep"] = 9;
  monthmap["Oct"] = 10;
  monthmap["Nov"] = 11;
  monthmap["Dec"] = 12;
  monthmap["January"] = 1;
  monthmap["February"] = 2;
  monthmap["March"] = 3;
  monthmap["April"] = 4;
  monthmap["May"] = 5;
  monthmap["June"] = 6;
  monthmap["July"] = 7;
  monthmap["August"] = 8;
  monthmap["September"] = 9;
  monthmap["October"] = 10;
  monthmap["November"] = 11;
  monthmap["December"] = 12;

  const QDate date(regexp.cap(static_cast<int>(MatchNames::YEARS)).toInt(),
                   monthmap[regexp.cap(static_cast<int>(MatchNames::MONTHS))],
                   regexp.cap(static_cast<int>(MatchNames::DAYS)).toInt());

  const QTime time(regexp.cap(static_cast<int>(MatchNames::HOURS)).toInt(),
                   regexp.cap(static_cast<int>(MatchNames::MINUTES)).toInt(),
                   regexp.cap(static_cast<int>(MatchNames::SECONDS)).toInt());

  return QDateTime(date, time);
}

const char* EnumToString(const QMetaObject& meta, const char* name, int value) {
  int index = meta.indexOfEnumerator(name);
  if (index == -1) return "[UnknownEnum]";
  QMetaEnum metaenum = meta.enumerator(index);
  const char* result = metaenum.valueToKey(value);
  if (result == 0) return "[UnknownEnumValue]";
  return result;
}

QStringList Prepend(const QString& text, const QStringList& list) {
  QStringList ret(list);
  for (int i = 0; i < ret.count(); ++i) ret[i].prepend(text);
  return ret;
}

QStringList Updateify(const QStringList& list) {
  QStringList ret(list);
  for (int i = 0; i < ret.count(); ++i) ret[i].prepend(ret[i] + " = :");
  return ret;
}

QString DecodeHtmlEntities(const QString& text) {
  QString copy(text);
  copy.replace("&amp;", "&");
  copy.replace("&quot;", "\"");
  copy.replace("&apos;", "'");
  copy.replace("&lt;", "<");
  copy.replace("&gt;", ">");
  return copy;
}

int SetThreadIOPriority(IoPriority priority) {
#ifdef Q_OS_LINUX
  return syscall(SYS_ioprio_set, IOPRIO_WHO_PROCESS, GetThreadId(),
                 4 | priority << IOPRIO_CLASS_SHIFT);
#elif defined(Q_OS_DARWIN)
  return setpriority(PRIO_DARWIN_THREAD, 0,
                     priority == IOPRIO_CLASS_IDLE ? PRIO_DARWIN_BG : 0);
#else
  return 0;
#endif
}

int GetThreadId() {
#ifdef Q_OS_LINUX
  return syscall(SYS_gettid);
#else
  return 0;
#endif
}

bool IsLaptop() {
#ifdef Q_OS_WIN
  SYSTEM_POWER_STATUS status;
  if (!GetSystemPowerStatus(&status)) {
    return false;
  }

  return !(status.BatteryFlag & 128);  // 128 = no system battery
#elif defined(Q_OS_LINUX)
  return !QDir("/proc/acpi/battery")
              .entryList(QDir::Dirs | QDir::NoDotAndDotDot)
              .isEmpty();
#elif defined(Q_OS_MAC)
  ScopedCFTypeRef<CFTypeRef> power_sources(IOPSCopyPowerSourcesInfo());
  ScopedCFTypeRef<CFArrayRef> power_source_list(
      IOPSCopyPowerSourcesList(power_sources.get()));
  for (CFIndex i = 0; i < CFArrayGetCount(power_source_list.get()); ++i) {
    CFTypeRef ps = CFArrayGetValueAtIndex(power_source_list.get(), i);
    CFDictionaryRef description =
        IOPSGetPowerSourceDescription(power_sources.get(), ps);

    if (CFDictionaryContainsKey(description, CFSTR(kIOPSBatteryHealthKey))) {
      return true;
    }
  }
  return false;
#else
  return false;
#endif
}

QString SystemLanguageName() {
#if QT_VERSION >= 0x040800
  QString system_language = QLocale::system().uiLanguages().empty()
                                ? QLocale::system().name()
                                : QLocale::system().uiLanguages().first();
  // uiLanguages returns strings with "-" as separators for language/region;
  // however QTranslator needs "_" separators
  system_language.replace("-", "_");
#else
  QString system_language = QLocale::system().name();
#endif

  return system_language;
}

bool UrlOnSameDriveAsClementine(const QUrl& url) {
  if (url.scheme() != "file") return false;

#ifdef Q_OS_WIN
  QUrl appUrl = QUrl::fromLocalFile(QCoreApplication::applicationDirPath());
  if (url.toLocalFile().left(1) == appUrl.toLocalFile().left(1))
    return true;
  else
    return false;
#else
  // Non windows systems have always a / in the path
  return true;
#endif
}

QUrl GetRelativePathToClementineBin(const QUrl& url) {
  QDir appPath(QCoreApplication::applicationDirPath());
  return QUrl::fromLocalFile(appPath.relativeFilePath(url.toLocalFile()));
}

QString PathWithoutFilenameExtension(const QString& filename) {
  if (filename.section('/', -1, -1).contains('.'))
    return filename.section('.', 0, -2);
  return filename;
}

QString FiddleFileExtension(const QString& filename,
                            const QString& new_extension) {
  return PathWithoutFilenameExtension(filename) + "." + new_extension;
}

}  // namespace Utilities

ScopedWCharArray::ScopedWCharArray(const QString& str)
    : chars_(str.length()), data_(new wchar_t[chars_ + 1]) {
  str.toWCharArray(data_.get());
  data_[chars_] = '\0';
}
