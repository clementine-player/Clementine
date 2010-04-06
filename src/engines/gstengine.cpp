/***************************************************************************
 *   Copyright (C) 2003-2005 by Mark Kretschmann <markey@web.de>           *
 *   Copyright (C) 2005 by Jakub Stachowski <qbast@go2.pl>                 *
 *   Copyright (C) 2006 Paul Cifarelli <paul@cifarelli.net>                *
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

#define DEBUG_PREFIX "Gst-Engine"

#include "enginebase.h"
#include "gstengine.h"

#include <math.h>
#include <unistd.h>
#include <vector>

#include <QTimer>
#include <QRegExp>
#include <QFile>
#include <QMessageBox>
#include <QtDebug>

#include <gst/gst.h>
#include <iostream>

#define RETURN_IF_PIPELINE_EMPTY if ( !m_pipelineFilled ) return;


using std::vector;

GstEngine* GstEngine::s_instance;


/////////////////////////////////////////////////////////////////////////////////////
// CALLBACKS
/////////////////////////////////////////////////////////////////////////////////////

//GstBusSyncReply
gboolean
GstEngine::bus_cb(GstBus*, GstMessage* msg, gpointer) // static
{
   switch ( GST_MESSAGE_TYPE(msg))
   {
   	case GST_MESSAGE_ERROR:
	{
	    GError* error;
	    gchar* debugs;

            gst_message_parse_error(msg,&error,&debugs);
            qDebug() << "ERROR RECEIVED IN BUS_CB <" << error->message << ">" ;;

            instance()->m_gst_error = QString::fromAscii( error->message );
    	    instance()->m_gst_debug = QString::fromAscii( debugs );
            QTimer::singleShot( 0, instance(), SLOT( handlePipelineError() ) );
	    break;
	}
        //case GST_MESSAGE_EOS:
        //   qDebug() << "EOS reached";
	//     QTimer::singleShot( 0, instance(), SLOT( endOfStreamReached() ) );
	//     break;
	case GST_MESSAGE_TAG:
	{
	     gchar* string=NULL;
    	     Engine::SimpleMetaBundle bundle;
	     GstTagList* taglist;
	     gst_message_parse_tag(msg,&taglist);
             bool success = false;

             if ( gst_tag_list_get_string( taglist, GST_TAG_TITLE, &string ) && string ) {
                  qDebug() << "received tag 'Title': " << QString( string ) ;
                  bundle.title = QString( string );
                  success = true;
             }
             if ( gst_tag_list_get_string( taglist, GST_TAG_ARTIST, &string ) && string ) {
                  qDebug() << "received tag 'Artist': " << QString( string ) ;
                  bundle.artist = QString( string );
                  success = true;
             }
             if ( gst_tag_list_get_string( taglist, GST_TAG_COMMENT, &string ) && string ) {
                  qDebug() << "received tag 'Comment': " << QString( string ) ;
                  bundle.comment = QString( string );
                  success = true;
             }
             if ( gst_tag_list_get_string( taglist, GST_TAG_ALBUM, &string ) && string ) {
                  qDebug() << "received tag 'Album': " << QString( string ) ;
                  bundle.album = QString( string );
                  success = true;
             }
             g_free( string );
             gst_tag_list_free(taglist);
             if ( success ) {
                  instance()->m_metaBundle = bundle;
                  QTimer::singleShot( 0, instance(), SLOT( newMetaData() ) );
             }
	     break;
	 }
	 default: ;
      }
      return GST_BUS_DROP;
}


void
GstEngine::newPad_cb( GstElement*, GstPad* pad, gboolean, gpointer ) //static
{
    GstPad* const audiopad = gst_element_get_pad( instance()->m_gst_audiobin, "sink" );

    if ( GST_PAD_IS_LINKED( audiopad ) ) {
        qDebug() << "audiopad is already linked. Unlinking old pad." ;
        gst_pad_unlink( audiopad, GST_PAD_PEER( audiopad ) );
    }

    gst_pad_link( pad, audiopad );

    gst_object_unref( audiopad );
}


void
GstEngine::handoff_cb( GstPad*, GstBuffer* buf, gpointer arg) //static
{
   GstEngine *thisObj = static_cast<GstEngine *>( arg );

   // push the buffer onto the delay queue
   gst_buffer_ref(buf);
   g_queue_push_tail(thisObj->m_delayq, buf);
}

void
GstEngine::event_cb( GstPad*, GstEvent* event, gpointer /*arg*/) //static
{
   //GstEngine *thisObj = static_cast<GstEngine *>( arg );

   std::cerr << "*** event_cb\n";
   switch ( static_cast<int>(event->type) )
   {
      case GST_EVENT_EOS:
         qDebug() << "EOS reached";
         QTimer::singleShot( 0, instance(), SLOT( endOfStreamReached() ) );
         break;
      case GST_EVENT_TAG:
      {
         qDebug() << "GOT NEW TAG";
      }
      case GST_EVENT_NEWSEGMENT:
         break;
      default:
         qDebug() << "** unknown event" << static_cast<int>(event->type);
         break;
   }
}


