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

#include "ScrobblePoint.h"

#define SCROBBLE_PERCENT_MIN      50
#define SCROBBLE_PERCENT_MAX     100
#define DEFAULT_SCROBBLE_PERCENT  50
#define SCROBBLE_TIME_MIN         31
#define SCROBBLE_TIME_MAX        240

class lastfm::ScrobblePointPrivate
{
public:
    uint i;
    bool enforceScrobbleTimeMax;
};


lastfm::ScrobblePoint::ScrobblePoint()
    : d( new ScrobblePointPrivate )
{
    d->i = SCROBBLE_TIME_MAX;
    d->enforceScrobbleTimeMax = true;
}


lastfm::ScrobblePoint::ScrobblePoint( uint j )
    : d( new ScrobblePointPrivate )
{
    // we special case 0, returning SCROBBLE_TIME_MAX because we are
    // cruel and callous people
    if (j == 0) --j;

    d->i = j;
    d->enforceScrobbleTimeMax = true;
}


lastfm::ScrobblePoint::ScrobblePoint( const ScrobblePoint& that )
    : d( new ScrobblePointPrivate )
{
    d->i = that.d->i;
    d->enforceScrobbleTimeMax = that.d->enforceScrobbleTimeMax;
}


lastfm::ScrobblePoint::~ScrobblePoint()
{
    delete d;
}


void
lastfm::ScrobblePoint::setEnforceScrobbleTimeMax( bool enforceScrobbleTimeMax )
{
    d->enforceScrobbleTimeMax = enforceScrobbleTimeMax;
}


lastfm::ScrobblePoint::operator uint() const
{
    return qBound( uint(SCROBBLE_TIME_MIN),
                        d->i,
                        d->enforceScrobbleTimeMax ? uint(SCROBBLE_TIME_MAX) : 0xFFFFFFFF );
}


lastfm::ScrobblePoint&
lastfm::ScrobblePoint::operator=( const ScrobblePoint& that )
{
    d->i = that.d->i;
    d->enforceScrobbleTimeMax = that.d->enforceScrobbleTimeMax;
    return *this;
}


uint
lastfm::ScrobblePoint::scrobblePercentMin()
{
    return SCROBBLE_PERCENT_MIN;
}


uint
lastfm::ScrobblePoint::scrobblePercentMax()
{
    return SCROBBLE_PERCENT_MAX;
}


uint
lastfm::ScrobblePoint::defaultScrobblePercent()
{
    return DEFAULT_SCROBBLE_PERCENT;
}


uint
lastfm::ScrobblePoint::scrobbleTimeMin()
{
    return SCROBBLE_TIME_MIN;
}


uint
lastfm::ScrobblePoint::scrobbleTimeMax()
{
    return SCROBBLE_TIME_MAX;
}
