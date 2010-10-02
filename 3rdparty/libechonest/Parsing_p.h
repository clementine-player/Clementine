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

#include <QXmlStreamReader>
#include "Song.h"
#include "Artist.h"

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
    
    // parses a <track></track> block
    Track parseTrack( QXmlStreamReader& xml ) throw( ParseError );
    
    // parses an <audio_summary> chunk
    AudioSummary parseAudioSummary( QXmlStreamReader& xml ) throw( ParseError );
    
    // parses a list of artists in an <artists></artists> block
    Echonest::Artists parseArtists( QXmlStreamReader& xml );
    
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
    void parseForeignIds( QXmlStreamReader& xml, Echonest::Artist& artist ) throw( ParseError );
    
    // parse a list of terms
    TermList parseTermList( QXmlStreamReader& xml );
    
    License parseLicense( QXmlStreamReader& xml ) throw( ParseError );
    
    QByteArray parsePlaylistSessionId( QXmlStreamReader& xml ) throw( ParseError );
}
}

#endif