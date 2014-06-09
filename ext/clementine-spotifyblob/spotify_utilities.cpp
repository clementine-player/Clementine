/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// Note: this file is licensed under the Apache License instead of GPL because
// it is used by the Spotify blob which links against libspotify and is not GPL
// compatible.

#include "spotify_utilities.h"

#include <stdlib.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace utilities {

QString GetCacheDirectory() {
  QString user_cache = GetUserDataDirectory();
  return user_cache + "/" + QCoreApplication::applicationName() +
         "/spotify-cache";
}

#ifndef Q_OS_DARWIN  // See spotify_utilities.mm for Mac implementation.
QString GetUserDataDirectory() {
  const char* xdg_cache_dir = getenv("XDG_CACHE_HOME");
  if (!xdg_cache_dir) {
    return QDir::homePath() + "/.config";
  }
  return QString::fromLocal8Bit(xdg_cache_dir);
}

QString GetSettingsDirectory() {
  QString ret;

#ifdef Q_OS_WIN32
  ret = GetUserDataDirectory() + "/" + QCoreApplication::applicationName() +
        "/spotify-settings";
#else
  ret = QFileInfo(QSettings().fileName()).absolutePath() + "/spotify-settings";
#endif  // Q_OS_WIN32

  // Create the directory
  QDir dir;
  dir.mkpath(ret);

  return ret;
}

#endif  // Q_OS_DARWIN

}  // namespace utilities
