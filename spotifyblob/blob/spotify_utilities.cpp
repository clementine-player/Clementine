#include "spotify_utilities.h"

#include <stdlib.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace utilities {

#ifndef Q_OS_DARWIN  // See spotify_utilities.mm for Mac implementation.
QString GetUserCacheDirectory() {
#ifndef Q_OS_WINDOWS
  const char* xdg_cache_dir = getenv("XDG_CACHE_HOME");
  if (!xdg_cache_dir) {
    return QDir::homePath() + "/.config";
  }
  return QString::fromLocal8Bit(xdg_cache_dir);

#else  // Q_OS_WINDOWS
  const char* cache_dir = getenv("APPDATA");
  if (!cache_dir) {
    return QDir::homePath() + "/.config/";
  }
  return QDir::fromNativeSeparators(QString::fromLocal8Bit(cache_dir));
#endif  // Q_OS_WINDOWS
}
#endif  // Q_OS_DARWIN

QString GetCacheDirectory() {
  QString user_cache = GetUserCacheDirectory();
  return user_cache + "/" + QCoreApplication::applicationName() + "/spotify-cache";
}

QString GetSettingsDirectory() {
  QSettings settings;
  QString file_name = settings.fileName();
  QFileInfo info(file_name);
  QDir dir = info.dir();
  dir.mkdir("spotify-settings");
  return info.absolutePath() + "/spotify-settings";
}

}  // namespace utilities
