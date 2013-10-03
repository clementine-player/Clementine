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

#include "utilities.h"

#include <stdlib.h>

#include <boost/scoped_array.hpp>

#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
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
#include "timeconstants.h"

#include "sha2.h"

#if defined(Q_OS_UNIX)
#  include <sys/statvfs.h>
#elif defined(Q_OS_WIN32)
#  include <windows.h>
#  include <QProcess>
#endif

#ifdef Q_OS_LINUX
#  include <sys/syscall.h>
#endif
#ifdef Q_OS_DARWIN
#  include <sys/resource.h>
#endif

#ifdef Q_OS_DARWIN
#  include "core/mac_startup.h"
#  include "core/mac_utilities.h"
#  include "core/scoped_cftyperef.h"
#  include "CoreServices/CoreServices.h"
#  include "IOKit/ps/IOPowerSources.h"
#  include "IOKit/ps/IOPSKeys.h"
#  include <QProcess>
#endif

namespace Utilities {

static QString tr(const char* str) {
  return QCoreApplication::translate("", str);
}

QString PrettyTimeDelta(int seconds) {
  return (seconds >= 0 ? "+" : "-") + PrettyTime(seconds);
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

QString PrettyTimeNanosec(qint64 nanoseconds) {
  return PrettyTime(nanoseconds / kNsecPerSec);
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

QString WordyTimeNanosec(qint64 nanoseconds) {
  return WordyTime(nanoseconds / kNsecPerSec);
}

QString Ago(int seconds_since_epoch, const QLocale& locale) {
  const QDateTime now = QDateTime::currentDateTime();
  const QDateTime then = QDateTime::fromTime_t(seconds_since_epoch);
  const int days_ago = then.date().daysTo(now.date());
  const QString time = then.time().toString(locale.timeFormat(QLocale::ShortFormat));

  if (days_ago == 0)
    return tr("Today") + " " + time;
  if (days_ago == 1)
    return tr("Yesterday") + " " + time;
  if (days_ago <= 7)
    return tr("%1 days ago").arg(days_ago);

  return then.date().toString(locale.dateFormat(QLocale::ShortFormat));
}

QString PrettyFutureDate(const QDate& date) {
  const QDate now = QDate::currentDate();
  const int delta_days = now.daysTo(date);

  if (delta_days < 0)
    return QString();
  if (delta_days == 0)
    return tr("Today");
  if (delta_days == 1)
    return tr("Tomorrow");
  if (delta_days <= 7)
    return tr("In %1 days").arg(delta_days);
  if (delta_days <= 14)
    return tr("Next week");

  return tr("In %1 weeks").arg(delta_days / 7);
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

QString MakeTempDir(const QString template_name) {
  QString path;
  {
    QTemporaryFile tempfile;
    if (!template_name.isEmpty())
      tempfile.setFileTemplate(template_name);

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
    tempfile.open();
    file = tempfile.fileName();
  }

  return file;
}

void RemoveRecursive(const QString& path) {
  QDir dir(path);
  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden))
    RemoveRecursive(path + "/" + child);

  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden))
    QFile::remove(path + "/" + child);

  dir.rmdir(path);
}

bool CopyRecursive(const QString& source, const QString& destination) {
  // Make the destination directory
  QString dir_name = source.section('/', -1, -1);
  QString dest_path = destination + "/" + dir_name;
  QDir().mkpath(dest_path);

  QDir dir(source);
  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs)) {
    if (!CopyRecursive(source + "/" + child, dest_path)) {
      qLog(Warning) << "Failed to copy dir" << source + "/" + child << "to" << dest_path;
      return false;
    }
  }

  foreach (const QString& child, dir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
    if (!QFile::copy(source + "/" + child, dest_path + "/" + child)) {
      qLog(Warning) << "Failed to copy file" << source + "/" + child << "to" << dest_path;
      return false;
    }
  }
  return true;
}