void
GstEngine::candecode_newPad_cb( GstElement*, GstPad* pad, gboolean, gpointer ) //static
{
    GstCaps* caps = gst_pad_get_caps( pad );
    if (gst_caps_get_size(caps)>0) {
        GstStructure* str = gst_caps_get_structure( caps,0 );
        if (g_strrstr(gst_structure_get_name( str ), "audio" )) instance()->m_canDecodeSuccess = true;
    }
    gst_caps_unref( caps );
}

void
GstEngine::candecode_last_cb( GstElement*, gpointer ) //static
{
    instance()->m_canDecodeLast = true;
}


/////////////////////////////////////////////////////////////////////////////////////
// CLASS GSTENGINE
/////////////////////////////////////////////////////////////////////////////////////

GstEngine::GstEngine()
        : Engine::Base()
        , event_cb_id_(0)
        , m_delayq(0)
        , m_current(0)
        , m_streamBuf( new char[STREAMBUF_SIZE] )
        , m_streamBuffering( false )
        , m_pipelineFilled( false )
        , m_fadeValue( 0.0 )
        , m_equalizerEnabled( false )
        , m_shutdown( false )
{
    // initialize the scope delay queue
    m_delayq = g_queue_new();
}


GstEngine::~GstEngine()
{
    destroyPipeline();

#ifdef GST_KIOSTREAMS
    delete[] m_streamBuf;
#endif

    // Destroy scope delay queue
    g_queue_free(m_delayq);

    // Save configuration
    gst_deinit();
}


/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
/////////////////////////////////////////////////////////////////////////////////////

bool
GstEngine::init()
{
    s_instance = this;

    // GStreamer initialization
    GError *err;
    if ( !gst_init_check( NULL, NULL, &err ) ) {
        emit error("GStreamer could not be initialized");
        return false;
    }

    // Check if registry exists
    GstElement* dummy = gst_element_factory_make ( "fakesink", "fakesink" );
    if ( !dummy ) {
        emit error("GStreamer is missing a registry.  Please make sure that you"
                   " have installed all necessary GStreamer plugins (e.g. OGG"
                   " and MP3), and run 'gst-register' afterwards.");
        return false;
    }

    gst_object_unref( dummy );

    return true;
}


bool
GstEngine::canDecode( const QUrl &url ) const
{
    bool ismp3 = false;

    // We had some bug reports claiming that video files cause crashes in canDecode(),
    // so don't try to decode them
    if ( url.path().toLower().endsWith( ".mov" ) ||
         url.path().toLower().endsWith( ".avi" ) ||
         url.path().toLower().endsWith( ".wmv" ) )
        return false;

    if ( url.path().toLower().endsWith( ".mp3" ) )
       ismp3 = true;

    qDebug() << "Can decode for " << url.toString() ;
    int count = 0;
    m_canDecodeSuccess = false;
    m_canDecodeLast = false;
    GstElement *pipeline, *giosrc, *decodebin;

    if ( !( pipeline = createElement( "pipeline" ) ) ) return false;
    if ( !( giosrc = createElement( "giosrc", pipeline ) ) ) return false;
    if ( !( decodebin = createElement( "decodebin", pipeline ) ) ) return false;


    gst_element_link( giosrc, decodebin );

    g_object_set( G_OBJECT( giosrc ), "location", (const char*) QFile::encodeName( url.path() ), NULL );
    g_signal_connect( G_OBJECT( decodebin ), "new-decoded-pad", G_CALLBACK( candecode_newPad_cb ), NULL );
    g_signal_connect( G_OBJECT( decodebin ), "no-more-pads", G_CALLBACK( candecode_last_cb ), NULL );

    GstStateChangeReturn sret;
    sret = gst_element_set_state( pipeline, GST_STATE_PLAYING );
    if (sret == GST_STATE_CHANGE_ASYNC)
       qDebug() << "gst set state returns ASYNC";
    else
       qDebug() << "gst set state does not return ASYNC";

    // Wait until found audio stream

    while ( !m_canDecodeSuccess && !m_canDecodeLast && count < 100 ) {
        count++;
	usleep(1000);
    }

    qDebug() << "Got " << m_canDecodeSuccess << " after " << count << " sleeps" ;
    gst_element_set_state( pipeline, GST_STATE_NULL );
    gst_object_unref( GST_OBJECT( pipeline ) );

    return m_canDecodeSuccess;
}


