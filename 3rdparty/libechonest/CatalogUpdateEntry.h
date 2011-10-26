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
#ifndef ECHONEST_CATALOG_ENTRY_H
#define ECHONEST_CATALOG_ENTRY_H

#include "echonest_export.h"
#include "Util.h"

#include <QByteArray>
#include <QSharedDataPointer>
#include <QString>
#include <QVector>

class CatalogUpdateEntryData;

namespace Echonest {

/**
 * This rather simple struct collects information about a status update
 */
typedef QVector< QPair< QByteArray, QString > > CatalogStatusItem;
typedef struct CatalogStatusStruct {
    CatalogTypes::TicketStatus status;
    QString details;

    int items_updated;
    CatalogStatusItem items; // List of [ item_id, info ]

//     int percent_complete;
    CatalogStatusStruct() : status( CatalogTypes::Unknown ), items_updated( -1 ) {}
} CatalogStatus;

/**
 * This class described a catalog entry for use in the Catalog update() call.
 *  All data fields are optional except Action, and only the ones specified will be sent.
 */
class ECHONEST_EXPORT CatalogUpdateEntry
{
public:
    CatalogUpdateEntry();
    CatalogUpdateEntry( CatalogTypes::Action action );
    virtual ~CatalogUpdateEntry();
    CatalogUpdateEntry( const CatalogUpdateEntry& other );
    CatalogUpdateEntry& operator=( const CatalogUpdateEntry& );

    /**
     * Optional, the item id for the catalog entry. hash( catalog_id + item_id )
     *  MUST be unique. If this is not set, a unique id will be generated internally.
     */
    QByteArray itemId() const;
    void setItemId( const QByteArray& id );

    /**
     * The type of action that this item represents, required.
     */
    CatalogTypes::Action action() const;
    void setAction( CatalogTypes::Action action );

    /**
     * The Echo Nest fingerprint.
     */
    QByteArray fingerprint() const;
    void setFingerprint( const QByteArray& id );

    // deprecated
    void setFingerpring( const QByteArray& id );
    /**
     * The song id. Rosetta id or Echo Nest ID.
     */
    QByteArray songId() const;
    void setSongId( const QByteArray& id );

    /**
     * The song name. Mutually exclusive with song id.
     */
    QString songName() const;
    void setSongName( const QString& name );

    /**
     * The artist id, either a rosetta stone ID or an Echo Nest ID.
     */
    QByteArray artistId() const;
    void setArtistId( const QByteArray& id );

    /**
     * The artist name, mutually exclusive with artist id.
     */
    QString artistName() const;
    void setArtistName( const QString& name );

    /**
     * The release, or album, name.
     */
    QString release() const;
    void setRelease( const QString& release );

    /**
     * The genre of the item.
     */
    QString genre() const;
    void setGenre( const QString& genre );

    /**
     * The track number.
     */
    int trackNumber() const;
    void setTrackNumber( int trackNum );

    /**
     * The disc number of this item.
     */
    int discNumber() const;
    void setDiscNumber( int disc );

    /**
     * The url or the local filename or remote url.
     */
    QString url() const;
    void setUrl( const QString& url );

    /**
     * If this song was marked as a favorite or not
     */
    bool favorite() const;
    void setFavorite( bool fav );

    /**
     * If this song was banned.
     */
    bool banned() const;
    void setBanned( bool banned );

    /**
     * The play count of this item.
     */
    int playCount() const;
    void setPlayCount( int playCount );

    /**
     * The skip count of this item.
     */
    int skipCount() const;
    void setSkipCount( int skipCount );

    /**
     * The rating of this item, from 1 to 10.
     */
    int rating() const;
    void setRating( int rating );

    bool favoriteSet() const;
    bool bannedSet() const;
private:
    QSharedDataPointer<CatalogUpdateEntryData> d;
};

typedef QVector<CatalogUpdateEntry> CatalogUpdateEntries;

}

#endif
