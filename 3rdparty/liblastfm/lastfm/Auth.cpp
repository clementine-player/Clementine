/*
   Copyright 2012 Last.fm Ltd.
      - Primarily authored by Michael Coffey

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Auth.h"
#include "ws.h"


lastfm::Auth::Auth()
{
}


QNetworkReply*
lastfm::Auth::getSessionInfo()
{
    QMap<QString, QString> map;
    map["method"] = "Auth.getSessionInfo";
    if (!lastfm::ws::Username.isEmpty()) map["username"] = lastfm::ws::Username;
    return nam()->get( QNetworkRequest( lastfm::ws::url( map, true ) ) );
}

QNetworkReply*
lastfm::Auth::getMobileSession( const QString& username, const QString& password )
{
    QMap<QString, QString> map;
    map["method"] = "Auth.getMobileSession";
    map["username"] = username;
    map["password"] = password;

    return 0;
    //return nam()->post( QNetworkRequest( lastfm::ws::url( map, true ) ) );
}
