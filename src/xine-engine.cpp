/***************************************************************************
 *   Copyright (C) 2005   Christophe Thommeret <hftom@free.fr>             *
 *             (C) 2005   Ian Monroe <ian@monroe.nu>                       *
 *             (C) 2005,6 Mark Kretschmann <markey@web.de>                 *
 *             (C) 2004,5 Max Howell <max.howell@methylblue.com>           *
 *             (C) 2003,4 J. Kofler <kaffeine@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define DEBUG_PREFIX "xine-engine"

#include "xine-engine.h"
#include "xine-scope.h"

#include <climits>
#include <cstdlib>
#include <cmath>

#include <QtDebug>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>

extern "C"
{
    #include <unistd.h>
}

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif


//define this to use xine in a more standard way
//#define XINE_SAFE_MODE


///some logging static globals
namespace Log
{
    static uint bufferCount = 0;
    static uint scopeCallCount = 1; //prevent divideByZero
    static uint noSuitableBuffer = 0;
}

static Fader *s_fader = 0;
static OutFader *s_outfader = 0;


XineEngine::XineEngine()
        : EngineBase()
        , m_xine( 0 )
        , m_stream( 0 )
        , m_audioPort( 0 )
        , m_eventQueue( 0 )
        , m_post( 0 )
        , m_preamp( 1.0 )
        , m_stopFader( false )
        , m_fadeOutRunning ( false )
        , m_equalizerEnabled( false )
{
  m_settings.beginGroup(kSettingsGroup);
  reloadSettings();
}

XineEngine::~XineEngine()
{
    // Wait until the fader thread is done
    if( s_fader ) {
        m_stopFader = true;
        s_fader->resume(); // safety call if the engine is in the pause state
        s_fader->wait();
    }

    delete s_fader;
    delete s_outfader;

    if( m_fadeoutOnExit ) {
        bool terminateFader = false;
        fadeOut( m_fadeoutDuration, &terminateFader, true ); // true == exiting
    }

    //if( m_xine )       xine_config_save( m_xine, configPath() );

    if( m_stream )     xine_close( m_stream );
    if( m_eventQueue ) xine_event_dispose_queue( m_eventQueue );
    if( m_stream )     xine_dispose( m_stream );
    if( m_audioPort )  xine_close_audio_driver( m_xine, m_audioPort );
    if( m_post )       xine_post_dispose( m_xine, m_post );
    if( m_xine )       xine_exit( m_xine );

    qDebug() << "xine closed";

    qDebug() << "Scope statistics:";
    qDebug() << "  Average list size: " << Log::bufferCount / Log::scopeCallCount;
    qDebug() << "  Buffer failure:    " << double(Log::noSuitableBuffer*100) / Log::scopeCallCount << "%";
}

void XineEngine::reloadSettings() {
  m_currentAudioPlugin = m_settings.value("XineAudioOutput", "auto").toString();
  m_fadeoutEnabled = m_settings.value("FadeoutEnabled", true).toBool();
  m_fadeoutOnExit = m_settings.value("FadeoutOnExit", true).toBool();
  m_fadeoutDuration = m_settings.value("FadeoutDuration", 2000).toInt();
  m_crossfadeEnabled = m_settings.value("CrossfadeEnabled", true).toBool();
}

bool
XineEngine::init()
{
   qDebug() << "'Bringing joy to small mexican gerbils, a few weeks at a time.'";

#ifdef Q_OS_WIN32
   putenv(QString("XINE_PLUGIN_PATH=" +
      QCoreApplication::applicationDirPath() + "/xine/plugins").toAscii().constData());
#endif  // Q_OS_WIN32

#ifdef Q_OS_DARWIN
   setenv("XINE_PLUGIN_PATH", QString(QCoreApplication::applicationDirPath() + "/../PlugIns/xine").toAscii().constData(), 1);
#endif // Q_OS_DARWIN

   QMutexLocker l(&m_initMutex);

   m_xine = xine_new();

   if( !m_xine ) {
      emit error("Could not initialize xine.");
      return false;
   }

   #ifdef XINE_SAFE_MODE
   xine_engine_set_param( m_xine, XINE_ENGINE_PARAM_VERBOSITY, 99 );
   #endif

   //xine_config_load( m_xine, configPath() );
   //debug() << "w00t" << configPath() << endl;

   xine_init( m_xine );

   makeNewStream();

   #ifndef XINE_SAFE_MODE
   startTimer( 200 ); //prunes the scope
   #endif

   return true;
}

bool
XineEngine::makeNewStream()
{
   m_audioPort = xine_open_audio_driver( m_xine, m_currentAudioPlugin.toLocal8Bit().constData(), NULL );
   if( !m_audioPort ) {
      //TODO make engine method that is the same but parents the dialog for us
      emit error("xine was unable to initialize any audio drivers.");
      return false;
   }

   m_stream = xine_stream_new( m_xine, m_audioPort, NULL );
   if( !m_stream ) {
      xine_close_audio_driver( m_xine, m_audioPort );
      m_audioPort = NULL;
      emit error("Could not create a new xine stream");
      return false;
   }

   if( m_eventQueue )
      xine_event_dispose_queue( m_eventQueue );

   xine_event_create_listener_thread(
         m_eventQueue = xine_event_new_queue( m_stream ),
         &XineEngine::XineEventListener,
         (void*)this );

   #ifndef XINE_SAFE_MODE
   //implemented in xine-scope.h
   m_post = scope_plugin_new( m_xine, m_audioPort );

   xine_set_param( m_stream, XINE_PARAM_METRONOM_PREBUFFER, 6000 );
   xine_set_param( m_stream, XINE_PARAM_IGNORE_VIDEO, 1 );
   #endif
#ifdef XINE_PARAM_EARLY_FINISHED_EVENT
    if ( xine_check_version(1,1,1) && !(m_xfadeLength > 0) ) {
        // enable gapless playback
        qDebug() << "gapless playback enabled.";
        //xine_set_param(m_stream, XINE_PARAM_EARLY_FINISHED_EVENT, 1 );
    }
#endif
   return true;
}

// Makes sure an audio port and a stream exist.
bool
XineEngine::ensureStream()
{
   if( !m_stream )
      return makeNewStream();

   return true;
}

bool
XineEngine::load( const QUrl &url, bool isStream )
{
    if( !ensureStream() )
        return false;

    Engine::Base::load( url, isStream );

    if( s_outfader ) {
        s_outfader->finish();
        delete s_outfader;
    }

    if( m_xfadeLength > 0 && xine_get_status( m_stream ) == XINE_STATUS_PLAY &&
         url.scheme().toLower() == "file" &&
         xine_get_param( m_stream, XINE_PARAM_SPEED ) != XINE_SPEED_PAUSE &&
        ( m_xfadeNextTrack || //set by engine controller when switching tracks automatically
         m_crossfadeEnabled))
    {
       m_xfadeNextTrack = false;
       // Stop a probably running fader
       if( s_fader ) {
           m_stopFader = true;
           s_fader->finish(); // makes the fader stop abruptly
           delete s_fader;
       }
       s_fader = new Fader( this, m_xfadeLength );
       setEqualizerParameters( m_intPreamp, m_equalizerGains );
    }

   // for users who stubbonly refuse to use DMIX or buy a good soundcard
   // why doesn't xine do this? I cannot say.
   xine_close( m_stream );

   qDebug() << "Before xine_open() *****";

   if( xine_open( m_stream, QFile::encodeName( url.toString() ) ) )
   {
      qDebug() << "After xine_open() *****";

      #ifndef XINE_SAFE_MODE
      //we must ensure the scope is pruned of old buffers
      timerEvent( 0 );

      xine_post_out_t *source = xine_get_audio_source( m_stream );
      xine_post_in_t  *target = (xine_post_in_t*)xine_post_input( m_post, const_cast<char*>("audio in") );
      xine_post_wire( source, target );
      #endif

      playlistChanged();

      return true;
   }
   else
   {
      #ifdef XINE_PARAM_GAPLESS_SWITCH
        if ( xine_check_version(1,1,1) && !(m_xfadeLength > 0) )
            xine_set_param( m_stream, XINE_PARAM_GAPLESS_SWITCH, 0);
      #endif
   }

   // FAILURE to load!
   //s_fader will delete itself
   determineAndShowErrorMessage();

   return false;
}

bool
XineEngine::play( uint offset )
{
    if( !ensureStream() )
        return false;

    const bool has_audio     = xine_get_stream_info( m_stream, XINE_STREAM_INFO_HAS_AUDIO );
    const bool audio_handled = xine_get_stream_info( m_stream, XINE_STREAM_INFO_AUDIO_HANDLED );

    if (has_audio && audio_handled && xine_play( m_stream, 0, offset ))
    {
        if( s_fader )
           s_fader->start( QThread::LowestPriority );

        emit stateChanged( Engine::Playing );

        return true;
    }

    //we need to stop the track that is prepped for crossfade
    delete s_fader;

    emit stateChanged( Engine::Empty );

    determineAndShowErrorMessage();

    xine_close( m_stream );

    return false;
}

void
XineEngine::determineAndShowErrorMessage()
{
    QString body;

    qDebug() << "xine_get_error()";
    switch (xine_get_error( m_stream )) {
        case XINE_ERROR_NO_INPUT_PLUGIN:
            body = "No suitable input plugin. This often means that the url's protocol is not supported. Network failures are other possible causes.";
        break;

        case XINE_ERROR_NO_DEMUX_PLUGIN:
            body = "No suitable demux plugin. This often means that the file format is not supported.";
        break;

        case XINE_ERROR_DEMUX_FAILED:
            body = "Demuxing failed.";
        break;

        case XINE_ERROR_INPUT_FAILED:
            body = "Could not open file.";
        break;

        case XINE_ERROR_MALFORMED_MRL:
            body = "The location is malformed.";
        break;

        case XINE_ERROR_NONE:
            // xine is thick. xine doesn't think there is an error
            // but there may be! We check for other errors below.

        default:
            if (!xine_get_stream_info( m_stream, XINE_STREAM_INFO_AUDIO_HANDLED ))
            {
                // xine can read the plugin but it didn't find any codec
                // THUS xine=daft for telling us it could handle the format in canDecode!
                body = "There is no available decoder.";
                QString const ext = QFileInfo(m_url.path()).completeSuffix();
                // TODO
                //if (ext == "mp3" && EngineController::installDistroCodec( "xine-engine" ))
                //    return;
            }
            else if (!xine_get_stream_info( m_stream, XINE_STREAM_INFO_HAS_AUDIO ))
                body = "There is no audio channel!";
        break;
    }

    // TODO
    /*Amarok::StatusBar::instance()->longMessage(
            "<b>" + i18n("Error Loading Media") + "</b><p>" + body + "<p>" + m_url.prettyURL(),
            KDE::StatusBar::Error );*/
}

