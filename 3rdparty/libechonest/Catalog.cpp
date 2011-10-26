/****************************************************************************************
 * Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>                                    *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include "Catalog.h"

#include "Catalog_p.h"
#include "Parsing_p.h"
#include "Generator_p.h"
#include "Track_p.h"

Echonest::Catalog::Catalog()
    : d( new CatalogData )
{

}

Echonest::Catalog::Catalog( const QByteArray& id )
    : d( new CatalogData )
{
    d->id = id;
}


Echonest::Catalog::Catalog( const Echonest::Catalog& catalog )
    : d( catalog.d )
{
}

Echonest::Catalog& Echonest::Catalog::operator=( const Echonest::Catalog& other )
{
    d = other.d;
    return *this;
}

Echonest::Catalog::~Catalog()
{
}

Echonest::CatalogArtists Echonest::Catalog::artists() const
{
    return d->artists;
}

void Echonest::Catalog::setArtists(const Echonest::CatalogArtists& artists)
{
    d->artists = artists;
}

QByteArray Echonest::Catalog::id() const
{
    return d->id;
}

void Echonest::Catalog::setId(const QByteArray& id)
{
    d->id = id;
}

QString Echonest::Catalog::name() const
{
    return d->name;
}

void Echonest::Catalog::setName(const QString& name)
{
    d->name = name;
}

int Echonest::Catalog::resolved() const
{
    return d->resolved;
}

void Echonest::Catalog::setResolved(int resolved)
{
    d->resolved = resolved;
}

int Echonest::Catalog::pendingTickets() const
{
//     return d->pending_tickets;
    return 0;
}

void Echonest::Catalog::setPendingTickets(int pending)
{
//     d->pending_tickets = pending;
}

Echonest::CatalogSongs Echonest::Catalog::songs() const
{
    return d->songs;
}

void Echonest::Catalog::setSongs(const Echonest::CatalogSongs& songs)
{
    d->songs = songs;
}

int Echonest::Catalog::total() const
{
    return d->total;
}

void Echonest::Catalog::setTotal(int total)
{
    d->total = total;
}

Echonest::CatalogTypes::Type Echonest::Catalog::type() const
{
    return d->type;
}

void Echonest::Catalog::setType(Echonest::CatalogTypes::Type type)
{
    d->type = type;
}

QNetworkReply* Echonest::Catalog::create(const QString& name, Echonest::CatalogTypes::Type type)
{
    QUrl url = Echonest::baseGetQuery( "catalog", "create" );
    url.addQueryItem( QLatin1String( "name" ), name );
    url.addEncodedQueryItem( "type", Echonest::catalogTypeToLiteral( type ) );

    QNetworkRequest request = QNetworkRequest( url );
    request.setHeader( QNetworkRequest::ContentTypeHeader, QLatin1String( "multipart/form-data" ) );
    qDebug() << "Sending create url:" << url.toString();
    return Echonest::Config::instance()->nam()->post( request, QByteArray() );
}

QNetworkReply* Echonest::Catalog::deleteCatalog() const
{
    QUrl url = Echonest::baseGetQuery( "catalog", "delete" );
    Q_ASSERT( !d->id.isEmpty() );
    url.addEncodedQueryItem( "id", d->id );
    
    return Echonest::doPost( url );
}

QNetworkReply* Echonest::Catalog::list(int results, int start)
{
    QUrl url = Echonest::baseGetQuery( "catalog", "list" );
    addLimits( url, results, start );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );    
}

QNetworkReply* Echonest::Catalog::profile() const
{
    QUrl url = Echonest::baseGetQuery( "catalog", "profile" );
    if( !d->id.isEmpty() )
        url.addEncodedQueryItem( "id", d->id );
    else if( !d->name.isEmpty() )
        url.addQueryItem( QLatin1String( "name" ), d->name );
    else
        Q_ASSERT_X( false, "Catalog", "Not enough information!" );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );    
}

QNetworkReply* Echonest::Catalog::status(const QByteArray& ticket)
{
    QUrl url = Echonest::baseGetQuery( "catalog", "status" );
    url.addEncodedQueryItem( "ticket", ticket );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) );    
}

QNetworkReply* Echonest::Catalog::update(const Echonest::CatalogUpdateEntries& entries) const
{
    QUrl url = Echonest::baseGetQuery( "catalog", "update" );
    Q_ASSERT( !d->id.isEmpty() );
    url.addEncodedQueryItem( "id", d->id );
    return Echonest::Catalog::updatePrivate( url, entries );
}

QNetworkReply* Echonest::Catalog::updateAndCreate(const Echonest::CatalogUpdateEntries& entries)
{
    QUrl url = Echonest::baseGetQuery( "catalog", "update" );
    return Echonest::Catalog::updatePrivate( url, entries );
}

QNetworkReply* Echonest::Catalog::readArtistCatalog(Echonest::ArtistInformation info, int results, int start) const
{
    QUrl url = Echonest::baseGetQuery( "catalog", "read" );
    Artist::addQueryInformation( url, info );
    
    return readPrivate( url, results, start ); 
}

QNetworkReply* Echonest::Catalog::readSongCatalog(Echonest::SongInformation info, int results, int start) const
{
    QUrl url = Echonest::baseGetQuery( "catalog", "read" );
    Song::addQueryInformation( url, info );
    
    return readPrivate( url, results, start ); 
}

QPair< QString, QByteArray > Echonest::Catalog::parseDelete( QNetworkReply* reply ) throw( Echonest::ParseError )
{
    QByteArray data = reply->readAll();
//     qDebug() << "DATA:" << data;
    QPair< QString, QByteArray > asd;
    Echonest::Parser::checkForErrors( reply );
    
    QXmlStreamReader xml( data );
    
    Echonest::Parser::readStatus( xml );
    
    // TODO, after create works :)
    reply->deleteLater();
    return asd;
}

Echonest::Catalogs Echonest::Catalog::parseList(QNetworkReply* reply) throw( Echonest::ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    QXmlStreamReader xml( reply->readAll() );
    Echonest::Parser::readStatus( xml );
    
    Echonest::Catalogs catalogs = Echonest::Parser::parseCatalogList( xml );
    
    reply->deleteLater();
    return catalogs;
}

void Echonest::Catalog::parseProfile(QNetworkReply* reply) throw( Echonest::ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    QXmlStreamReader xml( reply->readAll() );
    Echonest::Parser::readStatus( xml );
    
    Echonest::Catalog catalog = Echonest::Parser::parseCatalog( xml, true );
    d = catalog.d;
    reply->deleteLater();
}

void Echonest::Catalog::parseRead(QNetworkReply* reply) throw( Echonest::ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    QXmlStreamReader xml( reply->readAll() );
    Echonest::Parser::readStatus( xml );
    
    Echonest::Catalog catalog = Echonest::Parser::parseCatalog( xml, true );
    d = catalog.d;
    reply->deleteLater();
}

Echonest::CatalogStatus Echonest::Catalog::parseStatus(QNetworkReply* reply) throw( Echonest::ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    QByteArray data = reply->readAll();
//     qDebug() << data;
    QXmlStreamReader xml( data );
    Echonest::Parser::readStatus( xml );
    
    Echonest::CatalogStatus status = Echonest::Parser::parseCatalogStatus( xml );
    reply->deleteLater();
    return status;
}

QByteArray Echonest::Catalog::parseTicket(QNetworkReply* reply) throw( Echonest::ParseError )
{
    Echonest::Parser::checkForErrors( reply );
    QByteArray data = reply->readAll();
//    qDebug() << data;
    QXmlStreamReader xml( data );
    Echonest::Parser::readStatus( xml );
    
    QByteArray ticket = Echonest::Parser::parseCatalogTicket( xml );
    reply->deleteLater();
    return ticket;
}

Echonest::Catalog Echonest::Catalog::parseCreate(QNetworkReply* reply) throw( Echonest::ParseError )
{
    QByteArray data = reply->readAll();
//    qDebug() << data;
    Echonest::Parser::checkForErrors( reply );
    QXmlStreamReader xml( data  );
    Echonest::Parser::readStatus( xml );
    
    Catalog c = Echonest::Parser::parseNewCatalog( xml );
    reply->deleteLater();
    return c;
}


QNetworkReply* Echonest::Catalog::updatePrivate( QUrl& url, const Echonest::CatalogUpdateEntries& entries)
{
    url.addEncodedQueryItem( "data_type", "json" );
    
    QByteArray payload = Generator::catalogEntriesToJson( entries );
    url.addEncodedQueryItem( "data", payload );

    return Echonest::doPost( url );
}

void Echonest::Catalog::addLimits(QUrl& url, int results, int start)
{
    if( results != 30 )
        url.addEncodedQueryItem( "results", QString::number( results ).toLatin1() );
    if( start > -1 )
        url.addEncodedQueryItem( "start", QString::number( start ).toLatin1() );
}

QNetworkReply* Echonest::Catalog::readPrivate(QUrl& url, int results, int start) const
{
    Q_ASSERT( !d->id.isEmpty() );
    url.addEncodedQueryItem( "id", d->id );
    addLimits( url, results, start );
    
    return Echonest::Config::instance()->nam()->get( QNetworkRequest( url ) ); 
}

QDebug Echonest::operator<<(QDebug d, const Echonest::Catalog& catalog)
{
    return d.maybeSpace() << QString::fromLatin1( "Catalog(%1, %2, %3, %4)" ).arg( catalog.name() ).arg( QLatin1String( catalog.id() ) )
                                                                                     .arg( QString::fromLatin1( Echonest::catalogTypeToLiteral( catalog.type() ) ) ).arg( catalog.total() ) << catalog.artists() << catalog.songs();
}
