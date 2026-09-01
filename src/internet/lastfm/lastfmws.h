/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_LASTFM_LASTFMWS_H_
#define INTERNET_LASTFM_LASTFMWS_H_

#include <QMap>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

// Minimal client for Last.fm's web service API
// (https://www.last.fm/api/webauth), used in place of the liblastfm library
// (which has no Qt6 build). Only what Clementine actually calls is
// implemented: signed/unsigned requests to https://ws.audioscrobbler.com/2.0/
// with the credentials and session below.
namespace LastFmWs {

// Set once at startup from LastFMService::kApiKey/kSecret.
extern QString api_key;
extern QString shared_secret;

// The logged-in user, persisted to and restored from QSettings by
// LastFMService. Empty when not authenticated.
extern QString username;
extern QString session_key;

// Takes ownership of the network access manager used for every request
// issued through Get()/Post().
void SetNetworkAccessManager(QNetworkAccessManager* network);

// Computes api_sig: params sorted by key, concatenated as key+value pairs,
// with the shared secret appended, then MD5-hashed - see
// https://www.last.fm/api/authspec#8.
QByteArray Sign(const QMap<QString, QString>& params);

// Issues an unsigned GET request with api_key added. Used for public,
// unauthenticated methods like track.getInfo.
QNetworkReply* Get(QMap<QString, QString> params);

// Issues a signed POST request with api_key added, and sk (the session key)
// added too if one is set - i.e. this both handles public methods like
// album.search and authenticated ones like track.scrobble.
QNetworkReply* Post(QMap<QString, QString> params);

}  // namespace LastFmWs

#endif  // INTERNET_LASTFM_LASTFMWS_H_