uint
GstEngine::position() const
{
    if ( !m_pipelineFilled ) return 0;

    GstFormat fmt = GST_FORMAT_TIME;
    // Value will hold the current time position in nanoseconds. Must be initialized!
    gint64 value = 0;
    gst_element_query_position( m_gst_pipeline, &fmt, &value );

    return static_cast<uint>( ( value / GST_MSECOND ) ); // nanosec -> msec
}


uint
GstEngine::length() const
{
    if ( !m_pipelineFilled ) return 0;

    GstFormat fmt = GST_FORMAT_TIME;
    // Value will hold the track length in nanoseconds. Must be initialized!
    gint64 value = 0;
    gst_element_query_duration( m_gst_pipeline,  &fmt, &value );

    return static_cast<uint>( ( value / GST_MSECOND ) ); // nanosec -> msec
}


#define GST_STATE_TIMEOUT 10000000

Engine::State
GstEngine::state() const
{
    if ( !m_pipelineFilled )
        return m_url.isEmpty() ? Engine::Empty : Engine::Idle;

    GstStateChangeReturn sret;
    GstState s, sp;
    sret = gst_element_get_state( m_gst_pipeline, &s, &sp, GST_STATE_TIMEOUT );
    if (sret != GST_STATE_CHANGE_FAILURE)
    {
       switch ( s )
       {
          case GST_STATE_NULL:
             return Engine::Empty;
          case GST_STATE_READY:
             return Engine::Idle;
          case GST_STATE_PLAYING:
             return Engine::Playing;
          case GST_STATE_PAUSED:
             return Engine::Paused;

          default:
             return Engine::Empty;
       }
    }
    else
    {
       qDebug() << "Gst get state fails";
       return Engine::Empty;
    }
}


