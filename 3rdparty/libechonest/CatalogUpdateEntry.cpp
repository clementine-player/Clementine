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
#include "CatalogUpdateEntry.h"
#include "CatalogUpdateEntry_p.h"

Echonest::CatalogUpdateEntry::CatalogUpdateEntry()
    : d( new CatalogUpdateEntryData )
{

}

Echonest::CatalogUpdateEntry::CatalogUpdateEntry( CatalogTypes::Action action )
    : d( new CatalogUpdateEntryData )
{
    d->action = action;
}

Echonest::CatalogUpdateEntry::CatalogUpdateEntry( const Echonest::CatalogUpdateEntry& other )
    : d( other.d )
{

}

Echonest::CatalogUpdateEntry::~CatalogUpdateEntry()
{
}

Echonest::CatalogUpdateEntry& Echonest::CatalogUpdateEntry::operator=(const Echonest::CatalogUpdateEntry& other)
{
    d = other.d;
    return *this;
}

QByteArray Echonest::CatalogUpdateEntry::itemId() const
{
    return d->item_id;
}

void Echonest::CatalogUpdateEntry::setItemId(const QByteArray& id)
{
    d->item_id = id;
}

Echonest::CatalogTypes::Action Echonest::CatalogUpdateEntry::action() const
{
    return d->action;
}

void Echonest::CatalogUpdateEntry::setAction(Echonest::CatalogTypes::Action action)
{
    d->action = action;
}

QByteArray Echonest::CatalogUpdateEntry::artistId() const
{
    return d->artist_id;
}

void Echonest::CatalogUpdateEntry::setArtistId(const QByteArray& id)
{
    d->artist_id = id;
}

QByteArray Echonest::CatalogUpdateEntry::fingerprint() const
{
    return d->fp_code;
}

void Echonest::CatalogUpdateEntry::setFingerprint(const QByteArray& id)
{
    d->fp_code = id;
}

void Echonest::CatalogUpdateEntry::setFingerpring(const QByteArray& id)
{
   setFingerprint(id);
}

QString Echonest::CatalogUpdateEntry::artistName() const
{
    return d->artist_name;
}

void Echonest::CatalogUpdateEntry::setArtistName(const QString& name)
{
    d->artist_name = name;
}

bool Echonest::CatalogUpdateEntry::banned() const
{
    return d->banned;
}

void Echonest::CatalogUpdateEntry::setBanned(bool banned)
{
    d->bannedSet = true;
    d->banned = banned;
}

int Echonest::CatalogUpdateEntry::discNumber() const
{
    return d->disc_number;
}

void Echonest::CatalogUpdateEntry::setDiscNumber(int disc)
{
    d->disc_number = disc;
}

bool Echonest::CatalogUpdateEntry::favorite() const
{
    return d->favorite;
}

void Echonest::CatalogUpdateEntry::setFavorite(bool fav)
{
    d->favoriteSet = true;
    d->favorite = fav;
}

QString Echonest::CatalogUpdateEntry::genre() const
{
    return d->genre;
}

void Echonest::CatalogUpdateEntry::setGenre(const QString& genre)
{
    d->genre = genre;
}

int Echonest::CatalogUpdateEntry::playCount() const
{
    return d->play_count;
}

void Echonest::CatalogUpdateEntry::setPlayCount(int playCount)
{
    d->play_count = playCount;
}

int Echonest::CatalogUpdateEntry::rating() const
{
    return d->rating;
}

void Echonest::CatalogUpdateEntry::setRating(int rating)
{
    d->rating = rating;
}

QString Echonest::CatalogUpdateEntry::release() const
{
    return d->release;
}

void Echonest::CatalogUpdateEntry::setRelease(const QString& release)
{
    d->release = release;
}

int Echonest::CatalogUpdateEntry::skipCount() const
{
    return d->skip_count;
}

void Echonest::CatalogUpdateEntry::setSkipCount(int skipCount)
{
    d->skip_count = skipCount;
}

QByteArray Echonest::CatalogUpdateEntry::songId() const
{
    return d->song_id;
}

void Echonest::CatalogUpdateEntry::setSongId(const QByteArray& id)
{
    d->song_id = id;
}

QString Echonest::CatalogUpdateEntry::songName() const
{
    return d->song_name;
}

void Echonest::CatalogUpdateEntry::setSongName(const QString& name)
{
    d->song_name = name;
}

int Echonest::CatalogUpdateEntry::trackNumber() const
{
    return d->track_number;
}

void Echonest::CatalogUpdateEntry::setTrackNumber(int trackNum)
{
    d->track_number = trackNum;
}

QString Echonest::CatalogUpdateEntry::url() const
{
    return d->url;
}

void Echonest::CatalogUpdateEntry::setUrl(const QString& url)
{
    d->url = url;
}

bool Echonest::CatalogUpdateEntry::bannedSet() const
{
    return d->bannedSet;
}

bool Echonest::CatalogUpdateEntry::favoriteSet() const
{
    return d->favoriteSet;
}
