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

#include "gstengine.h"
#include "gstequalizer.h"

#include <math.h>
#include <unistd.h>
#include <vector>

#include <QTimer>
#include <QRegExp>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QtDebug>

#include <gst/gst.h>
#include <iostream>

#define RETURN_IF_PIPELINE_EMPTY if ( !pipeline_filled_ ) return


using std::vector;

GstEngine* GstEngine::sInstance = NULL;
const char* GstEngine::kSettingsGroup = "GstEngine";
const char* GstEngine::kAutoSink = "autoaudiosink";


/////////////////////////////////////////////////////////////////////////////////////
// CALLBACKS
/////////////////////////////////////////////////////////////////////////////////////

//GstBusSyncReply
gboolean GstEngine::BusCallback(GstBus*, GstMessage* msg, gpointer) {
  switch ( GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
      GError* error;
      gchar* debugs;

      gst_message_parse_error(msg,&error,&debugs);
      qDebug() << "ERROR RECEIVED IN BUS_CB <" << error->message << ">" ;;

      instance()->gst_error_ = QString::fromAscii( error->message );
      instance()->gst_debug_ = QString::fromAscii( debugs );
      QMetaObject::invokeMethod(instance(), "HandlePipelineError", Qt::QueuedConnection);
      break;
    }

    case GST_MESSAGE_TAG: {
      gchar* string=NULL;
      Engine::SimpleMetaBundle bundle;
      GstTagList* taglist;
      gst_message_parse_tag(msg,&taglist);
      bool success = false;

      if ( gst_tag_list_get_string( taglist, GST_TAG_TITLE, &string ) && string ) {
        qDebug() << "received tag 'Title': " << QString( string ) ;
        bundle.title = string;
        success = true;
      }
      if ( gst_tag_list_get_string( taglist, GST_TAG_ARTIST, &string ) && string ) {
        qDebug() << "received tag 'Artist': " << QString( string ) ;
        bundle.artist = string;
        success = true;
      }
      if ( gst_tag_list_get_string( taglist, GST_TAG_COMMENT, &string ) && string ) {
        qDebug() << "received tag 'Comment': " << QString( string ) ;
        bundle.comment = string;
        success = true;
      }
      if ( gst_tag_list_get_string( taglist, GST_TAG_ALBUM, &string ) && string ) {
        qDebug() << "received tag 'Album': " << QString( string ) ;
        bundle.album = string;
        success = true;
      }
      g_free(string);
      gst_tag_list_free(taglist);
      if (success) {
        instance()->meta_bundle_ = bundle;
        QMetaObject::invokeMethod(instance(), "NewMetaData", Qt::QueuedConnection);
      }
      break;
    }

    default:
      break;
  }
  return GST_BUS_DROP;
}

GstBusSyncReply GstEngine::BusCallbackSync(GstBus*, GstMessage* msg, gpointer) {
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      QMetaObject::invokeMethod(instance(), "EndOfStreamReached",
                                Qt::QueuedConnection);
      break;

    default:
      break;
  }

  return GST_BUS_PASS;
}


void GstEngine::NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer) {
  GstPad* const audiopad = gst_element_get_pad( instance()->gst_audiobin_, "sink" );

  if ( GST_PAD_IS_LINKED( audiopad ) ) {
    qDebug() << "audiopad is already linked. Unlinking old pad." ;
    gst_pad_unlink( audiopad, GST_PAD_PEER( audiopad ) );
  }

  gst_pad_link( pad, audiopad );

  gst_object_unref( audiopad );
}


void GstEngine::HandoffCallback(GstPad*, GstBuffer* buf, gpointer arg) {
  GstEngine *thisObj = static_cast<GstEngine *>( arg );

  // push the buffer onto the delay queue
  gst_buffer_ref(buf);
  g_queue_push_tail(thisObj->delayq_, buf);
}

void GstEngine::EventCallback(GstPad*, GstEvent* event, gpointer) {
  
  switch ( static_cast<int>(event->type) )
  {
    case GST_EVENT_EOS:
      qDebug() << "EOS reached";
      QMetaObject::invokeMethod(instance(), "EndOfStreamReached",
                                Qt::QueuedConnection);
      break;

    case GST_EVENT_TAG:
      qDebug() << "GOT NEW TAG";
      break;

    default:
      break;
  }
}