const Engine::Scope&
GstEngine::scope()
{
   gint channels = 2;

   typedef float sampletype;

   // prune the scope and get the current pos of the audio device
   gint64 pos = pruneScope();
   guint64 stime = 0, etime = 0, dur = 0;
   gint off, frames, sz;
   sampletype *data;

   // head of the delay queue is the most delayed, so we work with that one
   GstBuffer *buf = reinterpret_cast<GstBuffer *>( g_queue_peek_head(m_delayq) );
   if (buf)
   {
      // start time for this buffer
      stime = static_cast<guint64>( GST_BUFFER_TIMESTAMP( buf ) );
      // duration of the buffer...
      dur = static_cast<guint64>( GST_BUFFER_DURATION( buf ) );
      // therefore we can calculate the end time for the buffer
      etime = stime + dur;

      // determine the number of channels
      GstStructure* structure = gst_caps_get_structure ( GST_BUFFER_CAPS( buf ), 0);
      gst_structure_get_int (structure, "channels", &channels);

      // scope does not support >2 channels
      if (channels > 2)
         return m_scope;

      // if the audio device is playing this buffer now
      if (static_cast<guint64>(pos) > stime && static_cast<guint64>(pos) < etime)
      {
         // calculate the number of samples in the buffer
         sz = GST_BUFFER_SIZE(buf) / sizeof(sampletype);
         // number of frames is the number of samples in each channel (frames like in the alsa sense)
         frames = sz / channels;

         // find the offset into the buffer to the sample closest to where the audio device is playing
         // it is the (time into the buffer cooresponding to the audio device pos) / (the sample rate)
         // sample rate = duration of the buffer / number of frames in the buffer
         // then we multiply by the number of channels to find the offset of the left channel sample
         // of the frame in the buffer
         off = channels * (pos - stime) / (dur / frames);

         // note that we are assuming 32 bit samples, but this should probably be generalized...
         data = reinterpret_cast<sampletype *>( GST_BUFFER_DATA(buf) );
         if (off < sz) // better be...
         {
            int i = off; // starting at offset

            // loop while we fill the current buffer.  If we need another buffer and one is available,
            // get it and keep filling.  If there are no more buffers available (not too likely)
            // then leave everything in this state and wait until the next time the scope updates
            while (buf && m_current < SCOPESIZE && i < sz)
            {
               for (int j = 0; j < channels && m_current < SCOPESIZE; j++)
               {
                  m_currentScope[m_current ++] = data[i + j];
                  if (channels == 1)
                     m_currentScope[m_current ++] = data[i];
               }
               i+=channels; // advance to the next frame
               if (i >= sz) // here we are out of samples in the current buffer, so we get another one
               {
                  buf = reinterpret_cast<GstBuffer *>( g_queue_pop_head(m_delayq) );
                  gst_buffer_unref(buf);
                  buf = reinterpret_cast<GstBuffer *>( g_queue_peek_head(m_delayq) );
                  if (buf)
                  {
                     stime = static_cast<guint64>( GST_BUFFER_TIMESTAMP( buf ) );
                     dur = static_cast<guint64>( GST_BUFFER_DURATION( buf ) );
                     etime = stime + dur;
                     i = 0;
                     sz = GST_BUFFER_SIZE(buf) / sizeof(sampletype);
                     data = reinterpret_cast<sampletype *>( GST_BUFFER_DATA(buf) );
                  }
               }
            }
         }
      }
   }

   if (m_current >= SCOPESIZE)
   {
      // ok, we have a full buffer now, so give it to the scope
      for (int i=0; i< SCOPESIZE; i++)
          m_scope[i] = m_currentScope[i];
      m_current = 0;
   }

   return m_scope;

}


bool
GstEngine::metaDataForUrl(const QUrl &url, Engine::SimpleMetaBundle &b)
{
    qDebug() << "GstEngine::metaDataForUrl " << url ;
    /*if ( url.scheme() == "cdda" )
    {
        // TODO: gstreamer doesn't support cddb natively, but could perhaps use libkcddb?
        b.title = QString(  "Track %1"  ).arg( url.host() );
        b.album = "AudioCD";

        if ( setupAudioCD( url.encodedQuery().remove( QRegExp( "^\\?" ) ), url.host().toUInt(), true ) )
        {
            GstPad *pad;
            if ( ( pad = gst_element_get_pad( m_gst_src, "src" ) ) )
            {
                GstCaps *caps;
                if ( ( caps = gst_pad_get_caps( pad ) ) )
                {
                    GstStructure *structure;
                    if ( ( structure = gst_caps_get_structure( GST_CAPS(caps), 0 ) ) )
                    {
                        gint channels, rate, width;
                        gst_structure_get_int( structure, "channels", &channels );
                        gst_structure_get_int( structure, "rate", &rate );
                        gst_structure_get_int( structure, "width", &width );
                        b.bitrate = ( rate * width * channels ) / 1000;
                        b.samplerate = rate;
                    }
                    gst_caps_unref( caps );
                }

                GstQuery *query;
                if ( ( query = gst_query_new_duration( GST_FORMAT_TIME ) ) )
                {
                    if ( gst_pad_query( pad, query )) {
                        gint64 time;

                        gst_query_parse_duration( query, NULL, &time );
                        b.length = QString::number( time / GST_SECOND );
                    }
                    gst_query_unref( query );
                }
            }
            gst_object_unref( GST_OBJECT( pad ) );
            if ( !gst_element_set_state( m_gst_pipeline, GST_STATE_NULL ) ) {
                qWarning() << "Could not set thread to NULL.";
                destroyPipeline();
            }
        }
        return true;
    }*/
    return false;
}