void
XineEngine::stop()
{
    if( s_fader && s_fader->isRunning())
        s_fader->resume(); // safety call if the engine is in the pause state

    if ( !m_stream )
       return;

    if(  (m_fadeoutEnabled && !m_fadeOutRunning) || state() == Engine::Paused )
    {
        s_outfader = new OutFader( this, m_fadeoutDuration );
        s_outfader->start();
        ::usleep( 100 ); //to be sure engine state won't be changed before it is checked in fadeOut()
        m_url = QUrl(); //to ensure we return Empty from state()

        std::fill( m_scope.begin(), m_scope.end(), 0 );
    }
    else if( !m_fadeOutRunning )
    {
        xine_stop( m_stream );
        xine_close( m_stream );
        xine_set_param( m_stream, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);
    }

    emit stateChanged( Engine::Empty );
}

void
XineEngine::pause()
{
    if ( !m_stream )
        return;

    if( xine_get_param( m_stream, XINE_PARAM_SPEED ) != XINE_SPEED_PAUSE )
    {
        if( s_fader && s_fader->isRunning() )
            s_fader->pause();

        xine_set_param( m_stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
        xine_set_param( m_stream, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);
        emit stateChanged( Engine::Paused );

    }
}

void
XineEngine::unpause()
{
    if ( !m_stream )
        return;
    
    if( xine_get_param( m_stream, XINE_PARAM_SPEED ) == XINE_SPEED_PAUSE )
    {
        if( s_fader && s_fader->isRunning() )
            s_fader->resume();

        xine_set_param( m_stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL );
        emit stateChanged( Engine::Playing );
    }
}

Engine::State
XineEngine::state() const
{
    if ( !m_stream || m_fadeOutRunning )
       return Engine::Empty;

    switch( xine_get_status( m_stream ) )
    {
    case XINE_STATUS_PLAY: return xine_get_param( m_stream, XINE_PARAM_SPEED )  != XINE_SPEED_PAUSE ? Engine::Playing : Engine::Paused;
    case XINE_STATUS_IDLE: return Engine::Empty;
    case XINE_STATUS_STOP:
    default:               return m_url.isEmpty() ? Engine::Empty : Engine::Idle;
    }
}

uint
XineEngine::position() const
{
    if ( state() == Engine::Empty )
       return 0;

    int pos;
    int time = 0;
    int length;

    // Workaround for problems when you seek too quickly, see BUG 99808
    int tmp = 0, i = 0;
    while( ++i < 4 )
    {
        xine_get_pos_length( m_stream, &pos, &time, &length );
        if( time > tmp ) break;
        usleep( 100000 );
    }
			
    // Here we check for new metadata periodically, because xine does not emit an event
    // in all cases (e.g. with ogg streams). See BUG 122505
    if ( state() != Engine::Idle && state() != Engine::Empty )
    {
        const Engine::SimpleMetaBundle bundle = fetchMetaData();
        if( bundle.title != m_currentBundle.title || bundle.artist != m_currentBundle.artist ) {
            qDebug() << "Metadata received.";
            m_currentBundle = bundle;

            XineEngine* p = const_cast<XineEngine*>( this );
            p->emit metaData( bundle );
        }
    }

    return time;
}

uint
XineEngine::length() const
{
    if ( !m_stream )
       return 0;

    // xine often delivers nonsense values for VBR files and such, so we only
    // use the length for remote files

    if( m_url.scheme().toLower() == "file" )
        return 0;

    else {
        int pos;
        int time;
        int length = 0;

        xine_get_pos_length( m_stream, &pos, &time, &length );
        if( length < 0 )
            length=0;
	    	
        return length;
    }
}

void
XineEngine::seek( uint ms )
{
    if( !ensureStream() )
        return;

    if( xine_get_param( m_stream, XINE_PARAM_SPEED ) == XINE_SPEED_PAUSE ) {
        // FIXME this is a xine API issue really, they need to add a seek function
        xine_play( m_stream, 0, (int)ms );
        xine_set_param( m_stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
    }
    else
        xine_play( m_stream, 0, (int)ms );
}

void
XineEngine::setVolumeSW( uint vol )
{
    if ( !m_stream )
       return;
    if( !s_fader )
        xine_set_param( m_stream, XINE_PARAM_AUDIO_AMP_LEVEL, static_cast<uint>( vol * m_preamp ) );
}

void
XineEngine::fadeOut( uint fadeLength, bool* terminate, bool exiting )
{
    if( m_fadeOutRunning ) //Let us not start another fadeout...
        return;

    m_fadeOutRunning = !m_fadeOutRunning;
    const bool isPlaying = m_stream && ( xine_get_status( m_stream ) == XINE_STATUS_PLAY );
    const float originalVol = Engine::Base::makeVolumeLogarithmic( m_volume ) * m_preamp;

    // On shutdown, limit fadeout to 3 secs max, so that we don't risk getting killed
    const int length = exiting ? qMin( fadeLength, 3000u ) : fadeLength;

    if( length > 0 && isPlaying )
    {
        // fader-class doesn't work in this spot as is, so some parts need to be copied here... (ugly)
        uint stepsCount = length < 1000 ? length / 10 : 100;
        uint stepSizeUs = (int)( 1000.0 * (float)length / (float)stepsCount );

        ::usleep( stepSizeUs );
        QTime t;
        t.start();
        float mix = 0.0;
        while ( mix < 1.0 )
        {
            if( *terminate ) break;

            ::usleep( stepSizeUs );
            float vol = Engine::Base::makeVolumeLogarithmic( m_volume ) * m_preamp;
            float mix = (float)t.elapsed() / (float)length;
            if ( mix > 1.0 )
            {
                break;
            }
            if ( m_stream )
            {
                float v = 4.0 * (1.0 - mix) / 3.0;
                xine_set_param( m_stream, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)( v < 1.0 ? vol * v : vol ) );
            }
        }
    }
    if( m_fadeOutRunning && m_stream )
        xine_set_param( m_stream, XINE_PARAM_AUDIO_AMP_LEVEL, (uint) originalVol );
    m_fadeOutRunning = !m_fadeOutRunning;
}

void
XineEngine::setEqualizerEnabled( bool enable )
{
    if ( !m_stream )
       return;

    m_equalizerEnabled = enable;

    if( !enable ) {
        QList<int> gains;
        for( uint x = 0; x < 10; x++ )
            gains << -101; // sets eq gains to zero.

        setEqualizerParameters( 0, gains );
   }
}

/*
 sets the eq params for xine engine - have to rescale eq params to fitting range (adapted from kaffeine and xfmedia)

 preamp
   pre: (-100..100)
   post: (0.1..1.9) - this is not really a preamp but we use the xine preamp parameter for our normal volume. so we make a postamp.

 gains
   pre: (-100..100)
   post: (1..200) - (1 = down, 100 = middle, 200 = up, 0 = off)
 */
void
XineEngine::setEqualizerParameters( int preamp, const QList<int> &gains )
{
    if ( !m_stream )
        return;

    m_equalizerGains = gains;
    m_intPreamp = preamp;
    QList<int>::ConstIterator it = gains.begin();

    xine_set_param( m_stream, XINE_PARAM_EQ_30HZ,    int( (*it  )*0.995 + 100 ) );
    xine_set_param( m_stream, XINE_PARAM_EQ_60HZ,    int( (*++it)*0.995 + 100 ) );
    xine_set_param( m_stream, XINE_PARAM_EQ_125HZ,   int( (*++it)*0.995 + 100 ) );
    xine_set_param( m_stream, XINE_PARAM_EQ_250HZ,   int( (*++it)*0.995 + 100 ) );
    xine_set_param( m_stream, XINE_PARAM_EQ_500HZ,   int( (*++it)*0.995 + 100 ) );
    xine_set_param( m_stream, XINE_PARAM_EQ_1000HZ,  int( (*++it)*0.995 + 100 ) );
    xine_set_param( m_stream, XINE_PARAM_EQ_2000HZ,  int( (*++it)*0.995 + 100 ) );
    xine_set_param( m_stream, XINE_PARAM_EQ_4000HZ,  int( (*++it)*0.995 + 100 ) );
    xine_set_param( m_stream, XINE_PARAM_EQ_8000HZ,  int( (*++it)*0.995 + 100 ) );
    xine_set_param( m_stream, XINE_PARAM_EQ_16000HZ, int( (*++it)*0.995 + 100 ) );

    m_preamp = ( preamp - 0.1 * preamp + 100 ) / 100.0;
    setVolume( m_volume );
}

bool
XineEngine::canDecode( const QUrl &url ) const
{
    static QStringList list;
    if(list.isEmpty())
    {
        QMutexLocker l(&const_cast<XineEngine*>(this)->m_initMutex);

        if (list.isEmpty()) {
            char* exts = xine_get_file_extensions( m_xine );
            list = QString(exts).split(' ');
            free( exts ); exts = NULL;
            //images
            list.removeAll("png");
            list.removeAll("jpg");
            list.removeAll("jpeg");
            list.removeAll("gif");
            list.removeAll("ilbm");
            list.removeAll("iff");
            //subtitles
            list.removeAll("asc");
            list.removeAll("txt");
            list.removeAll("sub");
            list.removeAll("srt");
            list.removeAll("smi");
            list.removeAll("ssa");
    //HACK we also check for m4a because xine plays them but
    //for some reason doesn't return the extension
            if(!list.contains("m4a"))
                list << "m4a";
        }
    }

    if (url.scheme() == "cdda")
        // play audio CDs pls
        return true;

    QString path = url.path();

    // partial downloads from Konqi and other browsers
    // tend to have a .part extension
    if (path.endsWith( ".part" ))
        path = path.left( path.length() - 5 );

    const QString ext = path.mid( path.lastIndexOf( '.' ) + 1 ).toLower();

    return list.contains( ext );
}

const Engine::Scope&
XineEngine::scope()
{
    if( !m_post || !m_stream || xine_get_status( m_stream ) != XINE_STATUS_PLAY )
       return m_scope;

    MyNode* const myList         = scope_plugin_list( m_post );
    metronom_t* const myMetronom = scope_plugin_metronom( m_post );
    const int myChannels         = scope_plugin_channels( m_post );
    int scopeidx = 0;

    if (myChannels > 2)
       return m_scope;

    //prune the buffer list and update m_currentVpts
    timerEvent( 0 );

    for( int n, frame = 0; frame < 512; )
    {
        MyNode *best_node = 0;

        for( MyNode *node = myList->next; node != myList; node = node->next, Log::bufferCount++ )
            if( node->vpts <= m_currentVpts && (!best_node || node->vpts > best_node->vpts) )
               best_node = node;

        if( !best_node || best_node->vpts_end < m_currentVpts ) {
           Log::noSuitableBuffer++; break; }

        int64_t
        diff  = m_currentVpts;
        diff -= best_node->vpts;
        diff *= 1<<16;
        diff /= myMetronom->pts_per_smpls;

        const int16_t*
        data16  = best_node->mem;
        data16 += diff;

        diff += diff % myChannels; //important correction to ensure we don't overflow the buffer
        diff /= myChannels;        //use units of frames, not samples

        //calculate the number of available samples in this buffer
        n  = best_node->num_frames;
        n -= diff;
        n += frame; //clipping for # of frames we need

        if( n > 512 )
           n = 512; //we don't want more than 512 frames

        for( int a, c; frame < n; ++frame, data16 += myChannels ) {
            for( a = c = 0; c < myChannels; ++c )
            {
               // we now give interleaved pcm to the scope
               m_scope[scopeidx++] = data16[c];
               if (myChannels == 1) // duplicate mono samples
                  m_scope[scopeidx++] = data16[c];
            }
        }

        m_currentVpts = best_node->vpts_end;
        m_currentVpts++; //FIXME needs to be done for some reason, or you get situations where it uses same buffer again and again
    }

    Log::scopeCallCount++;

    return m_scope;
}

void
XineEngine::timerEvent( QTimerEvent* )
{
   if ( !m_stream )
      return;

   //here we prune the buffer list regularly

   MyNode *myList = scope_plugin_list( m_post );

   if ( ! myList ) return;

   //we operate on a subset of the list for thread-safety
   MyNode * const first_node = myList->next;
   MyNode const * const list_end = myList;

   m_currentVpts = (xine_get_status( m_stream ) == XINE_STATUS_PLAY)
         ? xine_get_current_vpts( m_stream )
         : LLONG_MAX; //if state is not playing OR paused, empty the list
   //: std::numeric_limits<int64_t>::max(); //TODO don't support crappy gcc 2.95

   for( MyNode *prev = first_node, *node = first_node->next; node != list_end; node = node->next )
   {
      //we never delete first_node
      //this maintains thread-safety
      if( node->vpts_end < m_currentVpts ) {
         prev->next = node->next;

         free( node->mem );
         free( node );

         node = prev;
      }

      prev = node;
   }
}

bool
XineEngine::event( QEvent* e )
{
    #define message static_cast<QString*>(static_cast<XineEvent*>(e)->data())

    switch( e->type() )
    {
    case XineEvent::PlaybackFinished: //XINE_EVENT_UI_PLAYBACK_FINISHED
        emit trackEnded();
        return true;

    case XineEvent::InfoMessage:
        emit infoMessage( (*message).arg( m_url.toString() ) );
        delete message;
        return true;

    case XineEvent::StatusMessage:
        emit statusText( *message );
        delete message;
        return true;

    case XineEvent::MetaInfoChanged: { //meta info has changed
        qDebug() << "Metadata received.";
        const Engine::SimpleMetaBundle bundle = fetchMetaData();
        m_currentBundle = bundle;
        emit metaData( bundle );
    }   return true;

    case XineEvent::Redirecting:
        emit statusText( QString("Redirecting to: ").arg( *message ) );
        load( QUrl( *message ), false );
        play();
        delete message;
        return true;
    case XineEvent::LastFMTrackChanged:
        emit lastFmTrackChange();
        return true;
    default:
        ;
    }

    #undef message
    return false;
}

//SLOT
void
XineEngine::playlistChanged()
{
  // TODO
    /*#ifdef XINE_PARAM_EARLY_FINISHED_EVENT
    #ifdef XINE_PARAM_GAPLESS_SWITCH
    if ( xine_check_version(1,1,1) && !(m_xfadeLength > 0) 
         && m_url.isLocalFile() && Playlist::instance()->isTrackAfter() )
    {
        xine_set_param(m_stream, XINE_PARAM_EARLY_FINISHED_EVENT, 1 );
        debug() << "XINE_PARAM_EARLY_FINISHED_EVENT enabled" << endl;
    }
    else
    {
        //we don't want an early finish event if there is no track after the current one
        xine_set_param(m_stream, XINE_PARAM_EARLY_FINISHED_EVENT, 0 );
        debug() << "XINE_PARAM_EARLY_FINISHED_EVENT disabled" << endl;
    }
    #endif
    #endif*/
}

static time_t last_error_time = 0; // hysteresis on xine errors
static int    last_error = XINE_MSG_NO_ERROR;

void
XineEngine::XineEventListener( void *p, const xine_event_t* xineEvent )
{
    time_t current;

    if( !p ) return;

    #define xe static_cast<XineEngine*>(p)

    switch( xineEvent->type )
    {
    case XINE_EVENT_UI_SET_TITLE:

        qDebug() << "XINE_EVENT_UI_SET_TITLE";

        QApplication::postEvent( xe, new XineEvent( XineEvent::MetaInfoChanged ) );

        break;

    case XINE_EVENT_UI_PLAYBACK_FINISHED:
        qDebug() << "XINE_EVENT_UI_PLAYBACK_FINISHED";

        #ifdef XINE_PARAM_GAPLESS_SWITCH
        // TODO
            /*if ( xine_check_version(1,1,1) && xe->m_url.isLocalFile() //Remote media break with gapless
            //don't prepare for a track that isn't coming
            && Playlist::instance()
            && Playlist::instance()->isTrackAfter()
            && !AmarokConfig::crossfade() )
                xine_set_param( xe->m_stream, XINE_PARAM_GAPLESS_SWITCH, 1);*/
        #endif
        //emit signal from GUI thread
        QApplication::postEvent( xe, new XineEvent(XineEvent::PlaybackFinished) );
        break;

    case XINE_EVENT_PROGRESS: {
        xine_progress_data_t* pd = (xine_progress_data_t*)xineEvent->data;

        QString
        msg = "%1 %2%";
        msg = msg.arg( QString::fromUtf8( pd->description ) )
              .arg( QString::number(pd->percent) + QLocale::system().percent() );

        XineEvent *e = new XineEvent( XineEvent::StatusMessage );
        e->setData( new QString( msg ) );

        QApplication::postEvent( xe, e );

    }   break;

    case XINE_EVENT_MRL_REFERENCE: {
        /// xine has read the stream and found it actually links to something else
        /// so we need to play that instead

        QString message = QString::fromUtf8( static_cast<xine_mrl_reference_data_t*>(xineEvent->data)->mrl );
        XineEvent *e = new XineEvent( XineEvent::Redirecting );
        e->setData( new QString( message ) );

        QApplication::postEvent( xe, e );

    }   break;

    case XINE_EVENT_UI_MESSAGE:
    {
        qDebug() << "message received from xine";

        xine_ui_message_data_t *data = (xine_ui_message_data_t *)xineEvent->data;
        QString message;

        switch( data->type )
        {
        case XINE_MSG_NO_ERROR:
        {
            //series of \0 separated strings, terminated with a \0\0
            char str[2000];
            char *p = str;
            for( char *msg = data->messages; !(*msg == '\0' && *(msg+1) == '\0'); ++msg, ++p )
                *p = *msg == '\0' ? '\n' : *msg;
            *p = '\0';

            qDebug() << str;

            break;
        }

        case XINE_MSG_ENCRYPTED_SOURCE:
            break;

        case XINE_MSG_UNKNOWN_HOST:
            message = "The host is unknown for the URL: <i>%1</i>"; goto param;
        case XINE_MSG_UNKNOWN_DEVICE:
            message = "The device name you specified seems invalid."; goto param;
        case XINE_MSG_NETWORK_UNREACHABLE:
            message = "The network appears unreachable."; goto param;
        case XINE_MSG_AUDIO_OUT_UNAVAILABLE:
            message = "Audio output unavailable; the device is busy."; goto param;
        case XINE_MSG_CONNECTION_REFUSED:
            message = "The connection was refused for the URL: <i>%1</i>"; goto param;
        case XINE_MSG_FILE_NOT_FOUND:
            message = "xine could not find the URL: <i>%1</i>"; goto param;
        case XINE_MSG_PERMISSION_ERROR:
            message = "Access was denied for the URL: <i>%1</i>"; goto param;
        case XINE_MSG_READ_ERROR:
            message = "The source cannot be read for the URL: <i>%1</i>"; goto param;
        case XINE_MSG_LIBRARY_LOAD_ERROR:
            message = "A problem occurred while loading a library or decoder."; goto param;

        case XINE_MSG_GENERAL_WARNING:
            message = "General Warning"; goto explain;
        case XINE_MSG_SECURITY:
            message = "Security Warning"; goto explain;
        default:
            message = "Unknown Error"; goto explain;


        explain:

            // Don't flood the user with error messages
            if( (last_error_time + 10) > time( &current ) &&
                   data->type == last_error )
            {
                last_error_time = current;
                return;
            }
            last_error_time = current;
            last_error = data->type;

            if( data->explanation )
            {
                message.prepend( "<b>" );
                message += "</b>:<p>";
                message += QString::fromUtf8( (char*)data + data->explanation );
            }
            else break; //if no explanation then why bother!

            //FALL THROUGH

        param:

            // Don't flood the user with error messages
            if((last_error_time + 10) > time(&current) &&
               data->type == last_error)
            {
                last_error_time = current;
                return;
            }
            last_error_time = current;
            last_error = data->type;

            message.prepend( "<p>" );
            message += "<p>";

            if(data->explanation)
            {
                message += "xine parameters: <i>";
                message += QString::fromUtf8( (char*)data + data->parameters );
                message += "</i>";
            }
            else message += "Sorry, no additional information is available.";

            QApplication::postEvent( xe, new XineEvent(XineEvent::InfoMessage, new QString(message)) );
        }

    } //case
    case XINE_EVENT_UI_CHANNELS_CHANGED: //Flameeyes used this for last.fm track changes
      QApplication::postEvent( xe, new XineEvent(XineEvent::LastFMTrackChanged) );
    break;
    } //switch

    #undef xe
}

Engine::SimpleMetaBundle
XineEngine::fetchMetaData() const
{
    Engine::SimpleMetaBundle bundle;
    bundle.title      = QString::fromUtf8( xine_get_meta_info( m_stream, XINE_META_INFO_TITLE ) );
    bundle.artist     = QString::fromUtf8( xine_get_meta_info( m_stream, XINE_META_INFO_ARTIST ) );
    bundle.album      = QString::fromUtf8( xine_get_meta_info( m_stream, XINE_META_INFO_ALBUM ) );
    bundle.comment    = QString::fromUtf8( xine_get_meta_info( m_stream, XINE_META_INFO_COMMENT ) );
    bundle.genre      = QString::fromUtf8( xine_get_meta_info( m_stream, XINE_META_INFO_GENRE ) );
    bundle.bitrate    = QString::number( xine_get_stream_info( m_stream, XINE_STREAM_INFO_AUDIO_BITRATE ) / 1000 );
    bundle.samplerate = QString::number( xine_get_stream_info( m_stream, XINE_STREAM_INFO_AUDIO_SAMPLERATE ) );
    bundle.year       = QString::fromUtf8( xine_get_meta_info( m_stream, XINE_META_INFO_YEAR ) );
    bundle.tracknr    = QString::fromUtf8( xine_get_meta_info( m_stream, XINE_META_INFO_TRACK_NUMBER ) );

    return bundle;
}

bool XineEngine::metaDataForUrl(const QUrl &url, Engine::SimpleMetaBundle &b)
{
    bool result = false;
    xine_stream_t* tmpstream = xine_stream_new(m_xine, NULL, NULL);
    if (xine_open(tmpstream, QFile::encodeName(url.toString()))) {
        QString audioCodec = QString::fromUtf8(xine_get_meta_info(tmpstream, XINE_META_INFO_SYSTEMLAYER));

        if (audioCodec == "CDDA") {
            QString title = QString::fromUtf8(
                xine_get_meta_info(tmpstream, XINE_META_INFO_TITLE));
            if ((!title.isNull()) && (!title.isEmpty())) { //no meta info
                b.title = title;
                b.artist =
                    QString::fromUtf8(
                        xine_get_meta_info(tmpstream, XINE_META_INFO_ARTIST));
                b.album =
                    QString::fromUtf8(
                        xine_get_meta_info(tmpstream, XINE_META_INFO_ALBUM));
                b.genre =
                    QString::fromUtf8(
                        xine_get_meta_info(tmpstream, XINE_META_INFO_GENRE));
                b.year =
                    QString::fromUtf8(
                        xine_get_meta_info(tmpstream, XINE_META_INFO_YEAR));
                b.tracknr =
                    QString::fromUtf8(
                        xine_get_meta_info(tmpstream, XINE_META_INFO_TRACK_NUMBER));
                if( b.tracknr.isEmpty() )
                    b.tracknr = QFileInfo(url.path()).fileName();
            } else {
                b.title = QString("Track %1").arg(QFileInfo(url.path()).fileName());
                b.album = "AudioCD";
            }
        }

        if (audioCodec == "CDDA" || audioCodec == "WAV") {
            result = true;
            int samplerate = xine_get_stream_info( tmpstream, XINE_STREAM_INFO_AUDIO_SAMPLERATE );

            // xine would provide a XINE_STREAM_INFO_AUDIO_BITRATE, but unfortunately not for CDDA or WAV
            // so we calculate the bitrate by our own
            int bitsPerSample = xine_get_stream_info( tmpstream, XINE_STREAM_INFO_AUDIO_BITS );
            int nbrChannels = xine_get_stream_info( tmpstream, XINE_STREAM_INFO_AUDIO_CHANNELS );
            int bitrate = (samplerate * bitsPerSample * nbrChannels) / 1000;

            b.bitrate = QString::number(bitrate);
            b.samplerate = QString::number(samplerate);
            int pos, time, length = 0;
            xine_get_pos_length(tmpstream, &pos, &time, &length);
            b.length = QString::number(length / 1000);
        }
        xine_close(tmpstream);
    }
    xine_dispose(tmpstream);
    return result;
}

bool XineEngine::getAudioCDContents(const QString &device, QList<QUrl> &urls)
{
    char **xine_urls = NULL;
    int num;
    int i = 0;

    if (!device.isNull()) {
        qDebug() << "xine-engine setting CD Device to: " << device;
        xine_cfg_entry_t config;
        if (!xine_config_lookup_entry(m_xine, "input.cdda_device", &config)) {
      emit statusText("Failed CD device lookup in xine engine");
	    return false;
	}
        config.str_value = (char *)device.toAscii().constData();
        xine_config_update_entry(m_xine, &config);
    }

    emit statusText("Getting AudioCD contents...");

    xine_urls = xine_get_autoplay_mrls(m_xine, "CD", &num);

    if (xine_urls) {
        while (xine_urls[i]) {
            urls << QUrl(xine_urls[i]);
            ++i;
        }
    }
    else emit statusText("Could not read AudioCD");

    return true;
}

bool XineEngine::flushBuffer()
{
    return false;
}

bool XineEngine::lastFmProxyRequired()
{
    return !( xine_check_version(1,1,9) );
}

//////////////////////////////////////////////////////////////////////////////
/// class Fader
//////////////////////////////////////////////////////////////////////////////

Fader::Fader( XineEngine *engine, uint fadeMs )
   : QThread(engine)
   , m_engine( engine )
   , m_xine( engine->m_xine )
   , m_decrease( engine->m_stream )
   , m_increase( 0 )
   , m_port( engine->m_audioPort )
   , m_post( engine->m_post )
   , m_fadeLength( fadeMs )
   , m_paused( false )
   , m_terminated( false )
{
    if( engine->makeNewStream() )
    {
        m_increase = engine->m_stream;

        xine_set_param( m_increase, XINE_PARAM_AUDIO_AMP_LEVEL, 0 );
    }
    else {
        s_fader = 0;
        deleteLater();
    }
}

Fader::~Fader()
{
     wait();

     xine_close( m_decrease );
     xine_dispose( m_decrease );
     xine_close_audio_driver( m_xine, m_port );
     if( m_post ) xine_post_dispose( m_xine, m_post );

     if( !m_engine->m_stopFader )
         m_engine->setVolume( m_engine->volume() );

     m_engine->m_stopFader = false;
     s_fader = 0;
}

void
Fader::run()
{
    // do a volume change in 100 steps (or every 10ms)
    uint stepsCount = m_fadeLength < 1000 ? m_fadeLength / 10 : 100;
    uint stepSizeUs = (int)( 1000.0 * (float)m_fadeLength / (float)stepsCount );

    float mix = 0.0;
    float elapsedUs = 0.0;
    while ( mix < 1.0 )
    {
        if ( m_terminated )
            break;
        // sleep a constant amount of time
        QThread::usleep( stepSizeUs );

        if ( m_paused )
        	continue;

        elapsedUs += stepSizeUs;

        // get volume (amarok main * equalizer preamp)
        float vol = Engine::Base::makeVolumeLogarithmic( m_engine->m_volume ) * m_engine->m_preamp;

        // compute the mix factor as the percentage of time spent since fade begun
        float mix = (elapsedUs / 1000.0) / (float)m_fadeLength;
        if ( mix > 1.0 )
        {
            if ( m_increase )
                xine_set_param( m_increase, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)vol );
            break;
        }

        // change volume of streams (using dj-like cross-fade profile)
        if ( m_decrease )
        {
            //xine_set_param( m_decrease, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)(vol * (1.0 - mix)) );  // linear
            float v = 4.0 * (1.0 - mix) / 3.0;
            xine_set_param( m_decrease, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)( v < 1.0 ? vol * v : vol ) );
        }
        if ( m_increase )
        {
            //xine_set_param( m_increase, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)(vol * mix) );  //linear
            float v = 4.0 * mix / 3.0;
            xine_set_param( m_increase, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)( v < 1.0 ? vol * v : vol ) );
        }
    }

    //stop using cpu!
    xine_stop( m_decrease );

    deleteLater();
}

void
Fader::pause()
{
	m_paused = true;
}

void
Fader::resume()
{
	m_paused = false;
}

void
Fader::finish()
{
	m_terminated = true;
}

//////////////////////////////////////////////////////////////////////////////
/// class OutFader
//////////////////////////////////////////////////////////////////////////////

OutFader::OutFader( XineEngine *engine, uint fadeLength )
   : QThread(engine)
   , m_engine( engine )
   , m_terminated( false )
   , m_fadeLength( fadeLength )
{
}

OutFader::~OutFader()
{
     wait();

     s_outfader = 0;
}

void
OutFader::run()
{
    m_engine->fadeOut( m_fadeLength, &m_terminated );

    xine_stop( m_engine->m_stream );
    xine_close( m_engine->m_stream );
    xine_set_param( m_engine->m_stream, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);

    deleteLater();
}

void
OutFader::finish()
{
	m_terminated = true;
}
