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

#include "Util.h"

#include <QString>

QByteArray Echonest::escapeSpacesAndPluses(const QString& in)
{
    // Echonest wants " " treated as "+", so we force QUrl to encode spaces as +es rather than %20
    QByteArray escaped = QUrl::toPercentEncoding( in, " " );
    escaped.replace( " ", "+" );
    return escaped;
}


Echonest::Analysis::AnalysisStatus Echonest::statusToEnum(const QString& status)
{
    if( status == QLatin1String("unknown") ) {
        return Echonest::Analysis::Unknown;
    } else if( status == QLatin1String("pending") ) {
        return Echonest::Analysis::Pending;
    } else if( status == QLatin1String("complete") ) {
        return Echonest::Analysis::Complete;
    } else if( status == QLatin1String("error" )) {
        return Echonest::Analysis::Error;
    }
    return Echonest::Analysis::Unknown;
}

QString Echonest::statusToString(Echonest::Analysis::AnalysisStatus status)
{
    switch( status )
    {
        case Echonest::Analysis::Unknown:
            return QLatin1String( "unknown" );
        case Echonest::Analysis::Pending:
            return QLatin1String( "pending" );
        case Echonest::Analysis::Complete:
            return QLatin1String( "complete" );
        case Echonest::Analysis::Error:
            return QLatin1String( "error" );
    }
    return QString();
}

QByteArray Echonest::catalogTypeToLiteral(Echonest::CatalogTypes::Type type)
{
    switch( type )
    {
        case Echonest::CatalogTypes::Artist:
            return "artist";
        case Echonest::CatalogTypes::Song:
            return "song";
        default:
            return "";
    }
}

Echonest::CatalogTypes::Type Echonest::literalToCatalogType( const QByteArray& type )
{
    if( type == "artist" )
        return Echonest::CatalogTypes::Artist;
    else if( type == "song" )
        return Echonest::CatalogTypes::Song;
    else
        return Echonest::CatalogTypes::Artist;
}


QByteArray Echonest::catalogStatusToLiteral(Echonest::CatalogTypes::TicketStatus status)
{
    return statusToString( static_cast<Echonest::Analysis::AnalysisStatus>( status ) ).toLatin1();
}

Echonest::CatalogTypes::TicketStatus Echonest::literalToCatalogStatus(const QByteArray& type)
{
    return static_cast<Echonest::CatalogTypes::TicketStatus>( statusToEnum( QLatin1String( type ) ) );
}

/**
 *             Delete,
 U pda*te,
 Play,
 Skip */

QByteArray Echonest::catalogUpdateActionToLiteral(Echonest::CatalogTypes::Action action)
{
    switch( action )
    {
        case Echonest::CatalogTypes::Delete:
            return "delete";
        case Echonest::CatalogTypes::Play:
            return "play";
        case Echonest::CatalogTypes::Update:
            return "update";
        case Echonest::CatalogTypes::Skip:
            return "skip";
        default:
            return "";
    }
}

Echonest::CatalogTypes::Action Echonest::literalToCatalogUpdateAction(const QByteArray& type)
{
    if( type == "delete" )
        return Echonest::CatalogTypes::Delete;
    else if( type == "play" )
        return Echonest::CatalogTypes::Play;
    else if( type == "update" )
        return Echonest::CatalogTypes::Update;
    else if( type == "skip" )
        return Echonest::CatalogTypes::Skip;

    return Echonest::CatalogTypes::Update;
}

QDebug Echonest::operator<<(QDebug d, const Echonest::ForeignId& id)
{
    return d.maybeSpace() << QString::fromLatin1( "Foreign Id(%1, %2)" ).arg( id.catalog ).arg( id.foreign_id );
}

QDebug Echonest::operator<<(QDebug d, const Echonest::ArtistLocation& loc)
{
    return d.maybeSpace() << QString::fromLatin1( "Artist Location(%1, %2, %3)" ).arg( loc.location ).arg( loc.latitude ).arg( loc.longitude );
}
