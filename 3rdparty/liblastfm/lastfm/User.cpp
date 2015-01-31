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
#include "User.h"
#include "Track.h"
#include "Artist.h"
#include "UrlBuilder.h"
#include "XmlQuery.h"

#include <QStringList>
#include <QAbstractNetworkCache>
#include <QDebug>

using lastfm::Gender;
using lastfm::User;
using lastfm::UserList;
using lastfm::XmlQuery;


class lastfm::UserListPrivate
{
public:
    UserListPrivate()
        : total( 0 ), page( 0 ), perPage( 0 ), totalPages( 0 )
    {}
    int total;
    int page;
    int perPage;
    int totalPages;
    QList<lastfm::User> users;
};

lastfm::UserList::UserList()
    : d( new UserListPrivate )
{
}

lastfm::UserList::UserList( const XmlQuery &lfm )
    : d( new UserListPrivate )
{
    if ( lfm.parseError().enumValue() == lastfm::ws::NoError )
    {
        foreach (XmlQuery e, lfm.children( "user" ))
        {
            User u( e );
            d->users.append( u );
        }

        d->total = lfm["friends"].attribute("total").toInt();
        d->page = lfm["friends"].attribute("page").toInt();
        d->perPage = lfm["friends"].attribute("perPage").toInt();
        d->totalPages = lfm["friends"].attribute("totalPages").toInt();
    }
    else
    {
        qDebug() << lfm.parseError().message() << lfm.parseError().enumValue();
    }
}

lastfm::UserList::~UserList()
{
    delete d;
}

lastfm::UserList::UserList( const UserList& other )
    : d( new UserListPrivate( *other.d ) )
{
}

QList<lastfm::User>
lastfm::UserList::users()
{
    return d->users;
}

lastfm::UserList&
lastfm::UserList::operator=( const UserList& other )
{
    d->total = other.d->total;
    d->page = other.d->page;
    d->perPage = other.d->perPage;
    d->totalPages = other.d->totalPages;
    d->users = other.d->users;
    return *this;
}

int
lastfm::UserList::totalUsers()
{
    return d->total;
}

int
lastfm::UserList::currentPage()
{
    return d->page;
}

int
lastfm::UserList::usersPerPage()
{
    return d->perPage;
}

int
lastfm::UserList::totalPages()
{
    return d->totalPages;
}


class lastfm::Gender::GenderPrivate
{
    public:
        QString s;
};


Gender::Gender()
    :d( new GenderPrivate )
{
}

Gender::Gender( const Gender& other )
    :d( new GenderPrivate( *other.d ) )
{
}


Gender::Gender( const QString& ss )
    :d( new GenderPrivate )
{
    d->s = ss.toLower();
}


Gender::~Gender()
{
    delete d;
}


bool
Gender::known() const
{
    return male() || female();
}


bool
Gender::male() const
{
    return d->s == "m";
}


bool
Gender::female() const
{
    return d->s == "f";
}

QString Gender::toString() const
{
    QString result;

    if (male())
        result = "Male";
    else if (female())
        result = "Female";
    else
        result = "Neuter";

    return result;
}


Gender&
Gender::operator=( const lastfm::Gender& that )
{
    d->s = that.d->s;
    return *this;
}



class lastfm::User::UserPrivate
{
    public:
        UserPrivate()
            : m_name( lastfm::ws::Username )
            , m_match( -1.0f )
            , m_age( 0 )
            , m_scrobbles( 0 )
            , m_registered( QDateTime() )
            , m_isSubscriber( false )
            , m_canBootstrap( false )
        {}
        QString m_name;
        Type m_type;
        QList<QUrl> m_images;
        float m_match;
        QString m_realName;
        Gender m_gender;
        unsigned short m_age;
        unsigned int m_scrobbles;
        QDateTime m_registered;
        QString m_country;
        bool m_isSubscriber;
        bool m_canBootstrap;
};