bool
GstEngine::getAudioCDContents(const QString &device, QList<QUrl> &urls)
{
    /*qDebug() << "GstEngine::getAudioCDContents " << device ;

    bool result = false;
    if ( setupAudioCD( device, 0, true ) )
    {
        GstFormat format;
        if ( ( format = gst_format_get_by_nick("track") ) != GST_FORMAT_UNDEFINED )
        {
            gint64 tracks = 0;
            if ( gst_element_query_duration( m_gst_pipeline, &format, &tracks ) )
            {
                qDebug() << "Found " << tracks << " cdda tracks" ;
                for ( int i = 1; i <= tracks; ++i )
                {
                    QUrl temp( QString( "cdda://%1" ).arg( i ) );
                    if ( !device.isNull() )
                        temp.setQuery( device );
                    urls << temp;
                }
                result = true;
            }
        }
        if ( !gst_element_set_state( m_gst_pipeline, GST_STATE_NULL ) ) {
            qWarning() << "Could not set thread to NULL.";
            destroyPipeline();
        }
    }
    return result;*/
  return false;
}


/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC SLOTS
/////////////////////////////////////////////////////////////////////////////////////

bool
GstEngine::load( const QUrl& url, bool stream )  //SLOT
{
    Engine::Base::load( url, stream );
    qDebug() << "Loading url: " << url.toEncoded() ;

    if ( url.scheme() == "cdda" )
    {
        /*if ( !setupAudioCD( url.encodedQuery().remove( QRegExp( "^\\?" ) ), url.host().toUInt(), false ) )*/
            return false;
    }
    else
    {
       if ( !createPipeline() )
          return false;

       m_gst_src = createElement("giosrc");

       if (m_gst_src)
       {
          g_object_set (G_OBJECT (m_gst_src), "location", url.toEncoded().constData(), NULL);

          gst_bin_add( GST_BIN( m_gst_pipeline ), m_gst_src );
          //g_object_set( G_OBJECT(m_gst_src), "location", static_cast<const char*>( QFile::encodeName( url.url() ) ),
                        //                   "iradio-mode", true,
                        //                    NULL );

          //gchar *name;
          //gboolean mdenabled = false;
          //g_object_get( G_OBJECT(m_gst_src), "name",  &name, "iradio-mode", &mdenabled, NULL );
          //qDebug() << "NAME WAS " << name << " md enabled? " << mdenabled ;

          if ( !( m_gst_decodebin = createElement( "decodebin", m_gst_pipeline ) ) ) { destroyPipeline(); return false; }
          g_signal_connect( G_OBJECT( m_gst_decodebin ), "new-decoded-pad", G_CALLBACK( newPad_cb ), NULL );

          GstPad *p;
          p = gst_element_get_pad (m_gst_decodebin, "sink");
          if (p)
          {
             event_cb_id_ = gst_pad_add_event_probe (p, G_CALLBACK(event_cb), this);
             gst_object_unref (p);
          }

          // Link elements. The link from decodebin to audioconvert will be made in the newPad-callback
          gst_element_link( m_gst_src, m_gst_decodebin );
       }
       else
       {
          qDebug() << "******* cannot get stream src " ;

          destroyPipeline();
          return false;
       }
    }

    setVolume( m_volume );
    setEqualizerEnabled( m_equalizerEnabled );
    if ( m_equalizerEnabled ) setEqualizerParameters( m_equalizerPreamp, m_equalizerGains );
    return true;
}


bool
GstEngine::play( uint offset )  //SLOT
{
    // Try to play input pipeline; if fails, destroy input bin
    GstStateChangeReturn sret;
    if ( !(sret = gst_element_set_state( m_gst_pipeline, GST_STATE_PLAYING )) ) {
        qWarning() << "Could not set thread to PLAYING.";
        destroyPipeline();
        return false;
    }

    // If "Resume playback on start" is enabled, we must seek to the last position
    if ( offset ) seek( offset );

    m_current = 0;
    startTimer( TIMER_INTERVAL );
    emit stateChanged( Engine::Playing );
    return true;
}


void
GstEngine::stop()  //SLOT
{
    m_url = QUrl(); // To ensure we return Empty from state()

    if ( m_pipelineFilled )
    {
       // Is a fade running?
       if ( m_fadeValue == 0.0 ) {
          // Not fading --> start fade now
          m_fadeValue = 1.0;
       }
       else
          // Fading --> stop playback
          destroyPipeline();
    }

    emit stateChanged( Engine::Empty );
}