bool Copy(QIODevice* source, QIODevice* destination) {
  if (!source->open(QIODevice::ReadOnly))
    return false;

  if (!destination->open(QIODevice::WriteOnly))
    return false;

  const qint64 bytes = source->size();
  boost::scoped_array<char> data(new char[bytes]);
  qint64 pos = 0;

  qint64 bytes_read;
  do {
    bytes_read = source->read(data.get() + pos, bytes - pos);
    if (bytes_read == -1)
      return false;

    pos += bytes_read;
  } while (bytes_read > 0 && pos != bytes);

  pos = 0;
  qint64 bytes_written;
  do {
    bytes_written = destination->write(data.get() + pos, bytes - pos);
    if (bytes_written == -1)
      return false;

    pos += bytes_written;
  } while (bytes_written > 0 && pos != bytes);

  return true;
}

QString ColorToRgba(const QColor& c) {
  return QString("rgba(%1, %2, %3, %4)")
      .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

QString GetConfigPath(ConfigPath config) {
  switch (config) {
    case Path_Root: {
      if (Application::kIsPortable) {
        return QString("%1/data").arg(QCoreApplication::applicationDirPath());
      }
      #ifdef Q_OS_DARWIN
        return mac::GetApplicationSupportPath() + "/" + QCoreApplication::organizationName();
      #else
        return QString("%1/.config/%2").arg(QDir::homePath(), QCoreApplication::organizationName());
      #endif
    }
    break;

    case Path_CacheRoot: {
      if (Application::kIsPortable) {
        return GetConfigPath(Path_Root) + "/cache";
      }
      #if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
        char* xdg = getenv("XDG_CACHE_HOME");
        if (!xdg || !*xdg) {
          return QString("%1/.cache/%2").arg(QDir::homePath(), QCoreApplication::organizationName());
        } else {
          return QString("%1/%2").arg(xdg, QCoreApplication::organizationName());
        }
      #else
        return GetConfigPath(Path_Root);
      #endif
    }
    break;

    case Path_AlbumCovers:
      return GetConfigPath(Path_Root) + "/albumcovers";

    case Path_NetworkCache:
      return GetConfigPath(Path_CacheRoot) + "/networkcache";

    case Path_MoodbarCache:
      return GetConfigPath(Path_CacheRoot) + "/moodbarcache";

    case Path_GstreamerRegistry:
      return GetConfigPath(Path_Root) +
          QString("/gst-registry-%1-bin").arg(QCoreApplication::applicationVersion());

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
qint32 GetMacVersion() {
  SInt32 minor_version;
  Gestalt(gestaltSystemVersionMinor, &minor_version);
  return minor_version;
}

// Better than openUrl(dirname(path)) - also highlights file at path
void RevealFileInFinder(QString const& path) {
  QProcess::execute("/usr/bin/open", QStringList() << "-R" << path);
}
#endif  // Q_OS_DARWIN

#ifdef Q_OS_WIN
void ShowFileInExplorer(QString const& path) {
  QProcess::execute("explorer.exe", QStringList() << "/select,"
                                                  << QDir::toNativeSeparators(path));
}
#endif

void OpenInFileBrowser(const QList<QUrl>& urls) {
  QSet<QString> dirs;

  foreach (const QUrl& url, urls) {
    if (url.scheme() != "file") {
      continue;
    }
    QString path = url.toLocalFile();

    if (!QFile::exists(path))
      continue;

    const QString directory = QFileInfo(path).dir().path();
    if (dirs.contains(directory))
      continue;
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

QByteArray Hmac(const QByteArray& key, const QByteArray& data, HashFunction method) {
  const int kBlockSize = 64; // bytes
  Q_ASSERT(key.length() <= kBlockSize);

  QByteArray inner_padding(kBlockSize, char(0x36));
  QByteArray outer_padding(kBlockSize, char(0x5c));

  for (int i=0 ; i<key.length() ; ++i) {
    inner_padding[i] = inner_padding[i] ^ key[i];
    outer_padding[i] = outer_padding[i] ^ key[i];
  }
  if (Md5_Algo == method) {
    return QCryptographicHash::hash(outer_padding +
                                    QCryptographicHash::hash(inner_padding + data,
                                                             QCryptographicHash::Md5),
                                    QCryptographicHash::Md5);
  } else if (Sha1_Algo == method) {
    return QCryptographicHash::hash(outer_padding +
                                    QCryptographicHash::hash(inner_padding + data,
                                                             QCryptographicHash::Sha1),
                                    QCryptographicHash::Sha1);
  } else { // Sha256_Algo, currently default
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
  SHA256_CTX context;
  SHA256_Init(&context);
  SHA256_Update(&context, reinterpret_cast<const u_int8_t*>(data.constData()),
                data.length());

  QByteArray ret(SHA256_DIGEST_LENGTH, '\0');
  SHA256_Final(reinterpret_cast<u_int8_t*>(ret.data()), &context);

  return ret;
}

QString PrettySize(const QSize& size) {
  return QString::number(size.width()) + "x" +
         QString::number(size.height());
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
      case QXmlStreamReader::StartElement: ++level; break;
      case QXmlStreamReader::EndElement:   --level; break;
      default: break;
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
  // This sucks but we need it because some podcasts don't quite follow the
  // spec properly - they might have 1-digit hour numbers for example.

  QRegExp re("([a-zA-Z]{3}),? (\\d{1,2}) ([a-zA-Z]{3}) (\\d{4}) (\\d{1,2}):(\\d{1,2}):(\\d{1,2})");
  if (re.indexIn(text) == -1)
    return QDateTime();

  return QDateTime(
    QDate::fromString(QString("%1 %2 %3 %4").arg(re.cap(1), re.cap(3), re.cap(2), re.cap(4)), Qt::TextDate),
    QTime(re.cap(5).toInt(), re.cap(6).toInt(), re.cap(7).toInt()));
}

const char* EnumToString(const QMetaObject& meta, const char* name, int value) {
  int index = meta.indexOfEnumerator(name);
  if (index == -1)
    return "[UnknownEnum]";
  QMetaEnum metaenum = meta.enumerator(index);
  const char* result = metaenum.valueToKey(value);
  if (result == 0)
    return "[UnknownEnumValue]";
  return result;
}

QStringList Prepend(const QString& text, const QStringList& list) {
  QStringList ret(list);
  for (int i=0 ; i<ret.count() ; ++i)
    ret[i].prepend(text);
  return ret;
}

QStringList Updateify(const QStringList& list) {
  QStringList ret(list);
  for (int i=0 ; i<ret.count() ; ++i)
    ret[i].prepend(ret[i] + " = :");
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

  return !(status.BatteryFlag & 128); // 128 = no system battery
#elif defined(Q_OS_LINUX)
  return !QDir("/proc/acpi/battery").entryList(QDir::Dirs | QDir::NoDotAndDotDot).isEmpty();
#elif defined(Q_OS_MAC)
  ScopedCFTypeRef<CFTypeRef> power_sources(IOPSCopyPowerSourcesInfo());
  ScopedCFTypeRef<CFArrayRef> power_source_list(
      IOPSCopyPowerSourcesList(power_sources.get()));
  for (CFIndex i = 0; i < CFArrayGetCount(power_source_list.get()); ++i) {
    CFTypeRef ps = CFArrayGetValueAtIndex(power_source_list.get(), i);
    CFDictionaryRef description = IOPSGetPowerSourceDescription(
        power_sources.get(), ps);

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
  QString system_language = QLocale::system().uiLanguages().empty() ?
      QLocale::system().name() : QLocale::system().uiLanguages().first();
  // uiLanguages returns strings with "-" as separators for language/region;
  // however QTranslator needs "_" separators
  system_language.replace("-", "_");
#else
  QString system_language = QLocale::system().name();
#endif

  return system_language;
}

bool UrlOnSameDriveAsClementine(const QUrl &url) {
  if (url.scheme() != "file")
    return false;

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

}  // namespace Utilities


ScopedWCharArray::ScopedWCharArray(const QString& str)
  : chars_(str.length()),
    data_(new wchar_t[chars_ + 1])
{
  str.toWCharArray(data_.get());
  data_[chars_] = '\0';
}