User::User( const XmlQuery& xml ) 
     :AbstractType(),
     d( new UserPrivate )
{
    d->m_name = xml["name"].text();
    d->m_images << xml["image size=small"].text()
             << xml["image size=medium"].text()
             << xml["image size=large"].text();
    d->m_realName = xml["realname"].text();

    QString type = xml["type"].text();

    if ( type == "subscriber" ) d->m_type = TypeSubscriber;
    else if ( type == "moderator" ) d->m_type = TypeModerator;
    else if ( type == "staff" ) d->m_type = TypeStaff;
    else if ( type == "alumni" ) d->m_type = TypeAlumni;
    else d->m_type = TypeUser;

    d->m_age = xml["age"].text().toUInt();
    d->m_scrobbles = xml["playcount"].text().toUInt();
    d->m_registered = QDateTime::fromTime_t(xml["registered"].attribute("unixtime").toUInt());
    d->m_country = xml["country"].text();
    d->m_isSubscriber = ( xml["subscriber"].text() == "1" );
    d->m_canBootstrap = ( xml["bootstrap"].text() == "1" );
    d->m_gender = xml["gender"].text();
    d->m_images << xml["image size=small"].text()
             << xml["image size=medium"].text()
             << xml["image size=large"].text()
             << xml["image size=extralarge"].text();
}

User::User( const User& other )
    :AbstractType(),
    d( new UserPrivate( *other.d ) )
{
}

User::User()
    :AbstractType(),
    d( new UserPrivate )
{
}

User::User( const QString& name )
    :AbstractType(),
    d( new UserPrivate )
{
    d->m_name = name;
    d->m_match = -1.0f;
    d->m_age = 0;
    d->m_scrobbles = 0;
    d->m_registered = QDateTime();
    d->m_isSubscriber = false;
    d->m_canBootstrap = false;
}


User::~User()
{
    delete d;
}


lastfm::User&
User::operator=( const User& that )
{
    d->m_name = that.name();
    d->m_images = that.d->m_images;
    d->m_realName = that.d->m_realName;
    d->m_match = that.d->m_match;
    d->m_type = that.d->m_type;
    d->m_age = that.d->m_age;
    d->m_scrobbles = that.d->m_scrobbles;
    d->m_registered = that.d->m_registered;
    d->m_country = that.d->m_country;
    d->m_isSubscriber = that.d->m_isSubscriber;
    d->m_canBootstrap = that.d->m_canBootstrap;
    d->m_gender = that.d->m_gender;
    d->m_images = that.d->m_images;
    return *this;
}


bool
User::operator==(const lastfm::User& that) const
{
    return d->m_name == that.d->m_name;
}


bool
User::operator<(const lastfm::User& that) const
{
    return d->m_name < that.d->m_name;
}



User::operator QString() const
{
    return d->m_name;
}


QString
User::name() const
{
    return d->m_name;
}


void
User::setName( const QString& name )
{
    d->m_name = name;
}


User::Type
User::type() const
{
    return d->m_type;
}


void
User::setType( Type type )
{
    d->m_type = type;
}


QUrl 
User::imageUrl( ImageSize size, bool square ) const
{
    if( !square ) return d->m_images.value( size );

    QUrl url = d->m_images.value( size );
    QRegExp re( "/serve/(\\d*)s?/" );
    return QUrl( url.toString().replace( re, "/serve/\\1s/" ));
}


QMap<QString, QString>
User::params(const QString& method) const
{
    QMap<QString, QString> map;
    map["method"] = "user."+method;
    map["user"] = d->m_name;
    return map;
}


