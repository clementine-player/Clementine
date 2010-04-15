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
#include <QLocale>
#include <QTimer>

extern "C"
{
#include <unistd.h>
}

#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif


//define this to use xine in a more standard way
#ifdef Q_OS_WIN32
#define XINE_SAFE_MODE
#endif


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
    , xine_( 0 )
    , stream_( 0 )
    , audioPort_( 0 )
    , eventQueue_( 0 )
    , post_( 0 )
    , preamp_( 1.0 )
    , stopFader_( false )
    , fadeOutRunning_ ( false )
    , equalizerEnabled_( false )
    , prune_(NULL)
{
  settings_.beginGroup(kSettingsGroup);
  reloadSettings();
}

XineEngine::~XineEngine()
{
  // Wait until the fader thread is done
  if( s_fader ) {
    stopFader_ = true;
    s_fader->resume(); // safety call if the engine is in the pause state
    s_fader->wait();
  }

  // Wait until the prune scope thread is done
  if (prune_) {
    prune_->exit();
    prune_->wait();
  }

  delete s_fader;
  delete s_outfader;
  delete prune_;

  if( fadeoutOnExit_ ) {
    bool terminateFader = false;
    fadeOut( fadeoutDuration_, &terminateFader, true ); // true == exiting
  }

  //if( xine_ )       xine_config_save( xine_, configPath() );

  if( stream_ )     xine_close( stream_ );
  if( eventQueue_ ) xine_event_dispose_queue( eventQueue_ );
  if( stream_ )     xine_dispose( stream_ );
  if( audioPort_ )  xine_close_audio_driver( xine_, audioPort_ );
  if( post_ )       xine_post_dispose( xine_, post_ );
  if( xine_ )       xine_exit( xine_ );

  qDebug() << "xine closed";

  qDebug() << "Scope statistics:";
  qDebug() << "  Average list size: " << Log::bufferCount / Log::scopeCallCount;
  qDebug() << "  Buffer failure:    " << double(Log::noSuitableBuffer*100) / Log::scopeCallCount << "%";
}

void XineEngine::reloadSettings() {
  currentAudioPlugin_ = settings_.value("XineAudioOutput", "auto").toString();
  fadeoutEnabled_ = settings_.value("FadeoutEnabled", true).toBool();
  fadeoutOnExit_ = settings_.value("FadeoutOnExit", true).toBool();
  fadeoutDuration_ = settings_.value("FadeoutDuration", 2000).toInt();
  crossfadeEnabled_ = settings_.value("CrossfadeEnabled", true).toBool();
}

bool
XineEngine::Init()
{
  qDebug() << "'Bringing joy to small mexican gerbils, a few weeks at a time.'";

#ifdef Q_OS_WIN32
  putenv(QString("XINE_PLUGIN_PATH=" + QCoreApplication::applicationDirPath() + "/xine/plugins").toAscii().constData());
#endif  // Q_OS_WIN32

#ifdef Q_OS_DARWIN
  setenv("XINE_PLUGIN_PATH", QString(QCoreApplication::applicationDirPath() + "/../PlugIns/xine").toAscii().constData(), 1);
#endif  // Q_OS_DARWIN

  QMutexLocker l(&initMutex_);

  xine_ = xine_new();

  if( !xine_ ) {
    emit Error("Could not initialize xine.");
    return false;
  }

#ifdef XINE_SAFE_MODE
  xine_engine_set_param( xine_, XINE_ENGINE_PARAM_VERBOSITY, 99 );
#endif

  //xine_config_load( xine_, configPath() );
  //debug() << "w00t" << configPath() << endl;

  xine_init( xine_ );

  makeNewStream();

#ifndef XINE_SAFE_MODE
  prune_ = new PruneScopeThread(this);
  prune_->start();
#endif

  return true;
}