void GstEngine::CanDecodeNewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer) {
  GstCaps* caps = gst_pad_get_caps(pad);
  if (gst_caps_get_size(caps) > 0) {
    GstStructure* str = gst_caps_get_structure(caps, 0);
    if (g_strrstr(gst_structure_get_name( str ), "audio" ))
      instance()->can_decode_success_ = true;
  }
  gst_caps_unref(caps);
}

void GstEngine::CanDecodeLastCallback(GstElement*, gpointer) {
  instance()->can_decode_last_ = true;
}


GstEngine::GstEngine()
  : Engine::Base(),
    event_cb_id_(0),
    delayq_(g_queue_new()),
    current_sample_(0),
    pipeline_filled_(false),
    fade_value_(0.0),
    equalizer_enabled_(false),
    shutdown_(false),
    can_decode_pipeline_(NULL),
    can_decode_src_(NULL),
    can_decode_bin_(NULL)
{
  ReloadSettings();
}

GstEngine::~GstEngine() {
  DestroyPipeline();

  if (can_decode_pipeline_)
    gst_object_unref(GST_OBJECT(can_decode_pipeline_));

#ifdef GST_KIOSTREAMS
  delete[] m_streamBuf;
#endif

  // Destroy scope delay queue
  g_queue_free(delayq_);

  // Save configuration
  gst_deinit();
}


bool GstEngine::init() {
  sInstance = this;

  // GStreamer initialization
  GError *err;
  if ( !gst_init_check( NULL, NULL, &err ) ) {
    qWarning("GStreamer could not be initialized");
    return false;
  }

  // Check if registry exists
  GstElement* dummy = gst_element_factory_make ( "fakesink", "fakesink" );
  if ( !dummy ) {
    qWarning("GStreamer is missing a registry.  Please make sure that you"
             " have installed all necessary GStreamer plugins (e.g. OGG"
             " and MP3), and run 'gst-register' afterwards.");
    return false;
  }

  gst_object_unref(dummy);

  return true;
}

void GstEngine::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  sink_ = s.value("sink", kAutoSink).toString();
  device_ = s.value("device").toString();
}


bool GstEngine::canDecode(const QUrl &url) {
  // We had some bug reports claiming that video files cause crashes in canDecode(),
  // so don't try to decode them
  if ( url.path().toLower().endsWith( ".mov" ) ||
       url.path().toLower().endsWith( ".avi" ) ||
       url.path().toLower().endsWith( ".wmv" ) )
    return false;

  can_decode_success_ = false;
  can_decode_last_ = false;

  // Create the pipeline
  if (!can_decode_pipeline_) {
    can_decode_pipeline_ = CreateElement("pipeline");
    can_decode_src_ = CreateElement("giosrc", can_decode_pipeline_);
    can_decode_bin_ = CreateElement("decodebin", can_decode_pipeline_);

    gst_element_link(can_decode_src_, can_decode_bin_);
    g_signal_connect(G_OBJECT(can_decode_bin_), "new-decoded-pad", G_CALLBACK(CanDecodeNewPadCallback), NULL);
    g_signal_connect(G_OBJECT(can_decode_bin_), "no-more-pads", G_CALLBACK(CanDecodeLastCallback), NULL);
  }

  // Set the file we're testing
  g_object_set(G_OBJECT(can_decode_src_), "location", url.toEncoded().constData(), NULL);

  // Start the pipeline playing
  gst_element_set_state(can_decode_pipeline_, GST_STATE_PLAYING);

  // Wait until found audio stream
  int count = 0;
  while (!can_decode_success_ && !can_decode_last_ && count < 100) {
    count++;
    usleep(1000);
  }

  // Stop playing
  gst_element_set_state(can_decode_pipeline_, GST_STATE_NULL);

  return can_decode_success_;
}


uint GstEngine::position() const {
  RETURN_IF_PIPELINE_EMPTY 0;

  GstFormat fmt = GST_FORMAT_TIME;
  // Value will hold the current time position in nanoseconds. Must be initialized!
  gint64 value = 0;
  gst_element_query_position( gst_pipeline_, &fmt, &value );

  return static_cast<uint>( ( value / GST_MSECOND ) ); // nanosec -> msec
}


