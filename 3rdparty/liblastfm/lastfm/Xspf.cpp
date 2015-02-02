/*
   Copyright 2009 Last.fm Ltd. 
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

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

#include "Xspf.h"
#include "XmlQuery.h"

#include <QTimer>
#include <QUrl>


class lastfm::XspfPrivate
{
    public:
        QList<Track> tracks;
        QString title;
};


lastfm::Xspf::Xspf( const QDomElement& playlist_node, QObject* parent )
    :QObject( parent )
    , d( new XspfPrivate )
{
    XmlQuery e( playlist_node );

    int expirySeconds = e["link rel=http://www.last.fm/expiry"].text().toInt();
    QTimer::singleShot( expirySeconds * 1000, this, SLOT(onExpired()));
    
    d->title = e["title"].text();
        
    //FIXME should we use UnicornUtils::urlDecode()?
    //The title is url encoded, has + instead of space characters 
    //and has a + at the begining. So it needs cleaning up:
    d->title.replace( '+', ' ' );
    d->title = QUrl::fromPercentEncoding( d->title.toUtf8() );
    d->title = d->title.trimmed();
    
    foreach (XmlQuery e, e["trackList"].children( "track" ))
    {
        MutableTrack t;
        t.setUrl( e["location"].text() );
        t.setImageUrl( AbstractType::LargeImage, e["image"].text() );
        t.setExtra( "trackauth", e["extension"]["trackauth"].text() );
        t.setTitle( e["title"].text() );
        t.setArtist( e["creator"].text() );
        t.setAlbum( e["album"].text() );
        t.setDuration( e["duration"].text().toInt() / 1000 );
        t.setLoved( e["extension"]["loved"].text() == "1" );
        t.setSource( Track::LastFmRadio );
        t.setExtra( "expiry", QString::number( QDateTime::currentDateTime().addSecs( expirySeconds ).toTime_t() ) );
        t.setExtra( "playlistTitle", d->title );

        QList<QString> contexts;
        QDomNodeList contextsNodeList = QDomElement(e["extension"]["context"]).childNodes();

        for ( int i = 0 ; i < contextsNodeList.count() ; ++i )
            contexts.append( contextsNodeList.item(i).toElement().text() );

        if ( contexts.count() > 0 )
            t.setContext( TrackContext( contextsNodeList.item(0).toElement().tagName(), contexts ) );

        d->tracks << t; // outside try block since location is enough basically
    }
}


lastfm::Xspf::~Xspf()
{
    delete d;
}


QString
lastfm::Xspf::title() const
{
    return d->title;
}


bool
lastfm::Xspf::isEmpty() const
{
    return d->tracks.isEmpty();
}


lastfm::Track
lastfm::Xspf::takeFirst()
{
    return d->tracks.takeFirst();
}

QList<lastfm::Track>
lastfm::Xspf::tracks() const
{
    return d->tracks;
}

void
lastfm::Xspf::onExpired()
{
    emit expired();
}
