/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

//Copyright: (C) 2003 Mark Kretschmann
//           (C) 2004 Max Howell, <max.howell@methylblue.com>
//License:   See COPYING

#ifndef AMAROK_ENGINEBASE_H
#define AMAROK_ENGINEBASE_H

#include <QUrl>
#include <QObject>
#include <QList>

#include <sys/types.h>
#include <vector>


/**
 * @class Engine::Base
 * @author Mark Kretshmann
 * @author Max Howell
 *
 * This is an abstract base class that you need to derive when making your own backends.
 * It is typdefed to EngineBase for your conveniece.
 *
 * The only key thing to get right is what to return from state(), as some Amarok
 * behaviour is dependent on you returning the right state at the right time.
 *
 *   Empty   = No URL loaded and ready to play
 *   Idle    = URL ready for play, but not playing, so before AND after playback
 *   Playing = Playing a stream
 *   Paused  = Stream playback is paused
 *
 * Not returning idle when you have reached End-Of-Stream but Amarok has not told you
 * to stop would be bad because some components behave differently when the engine is
 * Empty or not. You are Idle because you still have a URL assigned.
 *
 * load( KURL ) is a key function because after this point your engine is loaded, and
 * Amarok will expect you to be able to play the URL until stop() or another load() is
 * called.
 *
 * You must handle your own media, do not rely on Amarok to call stop() before play() etc.
 *
 * At this time, emitting stateChanged( Engine::Idle ) is not necessary, otherwise you should
 * let Amarok know of state changes so it updates the UI correctly.
 *
 * Basically, reimplement everything virtual and ensure you emit stateChanged() correctly,
 * try not to block in any function that is called by Amarok, try to keep the user informed
 * with emit statusText()
 *
 * Only canDecode() needs to be thread-safe. Everything else is only called from the GUI thread.
 */

#include "engine_fwd.h"

namespace Engine
{
    typedef std::vector<int16_t> Scope;

    class Base : public QObject
    {
    Q_OBJECT

    signals:
        /** Emitted when end of current track is reached. */
        void trackEnded();

        /** Transmits status message, the message disappears after ~2s. */
        void statusText( const QString& );

        /**
         * Shows a long message in a non-invasive manner, you should prefer
         * this over KMessageBoxes, but do use KMessageBox when you must
         * interrupt the user or the message is very important.
         */
        void infoMessage( const QString& );

        void error( const QString& );

        /** Transmits metadata package. */
        void metaData( const Engine::SimpleMetaBundle& );

        /** Signals that a SYNC has been recieved, and new last.fm data needs to be downloaded */
        void lastFmTrackChange();

        /** Signals a change in the engine's state. */
        void stateChanged( Engine::State );

        /** Shows Amarok config dialog at specified page */
        void showConfigDialog( const QString& );

    public:
        virtual ~Base();

        /**
         * Initializes the engine. Must be called after the engine was loaded.
         * @return True if initialization was successful.
         */
        virtual bool init() = 0;

        /**
         * Determines if the engine is able to play a given URL.
         * @param url The URL of the file/stream.
         * @return True if we can play the URL.
         */
        virtual bool canDecode( const QUrl &url ) const = 0;

        /**
         * Determines if current track is a stream.
         * @return True if track is a stream.
         */
        inline bool isStream() { return m_isStream; }

        /**
         * Load new track for playing.
         * @param url URL to be played.
         * @param stream True if URL is a stream.
         * @return True for success.
         */
        virtual bool load( const QUrl &url, bool stream = false );

        /**
         * Load new track and start Playback. Convenience function for Amarok to use.
         * @param url URL to be played.
         * @param stream True if URL is a stream.
         * @return True for success.
         */
        bool play( const QUrl &u, bool stream = false ) { return load( u, stream ) && play(); }

        /**
         * Start playback.
         * @param offset Start playing at @p msec position.
         * @return True for success.
         */
        virtual bool play( uint offset = 0 ) = 0;

        /** Stops playback */
        virtual void stop() = 0;

        /** Pauses playback */
        virtual void pause() = 0;

        /** Resumes playback if paused */
        virtual void unpause() = 0;