QNetworkReply*
User::getFriends( bool recentTracks, int limit, int page ) const
{
    QMap<QString, QString> map = params( "getFriends" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    if ( recentTracks ) map["recenttracks"] = "1";
    return ws::get( map );
}


QNetworkReply*
User::getFriendsListeningNow( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getFriendsListeningNow" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getFriendsThatListenTo( const lastfm::Artist& artist, int limit, int page ) const
{
    QMap<QString, QString> map = params( "getFriendsThatListenTo" );
    map["artist"] = artist.name();
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getLovedTracks( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getLovedTracks" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getTopTags() const
{
    return ws::get( params( "getTopTags" ) );
}


QNetworkReply*
User::getTopArtists( QString period, int limit, int page ) const
{
    QMap<QString, QString> map = params( "getTopArtists" );
    map["period"] = period;
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getRecentArtists() const
{
    return ws::get( params( "getRecentArtists" ) );
}


QNetworkReply*
User::getRecentTracks( int limit , int page ) const
{
    QMap<QString, QString> map = params( "getRecentTracks" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    map["extended"] = "true";
    return ws::get( map );
}

QNetworkReply*
User::getRecentStations( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getRecentStations" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getRecommendedArtists( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getRecommendedArtists" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getNeighbours( int limit, int page ) const
{
    QMap<QString, QString> map = params( "getNeighbours" );
    map["limit"] = QString::number( limit );
    map["page"] = QString::number( page );
    return ws::get( map );
}


QNetworkReply*
User::getPlaylists() const
{
    return ws::get( params( "getPlaylists" ) );
}


UserList //static
User::list( QNetworkReply* r )
{

    XmlQuery lfm;

    lfm.parse( r );
    return UserList( lfm );
}


QString
User::toString() const
{
    return name();
}


QDomElement
User::toDomElement( QDomDocument& ) const
{
    return QDomElement();
}


QNetworkReply* //static
User::getInfo( const QString& username )
{
    QMap<QString, QString> map;
    map["method"] = "user.getInfo";
    map["user"] = username;
    return ws::post( map );
}




/*
QNetworkReply* //static
User::getRecommendedArtists()
{
    QMap<QString, QString> map;
    map["method"] = "user.getRecommendedArtists";
    return ws::post( map );
}
*/

QUrl
User::www() const
{ 
    return UrlBuilder( "user" ).slash( d->m_name ).url();
}


float
User::match() const
{
    return d->m_match;
}


QString
User::getInfoString() const
{
    QString text;

    text = QString("%1").arg( d->m_realName.isEmpty() ? d->m_name : d->m_realName );
    if ( d->m_age ) text.append( QString(", %1").arg( d->m_age ) );
    if ( d->m_gender.known() ) text.append( QString(", %1").arg( d->m_gender.toString() ) );
    if ( !d->m_country.isEmpty() ) text.append( QString(", %1").arg( d->m_country ) );

    return text;
}


quint32
User::scrobbleCount() const
{
    return d->m_scrobbles;
}


void 
User::setScrobbleCount( quint32 scrobbleCount )
{
    d->m_scrobbles = scrobbleCount;
}


QDateTime
User::dateRegistered() const
{
    return d->m_registered;
}


void
User::setDateRegistered( const QDateTime& date )
{
    d->m_registered = date;
}


Gender
User::gender() const
{
    return d->m_gender;
}


QString
User::country() const
{
    return d->m_country;
}


QString
User::realName() const
{
    return d->m_realName;
}


void 
User::setRealName( const QString& realName )
{
    d->m_realName = realName;
}


void
User::setImages( const QList<QUrl>& images )
{
    d->m_images = images;
}

unsigned short
User::age() const
{
    return d->m_age;
}

void 
User::setAge( unsigned short age )
{
    d->m_age = age;
}


bool
User::isSubscriber() const
{
    return d->m_isSubscriber;
}


void 
User::setIsSubscriber( bool subscriber )
{
    d->m_isSubscriber = subscriber;
}


bool
User::canBootstrap() const
{
    return d->m_canBootstrap;
}


void 
User::setCanBootstrap( bool canBootstrap )
{
    d->m_canBootstrap = canBootstrap;
}

void 
User::setGender( const QString& s )
{
    d->m_gender = Gender( s );
}

void 
User::setCountry( const QString& country )
{
    d->m_country = country;
}

