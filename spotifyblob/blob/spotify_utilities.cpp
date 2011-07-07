#include "spotify_utilities.h"

#include <stdlib.h>

#include <QCoreApplication>
#include <QDir>

namespace utilities {

QString GetUserCacheDirectory() {
#ifndef Q_OS_WINDOWS
  const char* xdg_cache_dir = getenv("XDG_CACHE_HOME");
  if (!xdg_cache_dir) {
    return QDir::homePath() + "/.config";
  }
  return QString::fromLocal8Bit(xdg_cache_dir);

#else
  const char* cache_dir = getenv("APPDATA");
  if (!cache_dir) {
    return QDir::homePath() + "/.config/";
  }
  return QDir::fromNativeSeparators(QString::fromLocal8Bit(cache_dir));
#endif
}

QString GetCacheDirectory() {
  QString user_cache = GetUserCacheDirectory();
  return user_cache + "/" + QCoreApplication::applicationName() + "/spotify-cache";
}

}  // namespace utilities
