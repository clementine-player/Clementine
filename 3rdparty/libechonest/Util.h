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

#ifndef ECHONEST_UTIL_H
#define ECHONEST_UTIL_H

#include "echonest_export.h"

#include <QDebug>
#include <QVector>
#include <QUrl>

/**
 * Some shared declarations
 */

namespace Echonest
{
    namespace Analysis
    {
        enum AnalysisStatus {
            Unknown = 0,
            Pending = 1,
            Complete = 2,
            Error = 4
        };
    }

    namespace CatalogTypes
    {
        enum Type {
            Artist = 0,
            Song = 1
        };

        enum Action {
            Delete,
            Update,
            Play,
            Skip
        };

        enum TicketStatus {
            Unknown = 0,
            Pending = 1,
            Complete = 2,
            Error = 4
        };
    }

    typedef struct
    {
        qreal confidence;
        qreal duration;
        qreal start;
    } AudioChunk;

    typedef AudioChunk Bar;
    typedef AudioChunk Beat;
    typedef AudioChunk Section;
    typedef AudioChunk Tatum;

    typedef struct
    {
        qreal confidence;
        qreal duration;
        qreal loudness_max;
        qreal loudness_max_time;
        qreal loudness_start;
        QVector< qreal > pitches;
        qreal start;
        QVector< qreal > timbre;
    } Segment;


    typedef QVector< Bar > BarList;
    typedef QVector< Beat > BeatList;
    typedef QVector< Section > SectionList;
    typedef QVector< Tatum > TatumList;
    typedef QVector< Segment > SegmentList;

    typedef struct {
        QUrl url;
        QString attribution;
        QString type;
    } License;

    typedef struct {
        QString catalog;
        QString foreign_id;
    } ForeignId;

    typedef struct {
        qreal latitude;
        qreal longitude;
        QString location;
    } ArtistLocation;

    typedef QVector< ForeignId > ForeignIds;

    QByteArray escapeSpacesAndPluses( const QString& in );

    Analysis::AnalysisStatus statusToEnum( const QString& status );
    QString statusToString( Analysis::AnalysisStatus status );

    QByteArray catalogTypeToLiteral( CatalogTypes::Type );
    CatalogTypes::Type literalToCatalogType( const QByteArray& type );

    QByteArray catalogStatusToLiteral( CatalogTypes::TicketStatus );
    CatalogTypes::TicketStatus literalToCatalogStatus( const QByteArray& type );

    QByteArray catalogUpdateActionToLiteral( CatalogTypes::Action );
    CatalogTypes::Action literalToCatalogUpdateAction( const QByteArray& type );

    ECHONEST_EXPORT QDebug operator<<(QDebug d, const ForeignId& id);
    ECHONEST_EXPORT QDebug operator<<(QDebug d, const ArtistLocation& id);
}

#endif