bool
XineEngine::makeNewStream()
{
  audioPort_ = xine_open_audio_driver( xine_, currentAudioPlugin_.toLocal8Bit().constData(), NULL );
  if( !audioPort_ ) {
    //TODO make engine method that is the same but parents the dialog for us
    emit Error("xine was unable to initialize any audio drivers.");
    return false;
  }

  stream_ = xine_stream_new( xine_, audioPort_, NULL );
  if( !stream_ ) {
    xine_close_audio_driver( xine_, audioPort_ );
    audioPort_ = NULL;
    emit Error("Could not create a new xine stream");
    return false;
  }

  if( eventQueue_ )
    xine_event_dispose_queue( eventQueue_ );

  xine_event_create_listener_thread(
      eventQueue_ = xine_event_new_queue( stream_ ),
      &XineEngine::XineEventListener,
      (void*)this );

#ifndef XINE_SAFE_MODE
  //implemented in xine-scope.h
  post_ = scope_plugin_new( xine_, audioPort_ );

  xine_set_param( stream_, XINE_PARAM_METRONOM_PREBUFFER, 6000 );
  xine_set_param( stream_, XINE_PARAM_IGNORE_VIDEO, 1 );
#endif
#ifdef XINE_PARAM_EARLY_FINISHED_EVENT
  if ( xine_check_version(1,1,1) && !(xfadeLength_ > 0) ) {
    // enable gapless playback
    qDebug() << "gapless playback enabled.";
    //xine_set_param(stream_, XINE_PARAM_EARLY_FINISHED_EVENT, 1 );
  }
#endif
  return true;
}

// Makes sure an audio port and a stream exist.
bool
XineEngine::ensureStream()
{
  if( !stream_ )
    return makeNewStream();

  return true;
}

bool
XineEngine::Load( const QUrl &url ,Engine::TrackChangeType change )
{
  if( !ensureStream() )
    return false;

  Engine::Base::Load( url, change );

  if( s_outfader ) {
    s_outfader->finish();
    delete s_outfader;
  }

  if( xfadeLength_ > 0 && xine_get_status( stream_ ) == XINE_STATUS_PLAY &&
      url.scheme().toLower() == "file" &&
      xine_get_param( stream_, XINE_PARAM_SPEED ) != XINE_SPEED_PAUSE &&
      ( xfadeNextTrack_ || //set by engine controller when switching tracks automatically
        crossfadeEnabled_))
  {
    xfadeNextTrack_ = false;
    // Stop a probably running fader
    if( s_fader ) {
      stopFader_ = true;
      s_fader->finish(); // makes the fader stop abruptly
      delete s_fader;
    }
    s_fader = new Fader( this, xfadeLength_ );
    setEqualizerParameters( intPreamp_, equalizerGains_ );
  }

  // for users who stubbonly refuse to use DMIX or buy a good soundcard
  // why doesn't xine do this? I cannot say.
  xine_close( stream_ );

  qDebug() << "Before xine_open() *****";

  if( xine_open( stream_, url.toEncoded() ) )
  {
    qDebug() << "After xine_open() *****";

#ifndef XINE_SAFE_MODE
    xine_post_out_t *source = xine_get_audio_source( stream_ );
    xine_post_in_t  *target = (xine_post_in_t*)xine_post_input( post_, const_cast<char*>("audio in") );
    xine_post_wire( source, target );
#endif

    playlistChanged();

    return true;
  }
  else
  {
#ifdef XINE_PARAM_GAPLESS_SWITCH
    //if ( xine_check_version(1,1,1) && !(xfadeLength_ > 0) )
      //xine_set_param( stream_, XINE_PARAM_GAPLESS_SWITCH, 0);
#endif
  }

  // FAILURE to load!
  //s_fader will delete itself
  determineAndShowErrorMessage();

  return false;
}

bool
XineEngine::Play( uint offset )
{
  if( !ensureStream() )
    return false;

  const bool has_audio     = xine_get_stream_info( stream_, XINE_STREAM_INFO_HAS_AUDIO );
  const bool audio_handled = xine_get_stream_info( stream_, XINE_STREAM_INFO_AUDIO_HANDLED );

  if (has_audio && audio_handled && xine_play( stream_, 0, offset ))
  {
    if( s_fader )
      s_fader->start( QThread::LowestPriority );

    emit StateChanged( Engine::Playing );

    return true;
  }

  //we need to stop the track that is prepped for crossfade
  delete s_fader;

  emit StateChanged( Engine::Empty );

  determineAndShowErrorMessage();

  xine_close( stream_ );

  return false;
}

void
XineEngine::determineAndShowErrorMessage()
{
  QString body;

  qDebug() << "xine_get_error()";
  switch (xine_get_error( stream_ )) {
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
      if (!xine_get_stream_info( stream_, XINE_STREAM_INFO_AUDIO_HANDLED ))
      {
        // xine can read the plugin but it didn't find any codec
        // THUS xine=daft for telling us it could handle the format in canDecode!
        body = "There is no available decoder.";
        QString const ext = QFileInfo(url_.path()).completeSuffix();
        // TODO
        //if (ext == "mp3" && EngineController::installDistroCodec( "xine-engine" ))
        //    return;
      }
      else if (!xine_get_stream_info( stream_, XINE_STREAM_INFO_HAS_AUDIO ))
        body = "There is no audio channel!";
      break;
  }

  // TODO
  qWarning() << body;
}

