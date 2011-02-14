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

#include "config.h"
#include "gstengine.h"
#include "gstenginepipeline.h"
#include "core/boundfuturewatcher.h"
#include "core/utilities.h"

#ifdef HAVE_IMOBILEDEVICE
# include "gstafcsrc/gstafcsrc.h"
#endif

#include <math.h>
#include <unistd.h>
#include <vector>
#include <iostream>

#include <boost/bind.hpp>

#include <QTimer>
#include <QRegExp>
#include <QFile>
#include <QSettings>
#include <QtDebug>
#include <QCoreApplication>
#include <QTimeLine>
#include <QDir>
#include <QtConcurrentRun>

#include <gst/gst.h>


using std::vector;
using boost::shared_ptr;

const char* GstEngine::kSettingsGroup = "GstEngine";
const char* GstEngine::kAutoSink = "autoaudiosink";
const char* GstEngine::kHypnotoadPipeline = 
      "audiotestsrc wave=6 ! "
      "audioecho intensity=1 delay=50000000 ! "
      "audioecho intensity=1 delay=25000000 ! "
      "equalizer-10bands "
      "band0=-24 band1=-3 band2=7.5 band3=12 band4=8 "
      "band5=6 band6=5 band7=6 band8=0 band9=-24";

// TODO: weird analyzer problems with .cues

GstEngine::GstEngine()
  : Engine::Base(),
    delayq_(g_queue_new()),
    current_sample_(0),
    equalizer_enabled_(false),
    rg_enabled_(false),
    rg_mode_(0),
    rg_preamp_(0.0),
    rg_compression_(true),
    buffer_duration_nanosec_(1 * kNsecPerSec), // 1s
    seek_timer_(new QTimer(this)),
    timer_id_(-1),
    next_element_id_(0)
{
  seek_timer_->setSingleShot(true);
  seek_timer_->setInterval(kSeekDelayNanosec / kNsecPerMsec);
  connect(seek_timer_, SIGNAL(timeout()), SLOT(SeekNow()));

  ReloadSettings();
}

GstEngine::~GstEngine() {
  EnsureInitialised();

  current_pipeline_.reset();

  // Destroy scope delay queue
  ClearScopeBuffers();
  g_queue_free(delayq_);

  // Save configuration
  gst_deinit();
}

void GstEngine::SetEnv(const char *key, const QString &value) {
#ifdef Q_OS_WIN32
  putenv(QString("%1=%2").arg(key, value).toLocal8Bit().constData());
#else
  setenv(key, value.toLocal8Bit().constData(), 1);
#endif
}

bool GstEngine::Init() {
  QString scanner_path;
  QString plugin_path;
  QString registry_filename;

  // On windows and mac we bundle the gstreamer plugins with clementine
#if defined(Q_OS_DARWIN)
  scanner_path = QCoreApplication::applicationDirPath() + "/../PlugIns/gst-plugin-scanner";
  plugin_path = QCoreApplication::applicationDirPath() + "/../PlugIns/gstreamer";
#elif defined(Q_OS_WIN32)
  plugin_path = QCoreApplication::applicationDirPath() + "/gstreamer-plugins";
#endif

#if defined(Q_OS_WIN32) || defined(Q_OS_DARWIN)
  registry_filename = Utilities::GetConfigPath(Utilities::Path_GstreamerRegistry);
#endif

  if (!scanner_path.isEmpty())
    SetEnv("GST_PLUGIN_SCANNER", scanner_path);

  if (!plugin_path.isEmpty()) {
    SetEnv("GST_PLUGIN_PATH", plugin_path);
    // Never load plugins from anywhere else.
    SetEnv("GST_PLUGIN_SYSTEM_PATH", plugin_path);
  }

  if (!registry_filename.isEmpty()) {
    SetEnv("GST_REGISTRY", registry_filename);
  }

  initialising_ = QtConcurrent::run(this, &GstEngine::InitialiseGstreamer);

  return true;
}