void
GstEngine::pause()  //SLOT
{
   RETURN_IF_PIPELINE_EMPTY

   if ( GST_STATE( m_gst_pipeline ) == GST_STATE_PLAYING ) {
      gst_element_set_state( m_gst_pipeline, GST_STATE_PAUSED );
      emit stateChanged( Engine::Paused );
   }
}

void
GstEngine::unpause()  //SLOT
{
    RETURN_IF_PIPELINE_EMPTY

    if ( GST_STATE( m_gst_pipeline ) == GST_STATE_PAUSED ) {
        gst_element_set_state( m_gst_pipeline, GST_STATE_PLAYING );
        emit stateChanged( Engine::Playing );
    }
}


void
GstEngine::seek( uint ms )  //SLOT
{
    RETURN_IF_PIPELINE_EMPTY

    if (!gst_element_seek(m_gst_pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, ms*GST_MSECOND,
       GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) qDebug() << "Seek failed" ;
    else clearScopeQ();
    gst_element_get_state(m_gst_pipeline, NULL, NULL, 100*GST_MSECOND);
}


void
GstEngine::newStreamData( char* buf, int size )  //SLOT
{
    if ( m_streamBufIndex + size >= STREAMBUF_SIZE ) {
        m_streamBufIndex = 0;
        qDebug() << "Stream buffer overflow!" ;
    }

    //qDebug() << "new stream data, size " << size ;
    sendBufferStatus();

    // Copy data into stream buffer
    memcpy( m_streamBuf + m_streamBufIndex, buf, size );
    // Adjust index
    m_streamBufIndex += size;
}

void
GstEngine::setEqualizerEnabled( bool enabled ) //SLOT
{
    m_equalizerEnabled = enabled;

    RETURN_IF_PIPELINE_EMPTY

    //g_object_set( G_OBJECT( m_gst_equalizer ), "active", enabled, NULL );
}


void
GstEngine::setEqualizerParameters( int preamp, const QList<int>& bandGains ) //SLOT
{
    m_equalizerPreamp = preamp;
    m_equalizerGains = bandGains;

    RETURN_IF_PIPELINE_EMPTY

    // BEGIN Preamp
    //g_object_set( G_OBJECT( m_gst_equalizer ) , "preamp", ( preamp + 100 ) / 2 , NULL );
    // END

    // BEGIN Gains
    vector<int> gainsTemp;
    gainsTemp.resize( bandGains.count() );
    for ( int i = 0; i < bandGains.count(); i++ )
        gainsTemp[i] = ( bandGains.at( i ) + 100 ) / 2;

    //g_object_set( G_OBJECT( m_gst_equalizer ), "gain", &gainsTemp, NULL );
    // END
}


/////////////////////////////////////////////////////////////////////////////////////
// PROTECTED
/////////////////////////////////////////////////////////////////////////////////////

void
GstEngine::setVolumeSW( uint percent )
{
    RETURN_IF_PIPELINE_EMPTY

    double fade;
    if ( m_fadeValue > 0.0 )
        fade = 1.0 - log10( ( 1.0 - m_fadeValue ) * 9.0 + 1.0 );
    else
        fade = 1.0;

    g_object_set( G_OBJECT(m_gst_volume), "volume", (double) percent * fade * 0.01, NULL );
}


void GstEngine::timerEvent( QTimerEvent* )
{
    // keep the scope from building while we are not visible
    // this is why the timer must run as long as we are playing, and not just when
    // we are fading
    pruneScope();

    // *** Volume fading ***

    // Are we currently fading?
    if ( m_fadeValue > 0.0 )
    {
        // TODO
        //m_fadeValue -= ( AmarokConfig::fadeoutLength() ) ?  1.0 / AmarokConfig::fadeoutLength() * TIMER_INTERVAL : 1.0;
        m_fadeValue -= 1.0;

        // Fade finished?
        if ( m_fadeValue <= 0.0 ) {
            // Fade transition has finished, stop playback
            qDebug() << "[Gst-Engine] Fade-out finished.";
            destroyPipeline();
            //killTimers();
        }
        setVolume( volume() );
    }
}


/////////////////////////////////////////////////////////////////////////////////////
// PRIVATE SLOTS
/////////////////////////////////////////////////////////////////////////////////////

void
GstEngine::handlePipelineError()  //SLOT
{
    QString text = "[GStreamer Error] ";
    text += m_gst_error;

    if ( !m_gst_debug.isEmpty() ) {
        text += " ** ";
        text += m_gst_debug;
    }

    m_gst_error = QString();
    emit statusText( text );
    qWarning() << text ;

    destroyPipeline();
}


void
GstEngine::endOfStreamReached()  //SLOT
{
    destroyPipeline();
    emit trackEnded();
}


void
GstEngine::newMetaData()  //SLOT
{
    emit metaData( m_metaBundle );
}


void
GstEngine::errorNoOutput() //SLOT
{
    QMessageBox::information( 0, "Error", "<p>Please select a GStreamer <u>output plugin</u> in the engine settings dialog.</p>" );

    // Show engine settings dialog
    showEngineConfigDialog();
}


/////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
/////////////////////////////////////////////////////////////////////////////////////

GstElement*
GstEngine::createElement( const QString& factoryName, GstElement* bin, const QString& name ) //static
{
    GstElement* element =
        gst_element_factory_make( factoryName.toAscii().constData(),
                                  name.isNull() ? factoryName.toAscii().constData() : name.toAscii().constData() );

    if ( element ) {
        if ( bin ) gst_bin_add( GST_BIN( bin ), element );
    }
    else {
        QMessageBox::critical( 0, "Error",
          QString("<h3>GStreamer could not create the element: <i>%1</i></h3> "
                  "<p>Please make sure that you have installed all necessary GStreamer plugins (e.g. OGG and MP3), and run <i>'gst-register'</i> afterwards.</p>"
                  "<p>For further assistance consult the GStreamer manual, and join #gstreamer on irc.freenode.net.</p>" ).arg( factoryName ) );
        gst_object_unref( GST_OBJECT( bin ) );
    }

    return element;
}


QStringList
GstEngine::getPluginList( const QString& classname ) const
{
    GList* features = NULL;
    QString name;
    QStringList results;

    GstRegistry* registry = gst_registry_get_default();
    features = gst_registry_get_feature_list(registry,GST_TYPE_ELEMENT_FACTORY);
        while ( features ) {
        GstElementFactory * factory = GST_ELEMENT_FACTORY ( features->data );
        if ( g_strrstr ( factory->details.klass, classname.toAscii().constData() ) ) {
            name = g_strdup ( GST_PLUGIN_FEATURE_NAME ( features->data ) );
            if ( name != "autoaudiosink" )
                results << name;
            }
        features = g_list_next ( features );
        }
    gst_plugin_feature_list_free(features);
    return results;
}


bool
GstEngine::createPipeline()
{
    destroyPipeline();

    QString output = "autoaudiosink";

    m_gst_pipeline = gst_pipeline_new( "pipeline" );
    m_gst_audiobin = gst_bin_new( "audiobin" );


    if ( !( m_gst_audiosink = createElement( output, m_gst_audiobin ) ) ) {
        QTimer::singleShot( 0, this, SLOT( errorNoOutput() ) );
        return false;
    }

    /*m_gst_equalizer = GST_ELEMENT( gst_equalizer_new() );
    gst_bin_add( GST_BIN( m_gst_audiobin ), m_gst_equalizer );*/
    if ( !( m_gst_audioconvert = createElement( "audioconvert", m_gst_audiobin ) ) ) { return false; }
    if ( !( m_gst_identity = createElement( "identity", m_gst_audiobin ) ) ) { return false; }
    if ( !( m_gst_volume = createElement( "volume", m_gst_audiobin ) ) ) { return false; }
    if ( !( m_gst_audioscale = createElement( "audioresample", m_gst_audiobin ) ) ) { return false; }

    GstPad* p;
    p = gst_element_get_pad(m_gst_audioconvert, "sink");
    gst_element_add_pad(m_gst_audiobin,gst_ghost_pad_new("sink",p));
    gst_object_unref(p);

    // add a data probe on the src pad if the audioconvert element for our scope
    // we do it here because we want pre-equalized and pre-volume samples
    // so that our visualization are not affected by them
    p = gst_element_get_pad (m_gst_audioconvert, "src");
    gst_pad_add_buffer_probe (p, G_CALLBACK(handoff_cb), this);
    gst_object_unref (p);

    /* link elements */
    gst_element_link_many( m_gst_audioconvert, /*m_gst_equalizer,*/ m_gst_identity,
                           m_gst_volume, m_gst_audioscale, m_gst_audiosink, NULL );

    gst_bin_add( GST_BIN(m_gst_pipeline), m_gst_audiobin);
    //gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(m_gst_pipeline)), bus_cb, NULL);
    gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(m_gst_pipeline)), bus_cb, NULL);

    m_pipelineFilled = true;
    return true;
}