void
XineEngine::Stop()
{
  if( s_fader && s_fader->isRunning())
    s_fader->resume(); // safety call if the engine is in the pause state

  if ( !stream_ )
    return;

  if(  (fadeoutEnabled_ && !fadeOutRunning_) || state() == Engine::Paused )
  {
    s_outfader = new OutFader( this, fadeoutDuration_ );
    s_outfader->start();
    ::usleep( 100 ); //to be sure engine state won't be changed before it is checked in fadeOut()
    url_ = QUrl(); //to ensure we return Empty from state()

    std::fill( scope_.begin(), scope_.end(), 0 );
  }
  else if( !fadeOutRunning_ )
  {
    xine_stop( stream_ );
    xine_close( stream_ );
    xine_set_param( stream_, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);
  }

  emit StateChanged( Engine::Empty );
}

  void
XineEngine::Pause()
{
  if ( !stream_ )
    return;

  if( xine_get_param( stream_, XINE_PARAM_SPEED ) != XINE_SPEED_PAUSE )
  {
    if( s_fader && s_fader->isRunning() )
      s_fader->pause();

    xine_set_param( stream_, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
    xine_set_param( stream_, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);
    emit StateChanged( Engine::Paused );

  }
}

void
XineEngine::Unpause()
{
  if ( !stream_ )
    return;

  if( xine_get_param( stream_, XINE_PARAM_SPEED ) == XINE_SPEED_PAUSE )
  {
    if( s_fader && s_fader->isRunning() )
      s_fader->resume();

    xine_set_param( stream_, XINE_PARAM_SPEED, XINE_SPEED_NORMAL );
    emit StateChanged( Engine::Playing );
  }
}

Engine::State
XineEngine::state() const
{
  if ( !stream_ || fadeOutRunning_ )
    return Engine::Empty;

  switch( xine_get_status( stream_ ) )
  {
    case XINE_STATUS_PLAY: return xine_get_param( stream_, XINE_PARAM_SPEED )  != XINE_SPEED_PAUSE ? Engine::Playing : Engine::Paused;
    case XINE_STATUS_IDLE: return Engine::Empty;
    case XINE_STATUS_STOP:
    default:               return url_.isEmpty() ? Engine::Empty : Engine::Idle;
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
    xine_get_pos_length( stream_, &pos, &time, &length );
    if( time > tmp ) break;
    usleep( 100000 );
  }

  // Here we check for new metadata periodically, because xine does not emit an event
  // in all cases (e.g. with ogg streams). See BUG 122505
  if ( state() != Engine::Idle && state() != Engine::Empty )
  {
    const Engine::SimpleMetaBundle bundle = fetchMetaData();
    if( bundle.title != currentBundle_.title || bundle.artist != currentBundle_.artist ) {
      qDebug() << "Metadata received.";
      currentBundle_ = bundle;

      XineEngine* p = const_cast<XineEngine*>( this );
      p->emit MetaData( bundle );
    }
  }

  return time;
}

uint
XineEngine::length() const
{
  if ( !stream_ )
    return 0;

  // xine often delivers nonsense values for VBR files and such, so we only
  // use the length for remote files

  if( url_.scheme().toLower() == "file" )
    return 0;

  else {
    int pos;
    int time;
    int length = 0;

    xine_get_pos_length( stream_, &pos, &time, &length );
    if( length < 0 )
      length=0;

    return length;
  }
}

void
XineEngine::Seek( uint ms )
{
  if( !ensureStream() )
    return;

  if( xine_get_param( stream_, XINE_PARAM_SPEED ) == XINE_SPEED_PAUSE ) {
    // FIXME this is a xine API issue really, they need to add a seek function
    xine_play( stream_, 0, (int)ms );
    xine_set_param( stream_, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
  }
  else
    xine_play( stream_, 0, (int)ms );
}

void
XineEngine::SetVolumeSW( uint vol )
{
  if ( !stream_ )
    return;
  if( !s_fader )
    xine_set_param( stream_, XINE_PARAM_AUDIO_AMP_LEVEL, static_cast<uint>( vol * preamp_ ) );
}

void
XineEngine::fadeOut( uint fadeLength, bool* terminate, bool exiting )
{
  if( fadeOutRunning_ ) //Let us not start another fadeout...
    return;

  fadeOutRunning_ = !fadeOutRunning_;
  const bool isPlaying = stream_ && ( xine_get_status( stream_ ) == XINE_STATUS_PLAY );
  const float originalVol = Engine::Base::MakeVolumeLogarithmic( volume_ ) * preamp_;

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
      float vol = Engine::Base::MakeVolumeLogarithmic( volume_ ) * preamp_;
      float mix = (float)t.elapsed() / (float)length;
      if ( mix > 1.0 )
      {
        break;
      }
      if ( stream_ )
      {
        float v = 4.0 * (1.0 - mix) / 3.0;
        xine_set_param( stream_, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)( v < 1.0 ? vol * v : vol ) );
      }
    }
  }
  if( fadeOutRunning_ && stream_ )
    xine_set_param( stream_, XINE_PARAM_AUDIO_AMP_LEVEL, (uint) originalVol );
  fadeOutRunning_ = !fadeOutRunning_;
}