void GstEngine::InitialiseGstreamer() {
  gst_init(NULL, NULL);

#ifdef HAVE_IMOBILEDEVICE
  afcsrc_register_static();
#endif
}

void GstEngine::ReloadSettings() {
  Engine::Base::ReloadSettings();

  QSettings s;
  s.beginGroup(kSettingsGroup);

  sink_ = s.value("sink", kAutoSink).toString();
  device_ = s.value("device").toString();

  if (sink_.isEmpty())
    sink_ = kAutoSink;

  rg_enabled_ = s.value("rgenabled", false).toBool();
  rg_mode_ = s.value("rgmode", 0).toInt();
  rg_preamp_ = s.value("rgpreamp", 0.0).toDouble();
  rg_compression_ = s.value("rgcompression", true).toBool();

  buffer_duration_nanosec_ = s.value("bufferduration", 1000).toLongLong() * kNsecPerMsec;
}


bool GstEngine::CanDecode(const QUrl &url) {
  EnsureInitialised();

  // We had some bug reports claiming that video files cause crashes in canDecode(),
  // so don't try to decode them
  if ( url.path().toLower().endsWith( ".mov" ) ||
       url.path().toLower().endsWith( ".avi" ) ||
       url.path().toLower().endsWith( ".wmv" ) )
    return false;

  can_decode_success_ = false;
  can_decode_last_ = false;

  // Create the pipeline
  shared_ptr<GstElement> pipeline(gst_pipeline_new("pipeline"),
                                  boost::bind(gst_object_unref, _1));
  if (!pipeline) return false;
  GstElement* src = CreateElement("giosrc", pipeline.get());    if (!src) return false;
  GstElement* bin = CreateElement("decodebin2", pipeline.get()); if (!bin) return false;

  gst_element_link(src, bin);
  g_signal_connect(G_OBJECT(bin), "new-decoded-pad", G_CALLBACK(CanDecodeNewPadCallback), this);
  g_signal_connect(G_OBJECT(bin), "no-more-pads", G_CALLBACK(CanDecodeLastCallback), this);

  // These handlers just print out errors to stderr
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline.get())), CanDecodeBusCallbackSync, 0);
  gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(pipeline.get())), CanDecodeBusCallback, 0);

  // Set the file we're testing
  g_object_set(G_OBJECT(src), "location", url.toEncoded().constData(), NULL);

  // Start the pipeline playing
  gst_element_set_state(pipeline.get(), GST_STATE_PLAYING);

  // Wait until found audio stream
  int count = 0;
  while (!can_decode_success_ && !can_decode_last_ && count < 100) {
    count++;
    usleep(1000);
  }

  // Stop playing
  gst_element_set_state(pipeline.get(), GST_STATE_NULL);

  return can_decode_success_;
}



void GstEngine::CanDecodeNewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer self) {
  GstEngine* instance = reinterpret_cast<GstEngine*>(self);

  GstCaps* caps = gst_pad_get_caps(pad);
  if (gst_caps_get_size(caps) > 0) {
    GstStructure* str = gst_caps_get_structure(caps, 0);
    if (g_strrstr(gst_structure_get_name( str ), "audio" ))
      instance->can_decode_success_ = true;
  }
  gst_caps_unref(caps);
}

void GstEngine::CanDecodeLastCallback(GstElement*, gpointer self) {
  GstEngine* instance = reinterpret_cast<GstEngine*>(self);
  instance->can_decode_last_ = true;
}

void GstEngine::PrintGstError(GstMessage *msg) {
  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  qDebug() << error->message;
  qDebug() << debugs;

  g_error_free(error);
  free(debugs);
}

GstBusSyncReply GstEngine::CanDecodeBusCallbackSync(GstBus*, GstMessage* msg, gpointer) {
  if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR)
    PrintGstError(msg);
  return GST_BUS_PASS;
}

gboolean GstEngine::CanDecodeBusCallback(GstBus*, GstMessage* msg, gpointer) {
  if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR)
    PrintGstError(msg);
  return GST_BUS_DROP;
}


