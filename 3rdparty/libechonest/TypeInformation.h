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


#ifndef ECHONEST_TYPEINFORMATION_H
#define ECHONEST_TYPEINFORMATION_H

#include "echonest_export.h"

#include <qglobal.h>
#include <QStringList>
#include <QMetaType>

namespace Echonest {
    class ArtistInformationPrivate;
    class SongInformationPrivate;
    
    /**
     * This class describes what artist information to return with a query.
     * 
     * The default behaviour is NoInformation.
     */
    class ECHONEST_EXPORT ArtistInformation
    {
    public:
        enum ArtistInformationFlag {
            NoInformation = 0x0000,
            Audio = 0x0001,
            Biographies = 0x0002,
            Blogs = 0x0004,
            Familiarity = 0x0008,
            Hotttnesss = 0x0010,
            Images = 0x0020,
            News = 0x0040,
            Reviews = 0x0080,
            Terms = 0x0100,
            Urls = 0x200,
            Videos = 0x0400
        };
        Q_DECLARE_FLAGS( ArtistInformationFlags, ArtistInformationFlag )
        
        ArtistInformation();
        ArtistInformation( ArtistInformationFlags flags );
        ArtistInformation( ArtistInformationFlags flags, const QStringList& idSpaces );
        ArtistInformation( const ArtistInformation& other );
        ~ArtistInformation();
        ArtistInformation& operator=( const ArtistInformation& typeInfo );
        
        /**
         * The individual pieces of information to fetch for this artist.
         * 
         * Use \c setIdSpaces to set an id space for this query.
         */
        ArtistInformationFlags flags() const;
        void setArtistInformationFlags( ArtistInformationFlags flags );
        
        /**
         * The id spaces to limit this to. Do not include the "id:" prefix.
         */
        QStringList idSpaces() const;
        void setIdSpaces( const QStringList& idspaces );
       
        private:
            ArtistInformationPrivate* d_ptr;
            Q_DECLARE_PRIVATE( ArtistInformation )
    };
    
    Q_DECLARE_OPERATORS_FOR_FLAGS( ArtistInformation::ArtistInformationFlags )
    
    /**
     * This class describes what song information to return with a query.
     * 
     * The default behaviour is NoInformation.
     */
    class ECHONEST_EXPORT SongInformation
    {
    public:
        enum SongInformationFlag {
            AudioSummaryInformation = 0x001,
            Tracks = 0x002,
            Hotttnesss = 0x04,
            ArtistHotttnesss = 0x008,
            ArtistFamiliarity = 0x010,
            ArtistLocation = 0x020,
            
            NoInformation = 0x800
        };
        Q_DECLARE_FLAGS( SongInformationFlags, SongInformationFlag )
        
        SongInformation();
        SongInformation( SongInformationFlags flags );
        SongInformation( SongInformationFlags flags, const QStringList& idSpaces );
        SongInformation( const SongInformation& other );
        ~SongInformation();
        SongInformation& operator=( const SongInformation& info );
        
        /**
         * The individual pieces of information to fetch for this song.
         *  If id spaces are desired,see \c setIdSpaces for more information.
         */
        SongInformationFlags flags() const;
        void setSongInformationFlags( SongInformationFlags flags );
        
        /**
         * The id spaces to limit this to. Do not include the "id:" prefix.
         */
        QStringList idSpaces() const;
        void setIdSpaces( const QStringList& idspaces );
        
    private:
        SongInformationPrivate* d_ptr;
        Q_DECLARE_PRIVATE( SongInformation )
        
    };
    
    Q_DECLARE_OPERATORS_FOR_FLAGS(SongInformation::SongInformationFlags)
}

Q_DECLARE_METATYPE( Echonest::ArtistInformation )
Q_DECLARE_METATYPE( Echonest::SongInformation )

#endif