uint GstEngine::length() const {
  RETURN_IF_PIPELINE_EMPTY 0;

  GstFormat fmt = GST_FORMAT_TIME;
  // Value will hold the track length in nanoseconds. Must be initialized!
  gint64 value = 0;
  gst_element_query_duration(gst_pipeline_,  &fmt, &value);

  return uint( value / GST_MSECOND ); // nanosec -> msec
}


Engine::State GstEngine::state() const {
  RETURN_IF_PIPELINE_EMPTY m_url.isEmpty() ? Engine::Empty : Engine::Idle;

  GstState s, sp;
  GstStateChangeReturn sret = gst_element_get_state( gst_pipeline_, &s, &sp, kGstStateTimeout);

  if (sret == GST_STATE_CHANGE_FAILURE) {
    qDebug() << "Gst get state fails";
    return Engine::Empty;
  }

  switch (s) {
    case GST_STATE_NULL:    return Engine::Empty;
    case GST_STATE_READY:   return Engine::Idle;
    case GST_STATE_PLAYING: return Engine::Playing;
    case GST_STATE_PAUSED:  return Engine::Paused;
    default:                return Engine::Empty;
  }
}


const Engine::Scope& GstEngine::scope() {
  UpdateScope();

  if (current_sample_ >= SCOPESIZE) {
    // ok, we have a full buffer now, so give it to the scope
    for (int i=0; i< SCOPESIZE; i++)
      m_scope[i] = current_scope_[i];
    current_sample_ = 0;
  }

  return m_scope;
}

void GstEngine::UpdateScope() {
  typedef int16_t sampletype;

  // prune the scope and get the current pos of the audio device
  quint64 pos = PruneScope();

  // head of the delay queue is the most delayed, so we work with that one
  GstBuffer *buf = reinterpret_cast<GstBuffer *>( g_queue_peek_head(delayq_) );
  if (!buf)
    return;

  // start time for this buffer
  quint64 stime = GST_BUFFER_TIMESTAMP(buf);
  // duration of the buffer...
  quint64 dur = GST_BUFFER_DURATION(buf);
  // therefore we can calculate the end time for the buffer
  quint64 etime = stime + dur;

  // determine the number of channels
  GstStructure* structure = gst_caps_get_structure ( GST_BUFFER_CAPS( buf ), 0);
  int channels = 2;
  gst_structure_get_int (structure, "channels", &channels);

  // scope does not support >2 channels
  if (channels > 2)
    return;

  // if the audio device is playing this buffer now
  if (pos <= stime || pos >= etime)
    return;

  // calculate the number of samples in the buffer
  int sz = GST_BUFFER_SIZE(buf) / sizeof(sampletype);
  // number of frames is the number of samples in each channel (frames like in the alsa sense)
  int frames = sz / channels;

  // find the offset into the buffer to the sample closest to where the audio device is playing
  // it is the (time into the buffer cooresponding to the audio device pos) / (the sample rate)
  // sample rate = duration of the buffer / number of frames in the buffer
  // then we multiply by the number of channels to find the offset of the left channel sample
  // of the frame in the buffer
  int off = channels * (pos - stime) / (dur / frames);

  // note that we are assuming 32 bit samples, but this should probably be generalized...
  sampletype* data = reinterpret_cast<sampletype *>(GST_BUFFER_DATA(buf));
  if (off >= sz) // better be...
    return;

  int i = off; // starting at offset

  // loop while we fill the current buffer.  If we need another buffer and one is available,
  // get it and keep filling.  If there are no more buffers available (not too likely)
  // then leave everything in this state and wait until the next time the scope updates
  while (buf && current_sample_ < SCOPESIZE && i < sz) {
    for (int j = 0; j < channels && current_sample_ < SCOPESIZE; j++) {
      current_scope_[current_sample_ ++] = data[i + j];
    }
    i+=channels; // advance to the next frame

    if (i >= sz - 1) {
      // here we are out of samples in the current buffer, so we get another one
      buf = reinterpret_cast<GstBuffer *>( g_queue_pop_head(delayq_) );
      gst_buffer_unref(buf);
      buf = reinterpret_cast<GstBuffer *>( g_queue_peek_head(delayq_) );
      if (buf) {
        stime = GST_BUFFER_TIMESTAMP(buf);
        dur = GST_BUFFER_DURATION(buf);
        etime = stime + dur;
        i = 0;
        sz = GST_BUFFER_SIZE(buf) / sizeof(sampletype);
        data = reinterpret_cast<sampletype *>(GST_BUFFER_DATA(buf));
      }
    }
  }
}


