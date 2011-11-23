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

#include "TypeInformation.h"

#include <QStringList>

namespace Echonest {
    
class ArtistInformationPrivate 
{
public:
        
    ArtistInformationPrivate() : flags( ArtistInformation::NoInformation ) 
    {}
    
    Echonest::ArtistInformation::ArtistInformationFlags flags;
    QStringList idSpaces;
};

class SongInformationPrivate 
{
public:
    
    SongInformationPrivate() : flags( SongInformation::NoInformation ) 
    {}
    
    SongInformation::SongInformationFlags flags;
    QStringList idSpaces;
};

} // namespace 
Echonest::ArtistInformation::ArtistInformation() 
    : d_ptr( new Echonest::ArtistInformationPrivate )
{
    Q_D( ArtistInformation );
    d->flags = Echonest::ArtistInformation::NoInformation;
}


Echonest::ArtistInformation::ArtistInformation( ArtistInformationFlags flags )
: d_ptr( new Echonest::ArtistInformationPrivate )
{
    Q_D( ArtistInformation );
    d->flags = flags;
}

Echonest::ArtistInformation::ArtistInformation( ArtistInformation::ArtistInformationFlags flags, const QStringList& idSpaces )
    : d_ptr( new Echonest::ArtistInformationPrivate )
{
    Q_D( Echonest::ArtistInformation );
    d->flags = flags;
    d->idSpaces = idSpaces;
}

Echonest::ArtistInformation::ArtistInformation( const Echonest::ArtistInformation& other )
    : d_ptr( new Echonest::ArtistInformationPrivate( *other.d_ptr ) )
{
}

Echonest::ArtistInformation::~ArtistInformation()
{
    delete d_ptr;
}
Echonest::ArtistInformation& Echonest::ArtistInformation::operator=( const Echonest::ArtistInformation& typeInfo )
{
    d_ptr = new Echonest::ArtistInformationPrivate( *typeInfo.d_ptr );
    return *this;
}

Echonest::ArtistInformation::ArtistInformationFlags Echonest::ArtistInformation::flags() const
{
    Q_D( const Echonest::ArtistInformation );
    
    return d->flags;
}

void Echonest::ArtistInformation::setArtistInformationFlags( ArtistInformationFlags flags)
{
    Q_D( Echonest::ArtistInformation );
    
    d->flags = flags;
}

QStringList Echonest::ArtistInformation::idSpaces() const
{
    Q_D( const Echonest::ArtistInformation );
    
    return d->idSpaces;
}

void Echonest::ArtistInformation::setIdSpaces(const QStringList& idspaces)
{
    Q_D( Echonest::ArtistInformation );
    
    d->idSpaces = idspaces;
}

Echonest::SongInformation::SongInformation()
    : d_ptr( new Echonest::SongInformationPrivate )
{
    Q_D( Echonest::SongInformation );
    
    d->flags = Echonest::SongInformation::NoInformation;
}

Echonest::SongInformation::SongInformation( SongInformationFlags flags )
    : d_ptr( new Echonest::SongInformationPrivate )
{
    Q_D( SongInformation );
    
    d->flags = flags;
}

Echonest::SongInformation::SongInformation( SongInformation::SongInformationFlags flags, const QStringList& idSpaces )
    : d_ptr( new Echonest::SongInformationPrivate )
{
    Q_D( Echonest::SongInformation );
    
    d->flags = flags;
    d->idSpaces = idSpaces;
}

Echonest::SongInformation::SongInformation( const Echonest::SongInformation& other )
    : d_ptr( new Echonest::SongInformationPrivate( *other.d_ptr ) )
{

}

Echonest::SongInformation::~SongInformation()
{
    delete d_ptr;
}

Echonest::SongInformation& Echonest::SongInformation::operator=( const Echonest::SongInformation& info )
{
    d_ptr = new Echonest::SongInformationPrivate( *info.d_ptr );
    
    return *this;
}


Echonest::SongInformation::SongInformationFlags Echonest::SongInformation::flags() const
{
    Q_D( const Echonest::SongInformation );
    
    return d->flags;
}

void Echonest::SongInformation::setSongInformationFlags( SongInformationFlags flags )
{
    Q_D( Echonest::SongInformation );
    
    d->flags = flags;
}

QStringList Echonest::SongInformation::idSpaces() const
{
    Q_D( const Echonest::SongInformation );
    
    return d->idSpaces;
}

void Echonest::SongInformation::setIdSpaces(const QStringList& idspaces)
{
    Q_D( Echonest::SongInformation );
    
    d->idSpaces = idspaces;
}

