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
#ifndef LASTFM_TUNER_H
#define LASTFM_TUNER_H

#include "Track.h"
#include "ws.h"

#include <QObject>

namespace lastfm
{
    class RadioStation;

    /** With regard to error handling. We handle Ws::TryAgain up to 5 times,
      * don't try again after that! Just tell the user to try again later. 
      */
    class LASTFM_DLLEXPORT RadioTuner : public QObject
    {
        Q_OBJECT
    
    public:
        /** You need to have assigned Ws::* for this to work, creating the tuner
          * automatically fetches the first 5 tracks for the station */
        explicit RadioTuner( const RadioStation& );
        ~RadioTuner();

        Track takeNextTrack();

        void retune( const RadioStation& );

        void queueTrack( lastfm::Track& track );

    signals:
        void title( const QString& );
        void supportsDisco( bool supportsDisco );
        void trackAvailable();
        void error( lastfm::ws::Error, const QString& message );

    private slots:
        void onTuneReturn();
        void onGetPlaylistReturn();
        // no-op
        void onXspfExpired();

    private:
        class RadioTunerPrivate * const d;
    };
}

#endif