bool GstEngine::metaDataForUrl(const QUrl &url, Engine::SimpleMetaBundle& b) {
  qDebug() << "GstEngine::metaDataForUrl " << url ;
  // TODO
  /*if ( url.scheme() == "cdda" )
    {
        // TODO: gstreamer doesn't support cddb natively, but could perhaps use libkcddb?
        b.title = QString(  "Track %1"  ).arg( url.host() );
        b.album = "AudioCD";

        if ( setupAudioCD( url.encodedQuery().remove( QRegExp( "^\\?" ) ), url.host().toUInt(), true ) )
        {
            GstPad *pad;
            if ( ( pad = gst_element_get_pad( gst_src_, "src" ) ) )
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
            if ( !gst_element_set_state( gst_pipeline_, GST_STATE_NULL ) ) {
                qWarning() << "Could not set thread to NULL.";
                DestroyPipeline();
            }
        }
        return true;
    }*/
  return false;
}


bool GstEngine::getAudioCDContents(const QString &device, QList<QUrl> &urls) {
  // TODO
  /*qDebug() << "GstEngine::getAudioCDContents " << device ;

    bool result = false;
    if ( setupAudioCD( device, 0, true ) )
    {
        GstFormat format;
        if ( ( format = gst_format_get_by_nick("track") ) != GST_FORMAT_UNDEFINED )
        {
            gint64 tracks = 0;
            if ( gst_element_query_duration( gst_pipeline_, &format, &tracks ) )
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
        if ( !gst_element_set_state( gst_pipeline_, GST_STATE_NULL ) ) {
            qWarning() << "Could not set thread to NULL.";
            DestroyPipeline();
        }
    }
    return result;*/
  return false;
}

bool GstEngine::load(const QUrl& url, bool stream) {
  Engine::Base::load( url, stream );

  qDebug() << "Loading url: " << url.toEncoded() ;

  if ( url.scheme() == "cdda" ) {
    /*if ( !setupAudioCD( url.encodedQuery().remove( QRegExp( "^\\?" ) ), url.host().toUInt(), false ) )*/
    return false;
  } else {
    if ( !CreatePipeline() )
      return false;

    gst_src_ = CreateElement("giosrc");
    if (!gst_src_) {
      qDebug() << "******* cannot get stream src " ;

      DestroyPipeline();
      return false;
    }

    g_object_set (G_OBJECT (gst_src_), "location", url.toEncoded().constData(), NULL);
    gst_bin_add( GST_BIN( gst_pipeline_ ), gst_src_ );

    if ( !( gst_decodebin_ = CreateElement( "decodebin", gst_pipeline_ ) ) ) { DestroyPipeline(); return false; }
    g_signal_connect( G_OBJECT( gst_decodebin_ ), "new-decoded-pad", G_CALLBACK( NewPadCallback ), NULL );

    GstPad* p = gst_element_get_pad (gst_decodebin_, "sink");
    if (p) {
      event_cb_id_ = gst_pad_add_event_probe (p, G_CALLBACK(EventCallback), this);
      gst_object_unref (p);
    }

    // Link elements. The link from decodebin to audioconvert will be made in the newPad-callback
    gst_element_link( gst_src_, gst_decodebin_ );
  }

  setVolume(m_volume);
  setEqualizerEnabled(equalizer_enabled_);
  setEqualizerParameters(equalizer_preamp_, equalizer_gains_);
  return true;
}


bool GstEngine::play( uint offset ) {
  // Try to play input pipeline; if fails, destroy input bin
  GstStateChangeReturn sret;
  if ( !(sret = gst_element_set_state( gst_pipeline_, GST_STATE_PLAYING )) ) {
    qWarning() << "Could not set thread to PLAYING.";
    DestroyPipeline();
    return false;
  }

  // If "Resume playback on start" is enabled, we must seek to the last position
  if (offset) seek(offset);

  current_sample_ = 0;
  startTimer(kTimerInterval);
  emit stateChanged(Engine::Playing);
  return true;
}


