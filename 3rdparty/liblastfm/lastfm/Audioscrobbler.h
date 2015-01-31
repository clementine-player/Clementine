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
#ifndef LASTFM_AUDIOSCROBBLER_H
#define LASTFM_AUDIOSCROBBLER_H

#include "global.h"
#include <QList>
#include <QObject>

namespace lastfm
{
    class Track;

    /** @author Max Howell <max@last.fm>
      * An implementation of the Audioscrobbler Realtime Submissions Protocol 
      * version 1.2.1 for a single Last.fm user
      * http://www.audioscrobbler.net/development/protocol/
      */
    class LASTFM_DLLEXPORT Audioscrobbler : public QObject
    {
        Q_OBJECT

    public:
        /** You will need to do QCoreApplication::setVersion and 
          * QCoreApplication::setApplicationName for this to work, also you will 
          * need to have set all the keys in the Ws namespace in WsKeys.h */
        Audioscrobbler( const QString& clientId );
        ~Audioscrobbler();

    signals:
        void scrobblesCached( const QList<lastfm::Track>& tracks );
        /* Note that this is emitted after we tried to submit the scrobbles
        It could just be that they have an error code */
        void scrobblesSubmitted( const QList<lastfm::Track>& tracks );
        void nowPlayingError( int code, QString message );

    public slots:
        /** will ask Last.fm to update the now playing information for the 
          * authenticated user */
        void nowPlaying( const Track& );
        /** will cache the track and call submit() */
        void cache( const Track& );
        void cacheBatch( const QList<lastfm::Track>&, const QString& id );
        void cacheBatch( const QList<lastfm::Track>& );

        /** will submit the submission cache for this user */
        void submit();

    private slots:
        void onNowPlayingReturn();
        void onTrackScrobbleReturn();

    private:
        class AudioscrobblerPrivate * const d;
    };
}

#endif