qint64 GstEngine::position_nanosec() const {
  if (!current_pipeline_)
    return 0;

  qint64 result = current_pipeline_->position() - beginning_nanosec_;
  return qint64(qMax(0ll, result));
}

qint64 GstEngine::length_nanosec() const {
  if (!current_pipeline_)
    return 0;

  qint64 result = end_nanosec_ - beginning_nanosec_;
  return qint64(qMax(0ll, result));
}

Engine::State GstEngine::state() const {
  if (!current_pipeline_)
    return url_.isEmpty() ? Engine::Empty : Engine::Idle;

  switch (current_pipeline_->state()) {
    case GST_STATE_NULL:    return Engine::Empty;
    case GST_STATE_READY:   return Engine::Idle;
    case GST_STATE_PLAYING: return Engine::Playing;
    case GST_STATE_PAUSED:  return Engine::Paused;
    default:                return Engine::Empty;
  }
}

void GstEngine::ConsumeBuffer(GstBuffer *buffer, GstEnginePipeline* pipeline) {
  // Schedule this to run in the GUI thread.  The buffer gets added to the
  // queue and unreffed by UpdateScope.
  if (!QMetaObject::invokeMethod(this, "AddBufferToScope",
                                 Q_ARG(GstBuffer*, buffer),
                                 Q_ARG(GstEnginePipeline*, pipeline))) {
    qWarning() << "Failed to invoke AddBufferToScope on GstEngine";
  }
}

void GstEngine::AddBufferToScope(GstBuffer* buf, GstEnginePipeline* pipeline) {
  if (current_pipeline_.get() != pipeline) {
    gst_buffer_unref(buf);
    return;
  }

  g_queue_push_tail(delayq_, buf);
}

const Engine::Scope& GstEngine::scope() {
  UpdateScope();

  if (current_sample_ >= kScopeSize) {
    // ok, we have a full buffer now, so give it to the scope
    for (int i=0; i< kScopeSize; i++)
      scope_[i] = current_scope_[i];
    current_sample_ = 0;
  }

  return scope_;
}