         /**
         * Get current engine status.
         * @return the correct State as described at the enum
         */
        virtual State state() const = 0;

        /** Get time position (msec). */
        virtual uint position() const = 0;

        /** Get track length (msec). */
        virtual uint length() const { return 0; }

        /**
         * Jump to new time position.
         * @param ms New position.
         */
        virtual void seek( uint ms ) = 0;

        /**
         * Determines whether media is currently loaded.
         * @return True if media is loaded, system is ready to play.
         */
        inline bool loaded() const { return state() != Empty; }

        inline uint volume() const { return m_volume; }

        /**
         * Fetch the current audio sample buffer.
         * @return Audio sample buffer.
         */
        virtual const Scope &scope() { return m_scope; };

        /**
         * Set new volume value.
         * @param value Volume in range 0 to 100.
         */
        void setVolume( uint value );

        /** Set new crossfade length (msec) */
        void setXfadeLength( int value ) { m_xfadeLength = value; }

        /** Set whether to crossfade the next track
          * Used when the engine is switching tracks automatically
          * instead of manually.
          */
        void setXFadeNextTrack( bool enable ) { m_xfadeNextTrack = enable; }

        /** Set whether equalizer is enabled
          * You don't need to cache the parameters, setEqualizerParameters is called straight after this
          * function, _always_.
          */
        virtual void setEqualizerEnabled( bool ) {};

        /** Set equalizer parameters, all in range -100..100, where 0 = no adjustment
          * @param preamp the preamplification value
          * @param bandGains a list of 10 integers, ascending in frequency, the exact frequencies you amplify
          *                  are not too-important at this time
          */
        virtual void setEqualizerParameters( int /*preamp*/, const QList<int> &/*bandGains*/ ) {};


        /** Tries to retrieve metadata for the given url (called only if url
         * is not in the collection).  The intended usage is to retrieve
         * information for AudiCD tracks when they are added to the playlist
         * (i.e. before they are actually played)
         * @param url the url of the item
         * @param bundle the SimpleMetaBundle to fill
         * @return true if metadata found, false otherwise
         */
        virtual bool metaDataForUrl(const QUrl &, Engine::SimpleMetaBundle &)
        { return false; }

        /** returns true if this engine performs some special action to play
         * audio cds: in this case, the KURL::List is filled with the urls of
         * the songs in the cd...
         *
         * @param device the cdrom device , with QString::null meaning use engine-specific default value
         * @param urls the list of urls for AudioCD tracks to fill
         * @return true if the engine has the feature of reading from audio cds, false otherwise (note that this should return true also in case of error if the engine is capable of reading audio cds in general...)
         * */
        virtual bool getAudioCDContents(const QString &, QList<QUrl>&)
        { return false; }

        /** 
         * Whether amarok_proxy.rb is needed for last.fm files.
         * @return true if engine doesn't handle 'SYNC' messages in the stream from last.fm.
                   False if (like >=libxine-1.1.8) it does.
         */
        virtual bool lastFmProxyRequired() { return true; }

        /** flush the current stream buffer */
        virtual bool flushBuffer() { return false; }

        /** allow the engine to perform necessary work on changes in the playlist **/
        virtual void playlistChanged() { };

        virtual void reloadSettings() {};

        static const char* kSettingsGroup;

    protected:
        Base();

        /** Shows the Amarok configuration dialog at the engine page */
        void showEngineConfigDialog() { emit showConfigDialog( "Engine" ); }

        virtual void setVolumeSW( uint percent ) = 0;

        /** Converts master volume to a logarithmic scale */
        static uint makeVolumeLogarithmic( uint volume );

        Base( const Base& ); //disable copy constructor
        const Base &operator=( const Base& ); //disable copy constructor

        int           m_xfadeLength;
        bool          m_xfadeNextTrack;

    protected:
        static const int SCOPESIZE = 1024;
        uint  m_volume;
        QUrl  m_url;
        Scope m_scope;
        bool  m_isStream;
    };


    class SimpleMetaBundle {
    public:
        QString title;
        QString artist;
        QString album;
        QString comment;
        QString genre;
        QString bitrate;
        QString samplerate;
        QString length;
        QString year;
        QString tracknr;
    };
}

#endif