void
GstEngine::destroyPipeline()
{
    m_fadeValue = 0.0;

    clearScopeQ();

    if ( m_pipelineFilled ) {
        // Remove the event handler while we destroy the pipeline
        GstPad *p = gst_element_get_pad (m_gst_decodebin, "sink");
        if (p)
          gst_pad_remove_event_probe(p, event_cb_id_);

        gst_element_set_state( m_gst_pipeline, GST_STATE_NULL );
        gst_object_unref( GST_OBJECT( m_gst_pipeline ) );

        m_pipelineFilled = false;
    }

}


bool
GstEngine::setupAudioCD( const QString& device, unsigned track, bool pause )
{
    qDebug() << "setupAudioCD: device = " << device << ", track = " << track << ", pause = " << pause ;
    bool filled = m_pipelineFilled && m_gst_src && strcmp( gst_element_get_name( m_gst_src ), "cdiocddasrc" ) == 0;
    if ( filled || createPipeline() )
    {
        if ( filled || ( m_gst_src = createElement( "cdiocddasrc", m_gst_pipeline, "cdiocddasrc" ) ) )
        {
            // TODO: allow user to configure default device rather than falling back to gstreamer default when no device passed in
            if ( !device.isNull() )
                g_object_set( G_OBJECT(m_gst_src), "device", device.toAscii().constData(), NULL );
            if ( track )
                g_object_set (G_OBJECT (m_gst_src), "track", track, NULL);
            if ( filled || gst_element_link( m_gst_src, m_gst_audiobin ) )
            {
                // the doco says we should only have to go to READY to read metadata, but that doesn't actually work
                if ( gst_element_set_state( m_gst_pipeline, pause ? GST_STATE_PAUSED : GST_STATE_READY ) != GST_STATE_CHANGE_FAILURE && gst_element_get_state( m_gst_pipeline, NULL, NULL, GST_CLOCK_TIME_NONE ) == GST_STATE_CHANGE_SUCCESS )
                {
                    return true;
                }
            }
        }
        destroyPipeline();
    }
    return false;
}


