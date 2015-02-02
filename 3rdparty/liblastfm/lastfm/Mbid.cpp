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
#include "Mbid.h"
#include "mbid_mp3.c"

#include <QFile>

namespace lastfm
{

class MbidPrivate
{
public:
    QString id;
};


Mbid::Mbid( const QString& p )
    : d( new MbidPrivate )
{
    d->id = p;
}


Mbid::Mbid( const Mbid& that )
    : d( new MbidPrivate( *that.d ) )
{
}


Mbid::~Mbid()
{
    delete d;
}


bool
Mbid::isNull() const
{
    return d->id.isNull() || d->id.isEmpty();
}


Mbid::operator QString() const
{
    return d->id;
}


Mbid&
Mbid::operator=( const Mbid& that )
{
    d->id = that.d->id;
    return *this;
}


Mbid //static
Mbid::fromLocalFile( const QString& path )
{
    char out[MBID_BUFFER_SIZE];
    QByteArray const bytes = QFile::encodeName( path );
    int const r = getMP3_MBID( bytes.data(), out );
    Mbid mbid;
    if (r == 0) mbid.d->id = QString::fromLatin1( out );
    return mbid;
}

}
