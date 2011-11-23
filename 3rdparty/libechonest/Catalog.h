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

#ifndef ECHONEST_CATALOG_H
#define ECHONEST_CATALOG_H

#include "Artist.h"
#include "CatalogUpdateEntry.h"
#include "echonest_export.h"
#include "Song.h"
#include "TypeInformation.h"
#include "Util.h"

#include <QSharedDataPointer>
#include <QString>
#include "CatalogSong.h"
#include "CatalogArtist.h"

class QNetworkReply;
class CatalogData;

namespace Echonest
{

    
class Catalog;
typedef QVector< Catalog > Catalogs;

class ECHONEST_EXPORT Catalog
{
public:    
    Catalog();
    explicit Catalog( const QByteArray& id );
    Catalog( const Catalog& );
    virtual ~Catalog();
    Catalog& operator=( const Catalog& );
    
     /// Basic information about the catalog

     /**
      * The name of this catalog.
      */
     QString name() const;
     void setName( const QString& name );
     
     /**
      * The id of this catalog.
      */
     QByteArray id() const;
     void setId( const QByteArray& id );
     
     /**
      * The type of this catalog.
      */
     CatalogTypes::Type type() const;
     void setType( CatalogTypes::Type type );
    
     /**
      * The total number of items in this catalog.
      */
     int total() const;
     void setTotal( int total );
     
     /// The following fields only have data if the appropriate parse* methods have been called
     
     /**
      * The number of resolved items in the catalog.
      */
     int resolved() const;
     void setResolved( int resolved );
     
     /**
      * The number of pending tickets still to be resolved
      */
     int pendingTickets() const;
     void setPendingTickets( int pending );
     
     /**
      * The songs in this catalog, if this is a song catalog.
      */
     CatalogSongs songs() const;
     void setSongs( const CatalogSongs& songs );
     
     /**
      * The artists in this catalog, if it is an artist catalog.
      */
     CatalogArtists artists() const;
     void setArtists( const CatalogArtists& artists );
    
    /**
     * Update this catalog with the given items. Each item has an associated action, default is Update.
     *  Call parseTicket() to access the result ticket from this call.
     * 
     * See more information about this api call at http://developer.echonest.com/docs/v4/catalog.html#update
     * 
     * Requires catalog id.
     * 
     * \param entries The list of entries to update the catalog with.
    */
    QNetworkReply* update( const CatalogUpdateEntries& entries ) const;
    
    /**
     * Get basic information on a catalog. Only requires one of catalog id or name.
     */
    QNetworkReply* profile() const;
    
    /**
     * Fetch the full list of data from this catalog. It is possible to specify specific audio
     *  information that you wish to have included with each item. Use the appropriate artist- or 
     *  song-specific method calls in order to achieve this.
     * 
     * \param info The list of desired information to be included with each item.
     * \param results How many results to return in total
     * \param start The index of the first result
     */
    QNetworkReply* readArtistCatalog( ArtistInformation info = ArtistInformation(), int results = 30, int start = -1 ) const;
    QNetworkReply* readSongCatalog( SongInformation info = SongInformation(), int results = 30, int start = -1 ) const;
    
    /**
     * Deletes this catalog from The Echo Nest. Only the API key used to create a catalog can delete it.
     */
    QNetworkReply* deleteCatalog() const;
        
    /**
     * Create a new catalog with the given name and type. 
     * 
     * Parse the finished QNetworkReply with parseCreate()
     * 
     *  \param name The name of the catalog to create
     *  \param type The type of the catalog to create
     */
    static QNetworkReply* create( const QString& name, CatalogTypes::Type type );
    
    /**
     * Returns a list of catalogs created with this key.
     */
    static QNetworkReply* list( int results = 30, int start = -1 );
    
    /**
     * Creates a new catalog with the given items.
     * See more information about this api call at http://developer.echonest.com/docs/v4/catalog.html#update
     * 
     * Call parseTicket() to access the result ticket from this call.
     * 
     * \param entries The list of entries to populate the catalog with.
     * 
     */
    static QNetworkReply* updateAndCreate( const CatalogUpdateEntries& entries );
    
    /**
     * Checks the status of a catalog operation given a catalog ticket.
     * 
     * Parse the result with parseStatus()
     * 
     * \param ticket The catalog ticket returned from an update() or updateAndCreate() call
     */
    static QNetworkReply* status( const QByteArray& ticket );
    
    /**
     * Parses the result of a status call, returning the status information along with information on
     *  item resolution if available.
     */
    static CatalogStatus parseStatus( QNetworkReply* ) throw( Echonest::ParseError );
    
    /**
     * Parses the result of a profile() call. Saves the data to this catalog object.
     */
    void parseProfile( QNetworkReply* ) throw( Echonest::ParseError );
    
    /**
     * Parses the result of the read*Catalog() calls. Saves the catalog data to this object.
     */
    void parseRead( QNetworkReply * ) throw( Echonest::ParseError );
    
    /**
     * Parse the result of a delete call. Will throw if the catalog was not successfully deleted,
     *  and returns the name/id pair.
     * 
     * \return QPair of catalogName, catalogId that was just deleted.
     */
    QPair< QString, QByteArray > parseDelete( QNetworkReply* ) throw( Echonest::ParseError );
    
    /**
     * Parse the result of the list() API call. Will return a list of catalogs---each catalog only
     *  has id, name, type, and total tracks information.
     */
    static Catalogs parseList( QNetworkReply* ) throw( Echonest::ParseError );
    
    /**
     * Parse the result of a catalog call. The calls return a ticket that can be used to check the status
     *  of the call with status()
     */
    static QByteArray parseTicket( QNetworkReply* ) throw( Echonest::ParseError );
    
    /**
     * Parse the result of a create() call.
     */
    static Catalog parseCreate( QNetworkReply* reply ) throw( Echonest::ParseError );
private:
    static QNetworkReply* updatePrivate( QUrl&, const CatalogUpdateEntries& entries );
    QNetworkReply* readPrivate( QUrl& url, int results, int start ) const;
    static void addLimits( QUrl&, int results, int start );
    QSharedDataPointer< CatalogData > d;
};

ECHONEST_EXPORT QDebug operator<<(QDebug d, const Catalog &catalog);

}

Q_DECLARE_METATYPE( Echonest::Catalog )

#endif
