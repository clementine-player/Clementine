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
#ifndef LASTFM_ALBUM_H
#define LASTFM_ALBUM_H

#include "AbstractType.h"

#include "Artist.h"
#include "Mbid.h"

namespace lastfm
{
    class LASTFM_DLLEXPORT Album : public AbstractType
    {
    public:
        Album();
        explicit Album( Mbid mbid );
        Album( Artist artist, QString title );
        Album( const Album& album );
        ~Album();

        QDomElement toDomElement( QDomDocument& ) const;

        virtual QUrl imageUrl( ImageSize size, bool square = false ) const;
        void setImageUrl( ImageSize size, const QString& url );

        void setArtist( const QString& artist );

        bool operator==( const Album& that ) const;
        bool operator!=( const Album& that ) const;
        Album& operator=( const Album& that );
    
        QString toString() const;
        operator QString() const;
        QString title() const;
        Artist artist() const;
        Mbid mbid() const;

        /** artist may have been set, since we allow that in the ctor, but should we handle untitled albums? */
        bool isNull() const;
    
        /** Album.getInfo WebService */
        QNetworkReply* getInfo( const QString& username = "" ) const;
        QNetworkReply* share( const QStringList& recipients, const QString& message = "", bool isPublic = true ) const;

        /** use Tag::list to get the tag list out of the finished reply */
        QNetworkReply* getTags() const;
        QNetworkReply* getTopTags() const;
        
        /** Last.fm dictates that you may submit at most 10 of these */
        QNetworkReply* addTags( const QStringList& ) const;
    
        /** the Last.fm website url for this album */
        QUrl www() const;

    private:
        QExplicitlySharedDataPointer<class AlbumPrivate> d;
    };
}

#endif //LASTFM_ALBUM_H