//#ifdef GST_KIOSTREAMS
void
GstEngine::sendBufferStatus()
{
    if ( m_streamBuffering ) {
        const int percent = (int) ( (float) m_streamBufIndex / STREAMBUF_MIN * 105.0 );
        emit statusText( QString("Buffering.. %1%" ).arg( MIN( percent, 100 ) ) );
    }
}
//#endif

gint64 GstEngine::pruneScope()
{
    if ( !m_pipelineFilled ) return 0; // don't prune if we aren't playing

    // get the position playing in the audio device
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 pos = 0;
    gst_element_query_position( m_gst_pipeline, &fmt, &pos );

    GstBuffer *buf = 0;
    guint64 stime, etime, dur;

    // free up the buffers that the audio device has advanced past already
    do
    {
       // most delayed buffers are at the head of the queue
       buf = reinterpret_cast<GstBuffer *>( g_queue_peek_head(m_delayq) );
       if (buf)
       {
          // the start time of the buffer
          stime = static_cast<guint64>( GST_BUFFER_TIMESTAMP( buf ) );
          // the duration of the buffer
          dur = static_cast<guint64>( GST_BUFFER_DURATION( buf ) );
          // therefore we can calculate the end time of the buffer
          etime = stime + dur;

          // purge this buffer if the pos is past the end time of the buffer
          if (static_cast<guint64>(pos) > etime)
          {
             g_queue_pop_head(m_delayq);
             gst_buffer_unref(buf);
          }
       }
    } while (buf && static_cast<guint64>(pos) > etime);

    return pos;
}

void GstEngine::clearScopeQ()
{
   GstBuffer *buf;

   // just free them all
   while (g_queue_get_length(m_delayq))
   {
      buf = reinterpret_cast<GstBuffer *>( g_queue_pop_head(m_delayq) );
      gst_buffer_unref(buf);
   }
}