void GstEngine::stop() {
  m_url = QUrl(); // To ensure we return Empty from state()

  if (pipeline_filled_) {
    // Is a fade running?
    if ( fade_value_ == 0.0 )
      fade_value_ = 1.0;
    else
      DestroyPipeline();
  }

  emit stateChanged(Engine::Empty);
}

void GstEngine::pause() {
  RETURN_IF_PIPELINE_EMPTY;

  if ( GST_STATE( gst_pipeline_ ) == GST_STATE_PLAYING ) {
    gst_element_set_state( gst_pipeline_, GST_STATE_PAUSED );
    emit stateChanged( Engine::Paused );
  }
}

void GstEngine::unpause() {
  RETURN_IF_PIPELINE_EMPTY;

  if ( GST_STATE( gst_pipeline_ ) == GST_STATE_PAUSED ) {
    gst_element_set_state( gst_pipeline_, GST_STATE_PLAYING );
    emit stateChanged( Engine::Playing );
  }
}

void GstEngine::seek( uint ms ) {
  RETURN_IF_PIPELINE_EMPTY;

  if (!gst_element_seek(gst_pipeline_, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, ms*GST_MSECOND,
                            GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) qDebug() << "Seek failed" ;
  else ClearScopeQ();
  gst_element_get_state(gst_pipeline_, NULL, NULL, 100*GST_MSECOND);
}

void GstEngine::setEqualizerEnabled(bool enabled) {
  equalizer_enabled_= enabled;

  RETURN_IF_PIPELINE_EMPTY;

  g_object_set(G_OBJECT(gst_equalizer_), "active", enabled, NULL);
}


void GstEngine::setEqualizerParameters( int preamp, const QList<int>& band_gains ) {
  equalizer_preamp_ = preamp;
  equalizer_gains_ = band_gains;

  RETURN_IF_PIPELINE_EMPTY;

  // Preamp
  g_object_set(G_OBJECT(gst_equalizer_), "preamp", ( preamp + 100 ) / 2, NULL);

  // Gains
  vector<int> gains_temp;
  gains_temp.resize( band_gains.count() );
  for ( int i = 0; i < band_gains.count(); i++ )
    gains_temp[i] = band_gains.at( i ) + 100;

  g_object_set(G_OBJECT(gst_equalizer_), "gain", &gains_temp, NULL);
}

void GstEngine::setVolumeSW( uint percent ) {
  RETURN_IF_PIPELINE_EMPTY;

  double fade;
  if ( fade_value_ > 0.0 )
    fade = 1.0 - log10( ( 1.0 - fade_value_ ) * 9.0 + 1.0 );
  else
    fade = 1.0;

  g_object_set( G_OBJECT(gst_volume_), "volume", (double) percent * fade * 0.01, NULL );
}


void GstEngine::timerEvent( QTimerEvent* ) {
  // keep the scope from building while we are not visible
  // this is why the timer must run as long as we are playing, and not just when
  // we are fading
  PruneScope();

  // *** Volume fading ***

  // Are we currently fading?
  if ( fade_value_ > 0.0 ) {
    // TODO
    //m_fadeValue -= ( AmarokConfig::fadeoutLength() ) ?  1.0 / AmarokConfig::fadeoutLength() * TIMER_INTERVAL : 1.0;
    fade_value_ -= 1.0;

    // Fade finished?
    if ( fade_value_ <= 0.0 ) {
      // Fade transition has finished, stop playback
      qDebug() << "[Gst-Engine] Fade-out finished.";
      DestroyPipeline();
      //killTimers();
    }
    setVolume( volume() );
  }
}


/////////////////////////////////////////////////////////////////////////////////////
// PRIVATE SLOTS
/////////////////////////////////////////////////////////////////////////////////////

void GstEngine::HandlePipelineError() {
  QString text = "[GStreamer Error] ";
  text += gst_error_;

  if ( !gst_debug_.isEmpty() ) {
    text += " ** ";
    text += gst_debug_;
  }

  gst_error_ = QString();
  emit statusText( text );
  qWarning() << text ;

  DestroyPipeline();
}


void GstEngine::EndOfStreamReached() {
  DestroyPipeline();
  emit trackEnded();
}

void GstEngine::NewMetaData() {
  emit metaData( meta_bundle_ );
}

void GstEngine::ErrorNoOutput() {
  QMessageBox::information( 0, "Error", "<p>Please select a GStreamer <u>output plugin</u> in the engine settings dialog.</p>" );
}

GstElement* GstEngine::CreateElement(
    const QString& factoryName, GstElement* bin, const QString& name ) {
  GstElement* element =
      gst_element_factory_make(
          factoryName.toAscii().constData(),
          name.isNull() ? factoryName.toAscii().constData() : name.toAscii().constData() );

  if ( element ) {
    if ( bin ) gst_bin_add( GST_BIN( bin ), element );
  } else {
    QMessageBox::critical( 0, "Error",
                           QString("<h3>GStreamer could not create the element: <i>%1</i></h3> "
                                   "<p>Please make sure that you have installed all necessary GStreamer plugins (e.g. OGG and MP3), and run <i>'gst-register'</i> afterwards.</p>"
                                   "<p>For further assistance consult the GStreamer manual, and join #gstreamer on irc.freenode.net.</p>" ).arg( factoryName ) );
    gst_object_unref( GST_OBJECT( bin ) );
  }

  return element;
}


GstEngine::PluginDetailsList
    GstEngine::GetPluginList(const QString& classname) const {
  PluginDetailsList ret;

  GstRegistry* registry = gst_registry_get_default();
  GList* features =
      gst_registry_get_feature_list(registry, GST_TYPE_ELEMENT_FACTORY);

  while (features) {
    GstElementFactory* factory = GST_ELEMENT_FACTORY(features->data);
    if (QString(factory->details.klass).contains(classname)) {
      PluginDetails details;
      details.name = QString::fromUtf8(GST_PLUGIN_FEATURE_NAME(features->data));
      details.long_name = QString::fromUtf8(factory->details.longname);
      details.description = QString::fromUtf8(factory->details.description);
      details.author = QString::fromUtf8(factory->details.author);
      ret << details;
    }
    features = g_list_next ( features );
  }

  gst_plugin_feature_list_free(features);
  return ret;
}


bool GstEngine::CreatePipeline() {
  DestroyPipeline();

  gst_pipeline_ = gst_pipeline_new( "pipeline" );
  gst_audiobin_ = gst_bin_new( "audiobin" );

  if ( !( gst_audiosink_ = CreateElement( sink_, gst_audiobin_ ) ) ) {
    QMetaObject::invokeMethod(this, "ErrorNoOutput", Qt::QueuedConnection);
    return false;
  }

  if (DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(sink_) && !device_.isEmpty()) {
    g_object_set(G_OBJECT(gst_audiosink_), "device", device_.toUtf8().constData(), NULL);
  }

  gst_equalizer_ = GST_ELEMENT(gst_equalizer_new());
  gst_bin_add(GST_BIN(gst_audiobin_), gst_equalizer_);

  if ( !( gst_audioconvert_ = CreateElement( "audioconvert", gst_audiobin_ ) ) ) { return false; }
  if ( !( gst_identity_ = CreateElement( "identity", gst_audiobin_ ) ) ) { return false; }
  if ( !( gst_volume_ = CreateElement( "volume", gst_audiobin_ ) ) ) { return false; }
  if ( !( gst_audioscale_ = CreateElement( "audioresample", gst_audiobin_ ) ) ) { return false; }

  GstPad* p;
  p = gst_element_get_pad(gst_audioconvert_, "sink");
  gst_element_add_pad(gst_audiobin_,gst_ghost_pad_new("sink",p));
  gst_object_unref(p);

  // add a data probe on the src pad if the audioconvert element for our scope
  // we do it here because we want pre-equalized and pre-volume samples
  // so that our visualization are not affected by them
  p = gst_element_get_pad (gst_audioconvert_, "src");
  gst_pad_add_buffer_probe (p, G_CALLBACK(HandoffCallback), this);
  gst_object_unref (p);

  // Ensure we get the right type out of audioconvert for our scope
  GstCaps* caps = gst_caps_new_simple ("audio/x-raw-int",
      "width", G_TYPE_INT, 16,
      "signed", G_TYPE_BOOLEAN, true,
      NULL);
  gst_element_link_filtered(gst_audioconvert_, gst_equalizer_, caps);
  gst_caps_unref(caps);

  // Add an extra audioconvert at the end as osxaudiosink supports only one format.
  GstElement* convert = CreateElement( "audioconvert", gst_audiobin_, "FFFUUUU" );
  if (!convert) { return false; }
  gst_element_link_many( gst_equalizer_, gst_identity_, gst_volume_,
                         gst_audioscale_, convert, gst_audiosink_, NULL );


  gst_bin_add( GST_BIN(gst_pipeline_), gst_audiobin_);
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(gst_pipeline_)), BusCallbackSync, NULL);
  gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(gst_pipeline_)), BusCallback, NULL);

  pipeline_filled_ = true;
  return true;
}


