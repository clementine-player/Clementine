/*
   Copyright 2009 Last.fm Ltd. 

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

#include "RadioStation.h"
#include "XmlQuery.h"

#include <QDebug>
#include <QRegExp>
#include <QStringList>


const float k_defaultRep(0.5);
const float k_defaultMainstr(0.5);
const bool k_defaultDisco(false);

class lastfm::RadioStationData : public QSharedData
{
public:
    QUrl m_url;
    QString m_title;
    QString m_tagFilter;

    float m_rep;
    float m_mainstr;
    bool m_disco;
};


lastfm::RadioStation::RadioStation()
    : d( new RadioStationData )
{
}

lastfm::RadioStation::RadioStation( const QString& s )
    : d( new RadioStationData )
{
    // If it's a tag filtered station then extract that part
    QString tempString = s;

    if ( !tempString.startsWith("lastfm://tag/") )
    {
        int index = tempString.indexOf("/tag/");

        if ( index != -1 )
        {
            d->m_tagFilter = tempString.mid( index + 5, tempString.count() - (index + 5) );
            tempString = tempString.mid( 0, index );
        }
    }

    d->m_url = tempString;
}

lastfm::RadioStation::RadioStation( const RadioStation& other )
    : d(other.d)
{
}

lastfm::RadioStation&
lastfm::RadioStation::operator=( const RadioStation& that )
{
    d = that.d;
    return *this;
}

lastfm::RadioStation::~RadioStation()
{
}

lastfm::RadioStation
lastfm::RadioStation::library( const lastfm::User& user )
{
    QList<lastfm::User> users;
    users << user;
    return library( users );
}

lastfm::RadioStation
lastfm::RadioStation::library( QList<lastfm::User>& users )
{
    qSort(users.begin(), users.end());

    QString url = (users.count() > 1) ? "lastfm://users/" : "lastfm://user/";

    url.append( users[0].name() );

    for ( int i = 1 ; i < users.count() ; ++i )
        url.append( "," + users[i].name() );

    url.append("/personal");

    RadioStation s( url );
    if( users.count() != 1 )
    {
        QString title;

        for( QList<lastfm::User>::const_iterator i = users.constBegin(); i != users.constEnd(); i++ )
        {
            if( i == users.constEnd() - 1 )
                title += " and " + *i;
            else
                title += ", " + *i;
        }

        s.setTitle( title );
    }

    return s;
}


lastfm::RadioStation
lastfm::RadioStation::recommendations( const lastfm::User& user )
{
    return RadioStation( "lastfm://user/" + user + "/recommended" );
}

lastfm::RadioStation
lastfm::RadioStation::friends( const lastfm::User& user )
{
    return RadioStation( "lastfm://user/" + user + "/friends" );
}

lastfm::RadioStation
lastfm::RadioStation::neighbourhood( const lastfm::User& user )
{
    return RadioStation( "lastfm://user/" + user + "/neighbours" );
}


lastfm::RadioStation
lastfm::RadioStation::tag( const lastfm::Tag& tag )
{
    QList<lastfm::Tag> tags;
    tags << tag;
    return lastfm::RadioStation::tag( tags );
}


lastfm::RadioStation
lastfm::RadioStation::tag( QList<lastfm::Tag>& tag )
{
    qSort(tag.begin(), tag.end());

    QString url = (tag.count() > 1) ? "lastfm://tag/" : "lastfm://globaltags/";

    url.append( tag[0].name() );

    for ( int i = 1 ; i < tag.count() ; ++i )
        url.append( "*" + tag[i].name() );

    return RadioStation( url );
}


lastfm::RadioStation
lastfm::RadioStation::similar( const lastfm::Artist& artist )
{
    QList<lastfm::Artist> artists;
    artists << artist;
    return similar( artists );
}


lastfm::RadioStation
lastfm::RadioStation::similar( QList<lastfm::Artist>& artists )
{
    qSort(artists.begin(), artists.end());

    QString url = (artists.count() > 1) ? "lastfm://artistnames/" : "lastfm://artist/";

    url.append( artists[0].name() );

    for ( int i = 1 ; i < artists.count() ; ++i )
        url.append( "," + artists[i].name() );

    if (artists.count() == 1)
        url.append( "/similarartists" );

    return RadioStation( url );
}


lastfm::RadioStation
lastfm::RadioStation::mix( const lastfm::User& user )
{
    return RadioStation( "lastfm://user/" + user + "/mix" );
}


QString
lastfm::RadioStation::url() const
{
    return d->m_url.toString() + (d->m_tagFilter.isEmpty() ? "" : "/tag/" + d->m_tagFilter);
}


void
lastfm::RadioStation::setTitle( const QString& title )
{
    // Stop the radio station getting renamed when the web services don't know what it's called
    if ( !d->m_title.isEmpty() && title.compare( "a radio station", Qt::CaseInsensitive ) == 0 )
        return;

    // do not rename the current user's stations if they already have a name
    if ( !d->m_title.isEmpty() && d->m_url.toString().startsWith( "lastfm://user/" + User().name() ) )
        return;

    d->m_title = title.trimmed();
}

void
lastfm::RadioStation::setUrl( const QString& url )
{
    d->m_url = url;
}


QString
lastfm::RadioStation::title() const
{
    return d->m_title; // + (d->m_tagFilter.isEmpty() ? "" : ": " + d->m_tagFilter);
}


void
lastfm::RadioStation::setTagFilter( const QString& tag )
{
    d->m_tagFilter = tag;
}


QNetworkReply*
lastfm::RadioStation::getSampleArtists( int limit ) const
{
    QMap<QString, QString> map;
    map["method"] = "radio.getSampleArtists";
    map["station"] = d->m_url.toString();
    map["limit"] = QString::number( limit );
    return ws::get( map );
}


QNetworkReply*
lastfm::RadioStation::getTagSuggestions( int limit ) const
{
    QMap<QString, QString> map;
    map["method"] = "radio.getTagSuggestions";
    map["station"] = d->m_url.toString();
    map["limit"] = QString::number( limit );
    return ws::get( map );
}


bool
lastfm::RadioStation::isLegacyPlaylist() const
{
    return d->m_url.toString().startsWith( "lastfm://play/" ) ||
           d->m_url.toString().startsWith( "lastfm://preview/" ) ||
           d->m_url.toString().startsWith( "lastfm://track/" ) ||
           d->m_url.toString().startsWith( "lastfm://playlist/" );
}


//static 
QList<lastfm::RadioStation> 
lastfm::RadioStation::list( QNetworkReply* r )
{
    QList<lastfm::RadioStation> result;
    XmlQuery lfm;

    if ( lfm.parse( r ) )
    {

        foreach (XmlQuery xq, lfm.children("station"))
        {
            lastfm::RadioStation rs( QUrl::fromPercentEncoding( xq["url"].text().toUtf8() ) );
            rs.setTitle(xq["name"].text());
            result.append(rs);
        }
    }
    else
    {
        qWarning() << lfm.parseError().message();
    }

    return result;
}


bool
lastfm::RadioStation::operator==( const RadioStation& that ) const
{
    return this->d->m_url == that.d->m_url && this->d->m_tagFilter == that.d->m_tagFilter;
}


void
lastfm::RadioStation::setRep(float rep)
{
    d->m_rep = rep;
}


void
lastfm::RadioStation::setMainstr(float mainstr)
{
    d->m_mainstr = mainstr;
}


void
lastfm::RadioStation::setDisco(bool disco)
{
    d->m_disco = disco;
}


float lastfm::RadioStation::rep() const
{
    return d->m_rep;
}


float lastfm::RadioStation::mainstr() const
{
    return d->m_mainstr;
}


bool lastfm::RadioStation::disco() const
{
    return d->m_disco;
}


QDebug operator<<( QDebug d, const lastfm::RadioStation& station )
{
    return d << station.url();
}
