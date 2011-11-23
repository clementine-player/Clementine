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

#ifndef ECHONEST_PARSING_P_H
#define ECHONEST_PARSING_P_H

#include "Config.h"

#include <QtXml/QXmlStreamReader>
#include "Song.h"
#include "Artist.h"
#include "Catalog.h"
#include "Playlist.h"

class QNetworkReply;

namespace Echonest
{
namespace Parser
{
    /**
    * Internal helper parsing functions for QXmlStreamParser
    */

    void checkForErrors( QNetworkReply* reply ) throw( ParseError );

    // read the start element and then the status element, throwing
    //  if the result code is not Success
    void readStatus( QXmlStreamReader& xml ) throw( ParseError );

    // parses a <songs></songs> block and turns them into a list of Song object
    QVector< Song > parseSongList( QXmlStreamReader& xml ) throw( ParseError );

    // parses a <song></song> block
    Song parseSong( QXmlStreamReader& xml ) throw( ParseError );

    // parses a <artist_location> block
    ArtistLocation parseSongArtistLocation( QXmlStreamReader& xml ) throw( ParseError );

    // parses a <track></track> block
    Track parseTrack( QXmlStreamReader& xml ) throw( ParseError );

    // parses a <tracks> chunk when asking for a song with tracks bucket
    Tracks parseSongTrackBucket( QXmlStreamReader& xml ) throw( ParseError );

    // parses an <audio_summary> chunk
    AudioSummary parseAudioSummary( QXmlStreamReader& xml ) throw( ParseError );
    // parses the json of the detailed audio summary
    void parseDetailedAudioSummary( QNetworkReply* reply, AudioSummary& summary ) throw( ParseError );

    // parses a list of artists in an <artists></artists> block
    Echonest::Artists parseArtists( QXmlStreamReader& xml ) throw( ParseError );

    // parses the contents of an artist fetch result, expects to be positioned after the readStatus() call
    // it could be a profile query, in which case it has a bunch of different artist attributes
    // or it could be a single fetch, in which case it starts with result number and offset.
    // the results are saved back into the artist
    int parseArtistInfoOrProfile( QXmlStreamReader&, Echonest::Artist& artist ) throw( ParseError );

    // parse the individual artist attributes
    void parseArtistInfo( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );

    // parse each type of artist attribute
    void parseAudio( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );
    void parseBiographies( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );
    void parseImages( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );
    void parseNewsOrBlogs( QXmlStreamReader& xml, Echonest::Artist& artist, bool news = true ) throw( ParseError );
    void parseReviews( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );
    void parseTerms( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );
    void parseUrls( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );
    void parseArtistSong( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );
    void parseVideos( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );
    void parseForeignArtistIds( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );

    // parse a list of terms
    TermList parseTopTermList( QXmlStreamReader& xml ) throw( ParseError );
    QVector< QString > parseTermList( QXmlStreamReader& xml ) throw( ParseError );

    Artists parseArtistSuggestList( QXmlStreamReader& xml ) throw( ParseError );

    License parseLicense( QXmlStreamReader& xml ) throw( ParseError );

    QByteArray parsePlaylistSessionId( QXmlStreamReader& xml ) throw( ParseError );

    // Catalog functions
    Catalogs parseCatalogList( QXmlStreamReader& xml ) throw( ParseError );
    Catalog parseCatalog( QXmlStreamReader& xml, bool justOne = false /* the catalog API is ugly :( */ ) throw( ParseError );
    QList<CatalogItem*> parseCatalogItems( QXmlStreamReader& xml ) throw( ParseError );
    void parseCatalogRequestItem( QXmlStreamReader& xml, Echonest::CatalogArtist&, Echonest::CatalogSong& ) throw( ParseError );
    void saveArtistList( Catalog& catalog, QList<CatalogItem*>& );
    void saveSongList( Catalog& catalog, QList<CatalogItem*>& );
    Echonest::CatalogStatus parseCatalogStatus( QXmlStreamReader& xml ) throw( ParseError );
    Echonest::CatalogStatusItem parseTicketUpdateInfo( QXmlStreamReader& xml ) throw( ParseError );
    QByteArray parseCatalogTicket( QXmlStreamReader& xml ) throw( ParseError );
    Catalog parseNewCatalog( QXmlStreamReader& xml ) throw( ParseError );

    // parses a <tracks> chunk when asking for a song with tracks bucket in a catalog.read call
    Tracks parseCatalogSongTracks( QXmlStreamReader& xml ) throw( ParseError );

    SessionInfo parseSessionInfo( QXmlStreamReader& xml ) throw( ParseError );
    QVector< QString > parseRulesList( QXmlStreamReader& xml ) throw( ParseError );
    QVector< SessionItem > parseSessionSongItem( QXmlStreamReader& xml, const QString& type ) throw( ParseError );

}
}

#endif