void GstEngine::DestroyPipeline() {
  fade_value_ = 0.0;

  ClearScopeQ();

  if ( pipeline_filled_ ) {
    // Remove the event handler while we destroy the pipeline
    GstPad *p = gst_element_get_pad (gst_decodebin_, "sink");
    if (p)
      gst_pad_remove_event_probe(p, event_cb_id_);

    gst_element_set_state( gst_pipeline_, GST_STATE_NULL );
    gst_object_unref( GST_OBJECT( gst_pipeline_ ) );

    pipeline_filled_ = false;
  }
}


bool GstEngine::SetupAudioCD( const QString& device, unsigned track, bool pause) {
  qDebug() << "setupAudioCD: device = " << device << ", track = " << track << ", pause = " << pause ;
  bool filled = pipeline_filled_ && gst_src_ && strcmp( gst_element_get_name( gst_src_ ), "cdiocddasrc" ) == 0;
  if ( filled || CreatePipeline() ) {
    if ( filled || ( gst_src_ = CreateElement( "cdiocddasrc", gst_pipeline_, "cdiocddasrc" ) ) ) {
      // TODO: allow user to configure default device rather than falling back to gstreamer default when no device passed in
      if ( !device.isNull() )
        g_object_set( G_OBJECT(gst_src_), "device", device.toAscii().constData(), NULL );
      if ( track )
        g_object_set (G_OBJECT (gst_src_), "track", track, NULL);
      if ( filled || gst_element_link( gst_src_, gst_audiobin_ ) ) {
        // the doco says we should only have to go to READY to read metadata, but that doesn't actually work
        if ( gst_element_set_state( gst_pipeline_, pause ? GST_STATE_PAUSED : GST_STATE_READY ) != GST_STATE_CHANGE_FAILURE && gst_element_get_state( gst_pipeline_, NULL, NULL, GST_CLOCK_TIME_NONE ) == GST_STATE_CHANGE_SUCCESS )
        {
          return true;
        }
      }
    }
    DestroyPipeline();
  }
  return false;
}