void
XineEngine::setEqualizerEnabled( bool enable )
{
  if ( !stream_ )
    return;

  equalizerEnabled_ = enable;

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
  if ( !stream_ )
    return;

  equalizerGains_ = gains;
  intPreamp_ = preamp;
  QList<int>::ConstIterator it = gains.begin();

  xine_set_param( stream_, XINE_PARAM_EQ_30HZ,    int( (*it  )*0.995 + 100 ) );
  xine_set_param( stream_, XINE_PARAM_EQ_60HZ,    int( (*++it)*0.995 + 100 ) );
  xine_set_param( stream_, XINE_PARAM_EQ_125HZ,   int( (*++it)*0.995 + 100 ) );
  xine_set_param( stream_, XINE_PARAM_EQ_250HZ,   int( (*++it)*0.995 + 100 ) );
  xine_set_param( stream_, XINE_PARAM_EQ_500HZ,   int( (*++it)*0.995 + 100 ) );
  xine_set_param( stream_, XINE_PARAM_EQ_1000HZ,  int( (*++it)*0.995 + 100 ) );
  xine_set_param( stream_, XINE_PARAM_EQ_2000HZ,  int( (*++it)*0.995 + 100 ) );
  xine_set_param( stream_, XINE_PARAM_EQ_4000HZ,  int( (*++it)*0.995 + 100 ) );
  xine_set_param( stream_, XINE_PARAM_EQ_8000HZ,  int( (*++it)*0.995 + 100 ) );
  xine_set_param( stream_, XINE_PARAM_EQ_16000HZ, int( (*++it)*0.995 + 100 ) );

  preamp_ = ( preamp - 0.1 * preamp + 100 ) / 100.0;
  SetVolume( volume_ );
}

bool
XineEngine::CanDecode( const QUrl &url )
{
  static QStringList list;
  if(list.isEmpty())
  {
    QMutexLocker l(&const_cast<XineEngine*>(this)->initMutex_);

    if (list.isEmpty()) {
      char* exts = xine_get_file_extensions( xine_ );
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
  if( !post_ || !stream_ || xine_get_status( stream_ ) != XINE_STATUS_PLAY )
    return scope_;

  MyNode* const myList         = scope_plugin_list( post_ );
  metronom_t* const myMetronom = scope_plugin_metronom( post_ );
  const int myChannels         = scope_plugin_channels( post_ );
  int scopeidx = 0;

  if (myChannels > 2)
    return scope_;

  for( int n, frame = 0; frame < 512; )
  {
    MyNode *best_node = 0;

    for( MyNode *node = myList->next; node != myList; node = node->next, Log::bufferCount++ )
      if( node->vpts <= currentVpts_ && (!best_node || node->vpts > best_node->vpts) )
        best_node = node;

    if( !best_node || best_node->vpts_end < currentVpts_ ) {
      Log::noSuitableBuffer++; break; }

    int64_t
      diff  = currentVpts_;
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
        scope_[scopeidx++] = data16[c];
        if (myChannels == 1) // duplicate mono samples
          scope_[scopeidx++] = data16[c];
      }
    }

    currentVpts_ = best_node->vpts_end;
    currentVpts_++; //FIXME needs to be done for some reason, or you get situations where it uses same buffer again and again
  }

  Log::scopeCallCount++;

  return scope_;
}

