#ifndef SPOTIFY_UTILITIES_H
#define SPOTIFY_UTILITIES_H

#include <QString>

namespace utilities {

// Get the path to the current user's local cache for all apps.
QString GetUserCacheDirectory();
// Get the path for Clementine's cache.
QString GetCacheDirectory();

}

#endif