qint64 GstEngine::PruneScope() {
  if ( !pipeline_filled_ ) return 0; // don't prune if we aren't playing

  // get the position playing in the audio device
  GstFormat fmt = GST_FORMAT_TIME;
  gint64 pos = 0;
  gst_element_query_position( gst_pipeline_, &fmt, &pos );

  GstBuffer *buf = 0;
  quint64 etime;

  // free up the buffers that the audio device has advanced past already
  do {
    // most delayed buffers are at the head of the queue
    buf = reinterpret_cast<GstBuffer *>( g_queue_peek_head(delayq_) );
    if (buf) {
      // the start time of the buffer
      quint64 stime = GST_BUFFER_TIMESTAMP(buf);
      // the duration of the buffer
      quint64 dur = GST_BUFFER_DURATION(buf);
      // therefore we can calculate the end time of the buffer
      etime = stime + dur;

      // purge this buffer if the pos is past the end time of the buffer
      if (pos > qint64(etime)) {
        g_queue_pop_head(delayq_);
        gst_buffer_unref(buf);
      }
    }
  } while (buf && pos > qint64(etime));

  return pos;
}

void GstEngine::ClearScopeQ() {
  // just free them all
  while (g_queue_get_length(delayq_)) {
    GstBuffer* buf = reinterpret_cast<GstBuffer *>( g_queue_pop_head(delayq_) );
    gst_buffer_unref(buf);
  }
}

bool GstEngine::DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(const QString &name) {
  return (name == "alsasink" || name == "osssink" || name == "pulsesink");
}
