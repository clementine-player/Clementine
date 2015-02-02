/*
   Copyright 2011 Last.fm Ltd.
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
#include "Chart.h"
#include "ws.h"

#include <QNetworkReply>


lastfm::Chart::Chart()
{
}

QNetworkReply*
lastfm::Chart::getHypedArtists( int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "chart.getHypedArtists";
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::get( map );
}

QNetworkReply*
lastfm::Chart::getHypedTracks( int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "chart.getHypedTracks";
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::get( map );
}

QNetworkReply*
lastfm::Chart::getLovedTracks( int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "chart.getLovedTracks";
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::get( map );
}

QNetworkReply*
lastfm::Chart::getTopArtists( int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "chart.getTopArtists";
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::get( map );
}

QNetworkReply*
lastfm::Chart::getTopDownloads( int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "chart.getTopDownloads";
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::get( map );
}

QNetworkReply*
lastfm::Chart::getTopTags( int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "chart.getTopTags";
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::get( map );
}

QNetworkReply*
lastfm::Chart::getTopTracks( int limit, int page )
{
    QMap<QString, QString> map;
    map["method"] = "chart.getTopTracks";
    if ( page != -1 ) map["page"] = QString::number( page );
    if ( limit != -1 ) map["limit"] = QString::number( limit );
    return ws::get( map );
}