void
XineEngine::PruneScope()
{
  if ( !stream_ )
    return;

  //here we prune the buffer list regularly

  MyNode *myList = scope_plugin_list( post_ );

  if ( ! myList ) return;

  //we operate on a subset of the list for thread-safety
  MyNode * const first_node = myList->next;
  MyNode const * const list_end = myList;

  currentVpts_ = (xine_get_status( stream_ ) == XINE_STATUS_PLAY)
    ? xine_get_current_vpts( stream_ )
    : LLONG_MAX; //if state is not playing OR paused, empty the list
  //: std::numeric_limits<int64_t>::max(); //TODO don't support crappy gcc 2.95

  for( MyNode *prev = first_node, *node = first_node->next; node != list_end; node = node->next )
  {
    //we never delete first_node
    //this maintains thread-safety
    if( node->vpts_end < currentVpts_ ) {
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
      emit TrackEnded();
      return true;

    case XineEvent::InfoMessage:
      emit InfoMessage( (*message).arg( url_.toString() ) );
      delete message;
      return true;

    case XineEvent::StatusMessage:
      emit StatusText( *message );
      delete message;
      return true;

    case XineEvent::MetaInfoChanged: { //meta info has changed
      qDebug() << "Metadata received.";
      const Engine::SimpleMetaBundle bundle = fetchMetaData();
      if( bundle.title != currentBundle_.title || bundle.artist != currentBundle_.artist ) {
        currentBundle_ = bundle;

        emit MetaData( bundle );
      }
      return true;
    }

    case XineEvent::Redirecting:
                                     emit StatusText( QString("Redirecting to: ").arg( *message ) );
                                     Load( QUrl( *message ), Engine::Auto );
                                     Play();
                                     delete message;
                                     return true;
    case XineEvent::LastFMTrackChanged:
                                     emit LastFmTrackChange();
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
if ( xine_check_version(1,1,1) && !(xfadeLength_ > 0) 
&& url_.isLocalFile() && Playlist::instance()->isTrackAfter() )
{
xine_set_param(stream_, XINE_PARAM_EARLY_FINISHED_EVENT, 1 );
debug() << "XINE_PARAM_EARLY_FINISHED_EVENT enabled" << endl;
}
else
{
  //we don't want an early finish event if there is no track after the current one
  xine_set_param(stream_, XINE_PARAM_EARLY_FINISHED_EVENT, 0 );
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
      /*if ( xine_check_version(1,1,1) && xe->url_.isLocalFile() //Remote media break with gapless
      //don't prepare for a track that isn't coming
      && Playlist::instance()
      && Playlist::instance()->isTrackAfter()
      && !AmarokConfig::crossfade() )
      xine_set_param( xe->stream_, XINE_PARAM_GAPLESS_SWITCH, 1);*/
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

    case XINE_EVENT_MRL_REFERENCE_EXT: {
      /// xine has read the stream and found it actually links to something else
      /// so we need to play that instead

      QString message = QString::fromUtf8(
          static_cast<xine_mrl_reference_data_ext_t*>(xineEvent->data)->mrl);
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
  bundle.title      = QString::fromUtf8( xine_get_meta_info( stream_, XINE_META_INFO_TITLE ) );
  bundle.artist     = QString::fromUtf8( xine_get_meta_info( stream_, XINE_META_INFO_ARTIST ) );
  bundle.album      = QString::fromUtf8( xine_get_meta_info( stream_, XINE_META_INFO_ALBUM ) );
  bundle.comment    = QString::fromUtf8( xine_get_meta_info( stream_, XINE_META_INFO_COMMENT ) );
  bundle.genre      = QString::fromUtf8( xine_get_meta_info( stream_, XINE_META_INFO_GENRE ) );
  bundle.bitrate    = QString::number( xine_get_stream_info( stream_, XINE_STREAM_INFO_AUDIO_BITRATE ) / 1000 );
  bundle.samplerate = QString::number( xine_get_stream_info( stream_, XINE_STREAM_INFO_AUDIO_SAMPLERATE ) );
  bundle.year       = QString::fromUtf8( xine_get_meta_info( stream_, XINE_META_INFO_YEAR ) );
  bundle.tracknr    = QString::fromUtf8( xine_get_meta_info( stream_, XINE_META_INFO_TRACK_NUMBER ) );

  return bundle;
}

bool XineEngine::metaDataForUrl(const QUrl &url, Engine::SimpleMetaBundle &b)
{
  bool result = false;
  xine_stream_t* tmpstream = xine_stream_new(xine_, NULL, NULL);
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
    if (!xine_config_lookup_entry(xine_, "input.cdda_device", &config)) {
      emit StatusText("Failed CD device lookup in xine engine");
      return false;
    }
    config.str_value = (char *)device.toAscii().constData();
    xine_config_update_entry(xine_, &config);
  }

  emit StatusText("Getting AudioCD contents...");

  xine_urls = xine_get_autoplay_mrls(xine_, "CD", &num);

  if (xine_urls) {
    while (xine_urls[i]) {
      urls << QUrl(xine_urls[i]);
      ++i;
    }
  }
  else emit StatusText("Could not read AudioCD");

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
    , engine_( engine )
    , xine_( engine->xine_ )
    , decrease_( engine->stream_ )
    , increase_( 0 )
    , port_( engine->audioPort_ )
    , post_( engine->post_ )
    , fadeLength_( fadeMs )
    , paused_( false )
    , terminated_( false )
{
  if( engine->makeNewStream() )
  {
    increase_ = engine->stream_;

    xine_set_param( increase_, XINE_PARAM_AUDIO_AMP_LEVEL, 0 );
  }
  else {
    s_fader = 0;
    deleteLater();
  }
}

Fader::~Fader()
{
  wait();

  xine_close( decrease_ );
  xine_dispose( decrease_ );
  xine_close_audio_driver( xine_, port_ );
  if( post_ ) xine_post_dispose( xine_, post_ );

  if( !engine_->stopFader_ )
    engine_->SetVolume( engine_->volume() );

  engine_->stopFader_ = false;
  s_fader = 0;
}

void
Fader::run()
{
  // do a volume change in 100 steps (or every 10ms)
  uint stepsCount = fadeLength_ < 1000 ? fadeLength_ / 10 : 100;
  uint stepSizeUs = (int)( 1000.0 * (float)fadeLength_ / (float)stepsCount );

  float mix = 0.0;
  float elapsedUs = 0.0;
  while ( mix < 1.0 )
  {
    if ( terminated_ )
      break;
    // sleep a constant amount of time
    QThread::usleep( stepSizeUs );

    if ( paused_ )
      continue;

    elapsedUs += stepSizeUs;

    // get volume (amarok main * equalizer preamp)
    float vol = Engine::Base::MakeVolumeLogarithmic( engine_->volume_ ) * engine_->preamp_;

    // compute the mix factor as the percentage of time spent since fade begun
    float mix = (elapsedUs / 1000.0) / (float)fadeLength_;
    if ( mix > 1.0 )
    {
      if ( increase_ )
        xine_set_param( increase_, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)vol );
      break;
    }

    // change volume of streams (using dj-like cross-fade profile)
    if ( decrease_ )
    {
      //xine_set_param( decrease_, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)(vol * (1.0 - mix)) );  // linear
      float v = 4.0 * (1.0 - mix) / 3.0;
      xine_set_param( decrease_, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)( v < 1.0 ? vol * v : vol ) );
    }
    if ( increase_ )
    {
      //xine_set_param( increase_, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)(vol * mix) );  //linear
      float v = 4.0 * mix / 3.0;
      xine_set_param( increase_, XINE_PARAM_AUDIO_AMP_LEVEL, (uint)( v < 1.0 ? vol * v : vol ) );
    }
  }

  //stop using cpu!
  xine_stop( decrease_ );

  deleteLater();
}

  void
Fader::pause()
{
  paused_ = true;
}

  void
Fader::resume()
{
  paused_ = false;
}

  void
Fader::finish()
{
  terminated_ = true;
}

//////////////////////////////////////////////////////////////////////////////
/// class OutFader
//////////////////////////////////////////////////////////////////////////////

OutFader::OutFader( XineEngine *engine, uint fadeLength )
    : QThread(engine)
    , engine_( engine )
    , terminated_( false )
    , fadeLength_( fadeLength )
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
  engine_->fadeOut( fadeLength_, &terminated_ );

  xine_stop( engine_->stream_ );
  xine_close( engine_->stream_ );
  xine_set_param( engine_->stream_, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);

  deleteLater();
}

void
OutFader::finish()
{
  terminated_ = true;
}

PruneScopeThread::PruneScopeThread(XineEngine *parent)
  : engine_(parent)
{
}

void PruneScopeThread::run() {
  QTimer timer;
  connect(&timer, SIGNAL(timeout()), engine_, SLOT(PruneScope()), Qt::DirectConnection);
  timer.start(1000);

  exec();
}
