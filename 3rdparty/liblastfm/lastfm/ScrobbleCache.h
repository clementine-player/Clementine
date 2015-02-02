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
#ifndef LASTFM_SCROBBLE_CACHE_H
#define LASTFM_SCROBBLE_CACHE_H

#include "Track.h"
#include <QList>

namespace lastfm {

/** absolutely not thread-safe */
class LASTFM_DLLEXPORT ScrobbleCache
{
public:
    enum Invalidity
    {
        TooShort,
        ArtistNameMissing,
        TrackNameMissing,
        ArtistInvalid,
        NoTimestamp,
        FromTheFuture,
        FromTheDistantPast
    };

    explicit ScrobbleCache( const QString& username );
    ScrobbleCache( const ScrobbleCache& that );
    ~ScrobbleCache();

    /** note this is unique for Track::sameAs() and equal timestamps 
      * obviously playcounts will not be increased for the same timestamp */
    void add( const QList<Track>& );

    /** returns the number of tracks left in the queue */
    int remove( const QList<Track>& );

    static bool isValid( const lastfm::Track& track, Invalidity* v = 0 );

    ScrobbleCache& operator=( const ScrobbleCache& that );

    QList<Track> tracks() const;
    QString path() const;
    QString username() const;

private:
    bool operator==( const ScrobbleCache& ); //undefined
    class ScrobbleCachePrivate * const d;
};

}

#endif
