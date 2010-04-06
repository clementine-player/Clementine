/***************************************************************************
 *   Copyright (C) 2003-2005 by Mark Kretschmann <markey@web.de>           *
 *   Copyright (C) 2005 by Jakub Stachowski <qbast@go2.pl>                 *
 *   Portions Copyright (C) 2006 Paul Cifarelli <paul@cifarelli.net>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#ifndef AMAROK_GSTENGINE_H
#define AMAROK_GSTENGINE_H

#include "enginebase.h"

#include <QString>
#include <QTimerEvent>
#include <QList>
#include <QStringList>
#include <QMutex>

#include <gst/gst.h>

class QTimerEvent;

/**
 * @class GstEngine
 * @short GStreamer engine plugin
 * @author Mark Kretschmann <markey@web.de>
 *
 * GstEngine uses following pipeline for playing (syntax used by gst-launch):
 * { filesrc location=file.ext ! decodebin ! audioconvert ! audioscale ! volume
 * ! adder } ! { queue ! equalizer ! identity ! volume ! audiosink }
 */
class GstEngine : public Engine::Base
{
        friend class GstConfigDialog;

        Q_OBJECT

    public:
        GstEngine();
        ~GstEngine();

        bool init();

        bool canDecode( const QUrl &url ) const;
        uint position() const;
        uint length() const;
        Engine::State state() const;
        const Engine::Scope& scope();

        virtual bool metaDataForUrl(const QUrl &url, Engine::SimpleMetaBundle &b);
        virtual bool getAudioCDContents(const QString &device, QList<QUrl> &urls);

        void gstStatusText( const QString& str ) { emit statusText( str ); }
        void gstMetaData( Engine::SimpleMetaBundle &bundle ) { emit metaData( bundle ); }

    public slots:
        bool load( const QUrl&, bool stream );
        bool play( uint offset );
        void stop();
        void pause();
        void unpause();
        void seek( uint ms );

        /** Copies incoming radio stream data from StreamProvider into StreamSrc's buffer */
        void newStreamData( char* data, int size );

        /** Set whether equalizer is enabled */
        void setEqualizerEnabled( bool );

        /** Set equalizer preamp and gains, range -100..100. Gains are 10 values. */
        void setEqualizerParameters( int preamp, const QList<int>& bandGains );

    protected:
        void setVolumeSW( uint percent );
        void timerEvent( QTimerEvent* );

    private slots:
        void handlePipelineError();
        void endOfStreamReached();

        /** Called when no output sink was selected. Shows the GStreamer engine settings dialog. */
        void errorNoOutput();

        /** Transmits new decoded metadata to the application */
        void newMetaData();

    private:
        static GstEngine* instance() { return s_instance; }

        /**
         * Creates a GStreamer element and puts it into pipeline.
         * @param factoryName Name of the element class to create.
         * @param bin Container into which the element is put.
         * @param name Identifier for the element.
         * @return Pointer to the created element, or NULL for failure.
         */
        static GstElement* createElement( const QString& factoryName, GstElement* bin = 0, const QString& name = 0 );

        /**
         * Fetches a list of available output sink plugins
         * @return List of output sinks
         */
        QStringList getOutputsList() { return getPluginList( "Sink/Audio" ); }

        // CALLBACKS:
        /** Bus message */
        //static GstBusSyncReply bus_cb( GstBus*, GstMessage*, gpointer );
        static gboolean bus_cb( GstBus*, GstMessage*, gpointer );
        /** Called at end of track */
        static void eos_cb( GstElement*, gpointer );
        /** Called when decodebin has generated a new pad */
        static void newPad_cb( GstElement*, GstPad*, gboolean, gpointer );
        /** Used by canDecode(). When called, the format probably can be decoded */
        static void candecode_newPad_cb( GstElement*, GstPad*, gboolean, gpointer );
        /** Used by canDecode(). Called after last pad so it makes no sense to wait anymore */
        static void candecode_last_cb( GstElement*, gpointer );
        /** Called when new metadata tags have been found */
        static void event_cb( GstPad*, GstEvent* event, gpointer arg);
        //static void found_tag_cb( GstElement*, GstElement*, GstTagList*, gpointer );
        /** Duplicates audio data for application side processing */
        static void handoff_cb( GstPad*, GstBuffer*, gpointer );
        /** Called when the KIO buffer is empty */
        static void kio_resume_cb();

        /** Get a list of available plugins from a specified Class */
        QStringList getPluginList( const QString& classname ) const;

        /** Construct the output pipeline */
        bool createPipeline();

        /** Stops playback, destroys all input pipelines, destroys output pipeline, and frees resources */
        void destroyPipeline();

        /* Constructs the pipeline for audio CDs, optionally selecting a device and/or track and/or setting the state to paused */
        bool setupAudioCD( const QString& device, unsigned track, bool pause );

        /** Beams the streaming buffer status to Amarok */
        void sendBufferStatus();

        /////////////////////////////////////////////////////////////////////////////////////
        // DATA MEMBERS
        /////////////////////////////////////////////////////////////////////////////////////
        // Interval of main timer, handles the volume fading
        static const int  TIMER_INTERVAL = 40; //msec

        #define KB 1000
        static const uint SCOPEBUF_SIZE  = 600*KB;
        static const int  SCOPE_VALUES   = 512;
        static const int  STREAMBUF_SIZE = 600*KB;
        static const uint STREAMBUF_MIN  = 100*KB;
        static const int  STREAMBUF_MAX  = STREAMBUF_SIZE - 50*KB;
        #undef KB


        static GstEngine* s_instance;

        GstElement* m_gst_pipeline;

        GstElement* m_gst_src;
        GstElement* m_gst_decodebin;

        GstElement* m_gst_audiobin;

        GstElement* m_gst_audioconvert;
        //GstElement* m_gst_equalizer;
        GstElement* m_gst_identity;
        GstElement* m_gst_volume;
        GstElement* m_gst_audioscale;
        GstElement* m_gst_audiosink;

        QString m_gst_error;
        QString m_gst_debug;

        int m_metacount;

        uint event_cb_id_;

        //////////
        // scope
        //////////
        // delay queue for synchronizing samples to where the audio device is playing
        GQueue    *m_delayq;
        // the current set of samples for the scope, in case we don't have enough buffers yet
        // and end up with an incomplete buffer
        float    m_currentScope[SCOPESIZE];
        // the sample in m_currentScope we are working on
        gint       m_current;
        // function to remove buffers that are no longer relevant
        // returns the position currently playing in the audio device
        gint64 pruneScope();
        // free all the buffers in the delay queue
        void clearScopeQ();

        // These variables are shared between gst-engine and streamsrc
        char*    m_streamBuf;
        int      m_streamBufIndex;
        bool     m_streamBufStop;
        bool     m_streamBuffering;

        QMutex            m_mutexScope;

        bool              m_pipelineFilled;
        float             m_fadeValue;

        bool              m_equalizerEnabled;
        int               m_equalizerPreamp;
        QList<int>        m_equalizerGains;

        Engine::SimpleMetaBundle m_metaBundle;

        bool m_shutdown;
        mutable bool m_canDecodeSuccess;
        mutable bool m_canDecodeLast;
};


#endif /*AMAROK_GSTENGINE_H*/