void GstEngine::UpdateScope() {
  typedef int16_t sampletype;

  // prune the scope and get the current pos of the audio device
  const quint64 pos = PruneScope();
  const quint64 segment_start = current_pipeline_->segment_start();

  // head of the delay queue is the most delayed, so we work with that one
  GstBuffer *buf = reinterpret_cast<GstBuffer *>( g_queue_peek_head(delayq_) );
  if (!buf)
    return;

  // start time for this buffer
  quint64 stime = GST_BUFFER_TIMESTAMP(buf) - segment_start;
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
  while (buf && current_sample_ < kScopeSize && i < sz) {
    for (int j = 0; j < channels && current_sample_ < kScopeSize; j++) {
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

void GstEngine::StartPreloading(const QUrl& url) {
  EnsureInitialised();

  QUrl gst_url = FixupUrl(url);

  if (autocrossfade_enabled_) {
    // Have to create a new pipeline so we can crossfade between the two

    preload_pipeline_ = CreatePipeline(gst_url);
    if (!preload_pipeline_)
      return;

    // We don't want to get metadata messages before the track starts playing -
    // we reconnect this in GstEngine::Load
    disconnect(preload_pipeline_.get(), SIGNAL(MetadataFound(Engine::SimpleMetaBundle)), this, 0);

    preloaded_url_ = gst_url;
    preload_pipeline_->SetState(GST_STATE_PAUSED);
  } else {
    // No crossfading, so we can just queue the new URL in the existing
    // pipeline and get gapless playback (hopefully)
    if (current_pipeline_)
      current_pipeline_->SetNextUrl(gst_url);
  }
}

QUrl GstEngine::FixupUrl(const QUrl& url) {
  QUrl copy = url;

  // It's a file:// url with a hostname set.  QUrl::fromLocalFile does this
  // when given a \\host\share\file path on Windows.  Munge it back into a
  // path that gstreamer will recognise.
  if (url.scheme() == "file" && !url.host().isEmpty()) {
    copy.setPath("//" + copy.host() + copy.path());
    copy.setHost(QString());
  }

  return copy;
}

bool GstEngine::Load(const QUrl& url, Engine::TrackChangeType change,
                     quint64 beginning_nanosec, qint64 end_nanosec) {
  EnsureInitialised();

  Engine::Base::Load(url, change, beginning_nanosec, end_nanosec);

  // Clementine just crashes when asked to load a file that doesn't exist on
  // Windows, so check for that here.  This is definitely the wrong place for
  // this "fix"...
  if (url.scheme() == "file" && !QFile::exists(url.toLocalFile()))
    return false;

  QUrl gst_url = FixupUrl(url);

  const bool crossfade = current_pipeline_ &&
                         ((crossfade_enabled_ && change == Engine::Manual) ||
                          (autocrossfade_enabled_ && change == Engine::Auto));

  if (!crossfade && current_pipeline_ && current_pipeline_->url() == gst_url &&
      change == Engine::Auto) {
    // We're not crossfading, and the pipeline is already playing the URI we
    // want, so just do nothing.
    return true;
  }

  shared_ptr<GstEnginePipeline> pipeline;
  if (preload_pipeline_ && preloaded_url_ == gst_url) {
    pipeline = preload_pipeline_;
    connect(preload_pipeline_.get(),
            SIGNAL(MetadataFound(Engine::SimpleMetaBundle)),
            SLOT(NewMetaData(Engine::SimpleMetaBundle)));
  } else {
    pipeline = CreatePipeline(gst_url);
    if (!pipeline)
      return false;
  }

  if (crossfade)
    StartFadeout();

  current_pipeline_ = pipeline;
  preload_pipeline_.reset();

  SetVolume(volume_);
  SetEqualizerEnabled(equalizer_enabled_);
  SetEqualizerParameters(equalizer_preamp_, equalizer_gains_);

  // Maybe fade in this track
  if (crossfade)
    current_pipeline_->StartFader(fadeout_duration_nanosec_, QTimeLine::Forward);

  return true;
}

void GstEngine::StartFadeout() {
  fadeout_pipeline_ = current_pipeline_;
  disconnect(fadeout_pipeline_.get(), 0, 0, 0);
  fadeout_pipeline_->RemoveAllBufferConsumers();
  ClearScopeBuffers();

  fadeout_pipeline_->StartFader(fadeout_duration_nanosec_, QTimeLine::Backward);
  connect(fadeout_pipeline_.get(), SIGNAL(FaderFinished()), SLOT(FadeoutFinished()));
}


bool GstEngine::Play(quint64 offset_nanosec) {
  EnsureInitialised();

  if (!current_pipeline_)
    return false;

  QFuture<GstStateChangeReturn> future = current_pipeline_->SetState(GST_STATE_PLAYING);
  BoundFutureWatcher<GstStateChangeReturn, quint64>* watcher =
      new BoundFutureWatcher<GstStateChangeReturn, quint64>(offset_nanosec, this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(PlayDone()));

  return true;
}

void GstEngine::PlayDone() {
  BoundFutureWatcher<GstStateChangeReturn, quint64>* watcher =
      static_cast<BoundFutureWatcher<GstStateChangeReturn, quint64>*>(sender());
  watcher->deleteLater();

  GstStateChangeReturn ret = watcher->result();
  quint64 offset_nanosec = watcher->data();

  if (!current_pipeline_)
    return;

  if (ret == GST_STATE_CHANGE_FAILURE) {
    // Failure, but we got a redirection URL - try loading that instead
    QUrl redirect_url = current_pipeline_->redirect_url();
    if (!redirect_url.isEmpty() && redirect_url != current_pipeline_->url()) {
      qDebug() << "Redirecting to" << redirect_url;
      current_pipeline_ = CreatePipeline(redirect_url);
      Play(offset_nanosec);
      return;
    }

    // Failure - give up
    qWarning() << "Could not set thread to PLAYING.";
    current_pipeline_.reset();
    return;
  }

  StartTimers();

  current_sample_ = 0;

  // initial offset
  if(offset_nanosec != 0 || beginning_nanosec_ != 0) {
    Seek(offset_nanosec);
  }

  emit StateChanged(Engine::Playing);
}


void GstEngine::Stop() {
  StopTimers();

  url_ = QUrl(); // To ensure we return Empty from state()
  beginning_nanosec_ = end_nanosec_ = 0;

  if (fadeout_enabled_ && current_pipeline_)
    StartFadeout();

  current_pipeline_.reset();
  emit StateChanged(Engine::Empty);
}

void GstEngine::FadeoutFinished() {
  fadeout_pipeline_.reset();
}

void GstEngine::Pause() {
  if (!current_pipeline_)
    return;

  if ( current_pipeline_->state() == GST_STATE_PLAYING ) {
    current_pipeline_->SetState(GST_STATE_PAUSED);
    emit StateChanged(Engine::Paused);

    StopTimers();
  }
}

void GstEngine::Unpause() {
  if (!current_pipeline_)
    return;

  if ( current_pipeline_->state() == GST_STATE_PAUSED ) {
    current_pipeline_->SetState(GST_STATE_PLAYING);
    emit StateChanged(Engine::Playing);

    StartTimers();
  }
}

void GstEngine::Seek(quint64 offset_nanosec) {
  if (!current_pipeline_)
    return;

  seek_pos_ = beginning_nanosec_ + offset_nanosec;
  waiting_to_seek_ = true;

  if (!seek_timer_->isActive()) {
    SeekNow();
    seek_timer_->start(); // Stop us from seeking again for a little while
  }
}

void GstEngine::SeekNow() {
  if (!waiting_to_seek_) return;
  waiting_to_seek_ = false;

  if (!current_pipeline_)
    return;

  if (current_pipeline_->Seek(seek_pos_))
    ClearScopeBuffers();
  else
    qDebug() << "Seek failed";
}

void GstEngine::SetEqualizerEnabled(bool enabled) {
  equalizer_enabled_= enabled;

  if (current_pipeline_)
    current_pipeline_->SetEqualizerEnabled(enabled);
}


void GstEngine::SetEqualizerParameters(int preamp, const QList<int>& band_gains) {
  equalizer_preamp_ = preamp;
  equalizer_gains_ = band_gains;

  if (current_pipeline_)
    current_pipeline_->SetEqualizerParams(preamp, band_gains);
}

void GstEngine::SetVolumeSW( uint percent ) {
  if (current_pipeline_)
    current_pipeline_->SetVolume(percent);
}

void GstEngine::StartTimers() {
  StopTimers();

  timer_id_ = startTimer(kTimerIntervalNanosec / kNsecPerMsec);
}

void GstEngine::StopTimers() {
  if (timer_id_ != -1) {
    killTimer(timer_id_);
    timer_id_ = -1;
  }
}

void GstEngine::timerEvent(QTimerEvent* e) {
  if (e->timerId() != timer_id_)
    return;

  // keep the scope from building while we are not visible
  // this is why the timer must run as long as we are playing, and not just when
  // we are fading
  PruneScope();

  if (current_pipeline_) {
    const qint64 current_position = position_nanosec();
    const qint64 current_length = length_nanosec();

    const qint64 remaining = current_length - current_position;

    const qint64 fudge = kTimerIntervalNanosec + 100 * kNsecPerMsec; // Mmm fudge
    const qint64 gap = autocrossfade_enabled_ ? fadeout_duration_nanosec_ : kPreloadGapNanosec;

    // only if we know the length of the current stream...
    if(current_length > 0) {
      // emit TrackAboutToEnd when we're a few seconds away from finishing
      if (remaining < gap + fudge) {
        EmitAboutToEnd();
      }

      // when at the end, kill the track if it didn't stop yet (probably a
      // multisection media file).  We add 1 second onto the length during this
      // check to allow for the fact that the length has been rounded down to
      // the nearest second, and to stop us from occasionally stopping the
      // stream just before it ends normally.
      if(current_position >= current_length + 1000 * kNsecPerMsec) {
        EndOfStreamReached(current_pipeline_->has_next_valid_url());
      }
    }
  }
}

void GstEngine::HandlePipelineError(const QString& message) {
  qWarning() << "Gstreamer error:" << message;

  current_pipeline_.reset();
  emit Error(message);
  emit StateChanged(Engine::Empty);
}


void GstEngine::EndOfStreamReached(bool has_next_track) {
  if (!has_next_track)
    current_pipeline_.reset();
  ClearScopeBuffers();
  emit TrackEnded();
}

void GstEngine::NewMetaData(const Engine::SimpleMetaBundle& bundle) {
  emit MetaData(bundle);
}

GstElement* GstEngine::CreateElement(const QString& factoryName, GstElement* bin) {
  // Make a unique name
  QString name = factoryName + "-" + QString::number(next_element_id_ ++);

  GstElement* element = gst_element_factory_make(
      factoryName.toAscii().constData(), name.toAscii().constData());

  if (!element) {
    emit Error(QString("GStreamer could not create the element: %1.  "
                       "Please make sure that you have installed all necessary GStreamer plugins (e.g. OGG and MP3)").arg( factoryName ) );
    gst_object_unref(GST_OBJECT(bin));
    return NULL;
  }

  if (bin)
    gst_bin_add(GST_BIN(bin), element);

  return element;
}


GstEngine::PluginDetailsList
    GstEngine::GetPluginList(const QString& classname) const {
  const_cast<GstEngine*>(this)->EnsureInitialised();

  PluginDetailsList ret;

  GstRegistry* registry = gst_registry_get_default();
  GList* const features =
      gst_registry_get_feature_list(registry, GST_TYPE_ELEMENT_FACTORY);

  GList* p = features;
  while (p) {
    GstElementFactory* factory = GST_ELEMENT_FACTORY(p->data);
    if (QString(factory->details.klass).contains(classname)) {
      PluginDetails details;
      details.name = QString::fromUtf8(GST_PLUGIN_FEATURE_NAME(p->data));
      details.long_name = QString::fromUtf8(factory->details.longname);
      details.description = QString::fromUtf8(factory->details.description);
      details.author = QString::fromUtf8(factory->details.author);
      ret << details;
    }
    p = g_list_next(p);
  }

  gst_plugin_feature_list_free(features);
  return ret;
}

shared_ptr<GstEnginePipeline> GstEngine::CreatePipeline() {
  EnsureInitialised();

  shared_ptr<GstEnginePipeline> ret(new GstEnginePipeline(this));
  ret->set_output_device(sink_, device_);
  ret->set_replaygain(rg_enabled_, rg_mode_, rg_preamp_, rg_compression_);
  ret->set_buffer_duration_nanosec(buffer_duration_nanosec_);

  ret->AddBufferConsumer(this);
  foreach (BufferConsumer* consumer, buffer_consumers_)
    ret->AddBufferConsumer(consumer);

  connect(ret.get(), SIGNAL(EndOfStreamReached(bool)), SLOT(EndOfStreamReached(bool)));
  connect(ret.get(), SIGNAL(Error(QString)), SLOT(HandlePipelineError(QString)));
  connect(ret.get(), SIGNAL(MetadataFound(Engine::SimpleMetaBundle)),
          SLOT(NewMetaData(Engine::SimpleMetaBundle)));
  connect(ret.get(), SIGNAL(destroyed()), SLOT(ClearScopeBuffers()));

  return ret;
}

shared_ptr<GstEnginePipeline> GstEngine::CreatePipeline(const QUrl& url) {
  shared_ptr<GstEnginePipeline> ret = CreatePipeline();

  if (url.scheme() == "hypnotoad") {
    ret->InitFromString(kHypnotoadPipeline);
    return ret;
  }

  if (!ret->InitFromUrl(url))
    ret.reset();

  return ret;
}

qint64 GstEngine::PruneScope() {
  if (!current_pipeline_)
    return 0;

  // get the position playing in the audio device
  const qint64 pos = position_nanosec();
  const qint64 segment_start = current_pipeline_->segment_start();

  GstBuffer *buf = 0;
  quint64 etime = 0;

  // free up the buffers that the audio device has advanced past already
  do {
    // most delayed buffers are at the head of the queue
    buf = reinterpret_cast<GstBuffer *>( g_queue_peek_head(delayq_) );
    if (buf) {
      // the start time of the buffer
      quint64 stime = GST_BUFFER_TIMESTAMP(buf) - segment_start;
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

void GstEngine::ClearScopeBuffers() {
  // just free them all
  while (g_queue_get_length(delayq_)) {
    GstBuffer* buf = reinterpret_cast<GstBuffer *>( g_queue_pop_head(delayq_) );
    gst_buffer_unref(buf);
  }
}

bool GstEngine::DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(const QString &name) {
  return (name == "alsasink" || name == "osssink" || name == "pulsesink");
}

void GstEngine::AddBufferConsumer(BufferConsumer *consumer) {
  buffer_consumers_ << consumer;
  if (current_pipeline_)
    current_pipeline_->AddBufferConsumer(consumer);
}

void GstEngine::RemoveBufferConsumer(BufferConsumer *consumer) {
  buffer_consumers_.removeAll(consumer);
  if (current_pipeline_)
    current_pipeline_->RemoveBufferConsumer(consumer);
}

int GstEngine::AddBackgroundStream(shared_ptr<GstEnginePipeline> pipeline) {
  // We don't want to get metadata messages or end notifications.
  disconnect(pipeline.get(), SIGNAL(MetadataFound(Engine::SimpleMetaBundle)), this, 0);
  disconnect(pipeline.get(), SIGNAL(EndOfStreamReached(bool)), this, 0);
  connect(pipeline.get(), SIGNAL(EndOfStreamReached(bool)), SLOT(BackgroundStreamFinished()));

  const int stream_id = next_background_stream_id_++;
  background_streams_[stream_id] = pipeline;

  QFuture<GstStateChangeReturn> future = pipeline->SetState(GST_STATE_PLAYING);
  BoundFutureWatcher<GstStateChangeReturn, int>* watcher =
      new BoundFutureWatcher<GstStateChangeReturn, int>(stream_id, this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(BackgroundStreamPlayDone()));

  return stream_id;
}

void GstEngine::BackgroundStreamPlayDone() {
  BoundFutureWatcher<GstStateChangeReturn, int>* watcher =
      static_cast<BoundFutureWatcher<GstStateChangeReturn, int>*>(sender());
  watcher->deleteLater();

  const int stream_id = watcher->data();
  GstStateChangeReturn ret = watcher->result();

  if (ret == GST_STATE_CHANGE_FAILURE) {
    qWarning() << "Could not set thread to PLAYING.";
    background_streams_.remove(stream_id);
  }
}

int GstEngine::AddBackgroundStream(const QUrl& url) {
  shared_ptr<GstEnginePipeline> pipeline = CreatePipeline(url);
  if (!pipeline) {
    return -1;
  }
  pipeline->SetVolume(30);
  pipeline->SetNextUrl(url);
  return AddBackgroundStream(pipeline);
}

void GstEngine::StopBackgroundStream(int id) {
  background_streams_.remove(id);  // Removes last shared_ptr reference.
}

void GstEngine::BackgroundStreamFinished() {
  GstEnginePipeline* pipeline = qobject_cast<GstEnginePipeline*>(sender());
  pipeline->SetNextUrl(pipeline->url());
}

void GstEngine::SetBackgroundStreamVolume(int id, int volume) {
  shared_ptr<GstEnginePipeline> pipeline = background_streams_[id];
  Q_ASSERT(pipeline);
  pipeline->SetVolume(volume);
}
