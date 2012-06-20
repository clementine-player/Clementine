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

#include "config.h"
#include "gstengine.h"
#include "gstenginepipeline.h"
#include "core/logging.h"
#include "core/taskmanager.h"
#include "core/utilities.h"

#ifdef HAVE_IMOBILEDEVICE
# include "gst/afcsrc/gstafcsrc.h"
#endif

#ifdef HAVE_MOODBAR
# include "gst/moodbar/spectrum.h"
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
const char* GstEngine::kEnterprisePipeline =
      "audiotestsrc wave=5 ! "
      "audiocheblimit mode=0 cutoff=120";

GstEngine::GstEngine(TaskManager* task_manager)
  : Engine::Base(),
    task_manager_(task_manager),
    buffering_task_id_(-1),
    delayq_(g_queue_new()),
    current_sample_(0),
    equalizer_enabled_(false),
    rg_enabled_(false),
    rg_mode_(0),
    rg_preamp_(0.0),
    rg_compression_(true),
    buffer_duration_nanosec_(1 * kNsecPerSec), // 1s
    mono_playback_(false),
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

bool GstEngine::Init() {
  initialising_ = QtConcurrent::run(&GstEngine::InitialiseGstreamer);
  return true;
}

void GstEngine::InitialiseGstreamer() {
  gst_init(NULL, NULL);

#ifdef HAVE_IMOBILEDEVICE
  afcsrc_register_static();
#endif

#ifdef HAVE_MOODBAR
  gstmoodbar_register_static();
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

  buffer_duration_nanosec_ = s.value("bufferduration", 4000).toLongLong() * kNsecPerMsec;

  mono_playback_ = s.value("monoplayback", false).toBool();
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

void GstEngine::ConsumeBuffer(GstBuffer* buffer, int pipeline_id) {
  // Schedule this to run in the GUI thread.  The buffer gets added to the
  // queue and unreffed by UpdateScope.
  if (!QMetaObject::invokeMethod(this, "AddBufferToScope",
                                 Q_ARG(GstBuffer*, buffer),
                                 Q_ARG(int, pipeline_id))) {
    qLog(Warning) << "Failed to invoke AddBufferToScope on GstEngine";
  }
}

void GstEngine::AddBufferToScope(GstBuffer* buf, int pipeline_id) {
  if (!current_pipeline_ || current_pipeline_->id() != pipeline_id) {
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

void GstEngine::StartPreloading(const QUrl& url, bool force_stop_at_end,
                                qint64 beginning_nanosec, qint64 end_nanosec) {
  EnsureInitialised();

  QUrl gst_url = FixupUrl(url);

  // No crossfading, so we can just queue the new URL in the existing
  // pipeline and get gapless playback (hopefully)
  if (current_pipeline_)
    current_pipeline_->SetNextUrl(gst_url, beginning_nanosec,
        force_stop_at_end ? end_nanosec : 0);
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

bool GstEngine::Load(const QUrl& url, Engine::TrackChangeFlags change,
                     bool force_stop_at_end,
                     quint64 beginning_nanosec, qint64 end_nanosec) {
  EnsureInitialised();

  Engine::Base::Load(url, change, force_stop_at_end, beginning_nanosec, end_nanosec);

  QUrl gst_url = FixupUrl(url);

  bool crossfade = current_pipeline_ &&
                   ((crossfade_enabled_ && change & Engine::Manual) ||
                    (autocrossfade_enabled_ && change & Engine::Auto));

  if (change & Engine::Auto && change & Engine::SameAlbum && !crossfade_same_album_)
    crossfade = false;

  if (!crossfade && current_pipeline_ && current_pipeline_->url() == gst_url &&
      change & Engine::Auto) {
    // We're not crossfading, and the pipeline is already playing the URI we
    // want, so just do nothing.
    return true;
  }

  shared_ptr<GstEnginePipeline> pipeline = CreatePipeline(gst_url,
      force_stop_at_end ? end_nanosec : 0);
  if (!pipeline)
    return false;

  if (crossfade)
    StartFadeout();

  BufferingFinished();
  current_pipeline_ = pipeline;

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

  if (!current_pipeline_ || current_pipeline_->is_buffering())
    return false;

  QFuture<GstStateChangeReturn> future = current_pipeline_->SetState(GST_STATE_PLAYING);
  PlayFutureWatcher* watcher = new PlayFutureWatcher(
        PlayFutureWatcherArg(offset_nanosec, current_pipeline_->id()), this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(PlayDone()));

  return true;
}

void GstEngine::PlayDone() {
  PlayFutureWatcher* watcher = static_cast<PlayFutureWatcher*>(sender());
  watcher->deleteLater();

  GstStateChangeReturn ret = watcher->result();
  quint64 offset_nanosec = watcher->data().first;

  if (!current_pipeline_ || watcher->data().second != current_pipeline_->id()) {
    return;
  }

  if (ret == GST_STATE_CHANGE_FAILURE) {
    // Failure, but we got a redirection URL - try loading that instead
    QUrl redirect_url = current_pipeline_->redirect_url();
    if (!redirect_url.isEmpty() && redirect_url != current_pipeline_->url()) {
      qLog(Info) << "Redirecting to" << redirect_url;
      current_pipeline_ = CreatePipeline(redirect_url, end_nanosec_);
      Play(offset_nanosec);
      return;
    }

    // Failure - give up
    qLog(Warning) << "Could not set thread to PLAYING.";
    current_pipeline_.reset();
    BufferingFinished();
    return;
  }

  StartTimers();

  current_sample_ = 0;

  // initial offset
  if(offset_nanosec != 0 || beginning_nanosec_ != 0) {
    Seek(offset_nanosec);
  }

  emit StateChanged(Engine::Playing);
  // we've successfully started playing a media stream with this url
  emit ValidSongRequested(url_);
}


void GstEngine::Stop() {
  StopTimers();

  url_ = QUrl(); // To ensure we return Empty from state()
  beginning_nanosec_ = end_nanosec_ = 0;

  if (fadeout_enabled_ && current_pipeline_)
    StartFadeout();

  current_pipeline_.reset();
  BufferingFinished();
  emit StateChanged(Engine::Empty);
}

void GstEngine::FadeoutFinished() {
  fadeout_pipeline_.reset();
  emit FadeoutFinishedSignal();
}

void GstEngine::Pause() {
  if (!current_pipeline_ || current_pipeline_->is_buffering())
    return;

  if ( current_pipeline_->state() == GST_STATE_PLAYING ) {
    current_pipeline_->SetState(GST_STATE_PAUSED);
    emit StateChanged(Engine::Paused);

    StopTimers();
  }
}

void GstEngine::Unpause() {
  if (!current_pipeline_ || current_pipeline_->is_buffering())
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
    qLog(Warning) << "Seek failed";
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
    }
  }
}

void GstEngine::HandlePipelineError(int pipeline_id, const QString& message,
                                    int domain, int error_code) {
  if (!current_pipeline_.get() || current_pipeline_->id() != pipeline_id)
    return;

  qLog(Warning) << "Gstreamer error:" << message;

  current_pipeline_.reset();

  BufferingFinished();
  emit StateChanged(Engine::Empty);
  // unable to play media stream with this url
  emit InvalidSongRequested(url_);

  // TODO: the types of errors listed below won't be shown to user - they will 
  // get logged and the current song will be skipped; instead of maintaining 
  // the list we should probably:
  // - don't report any engine's errors to user (always just log and skip)
  // - come up with a less intrusive error box (not a dialog but a notification
  //   popup of some kind) and then report all errors
  if(!(domain == GST_RESOURCE_ERROR && error_code == GST_RESOURCE_ERROR_NOT_FOUND) &&
     !(domain == GST_STREAM_ERROR && error_code == GST_STREAM_ERROR_TYPE_NOT_FOUND) &&
     !(domain == GST_RESOURCE_ERROR && error_code == GST_RESOURCE_ERROR_OPEN_READ)) {
    emit Error(message);
  }
}

void GstEngine::EndOfStreamReached(int pipeline_id, bool has_next_track) {
  if (!current_pipeline_.get() || current_pipeline_->id() != pipeline_id)
    return;

  if (!has_next_track) {
    current_pipeline_.reset();
    BufferingFinished();
  }
  ClearScopeBuffers();
  emit TrackEnded();
}

void GstEngine::NewMetaData(int pipeline_id, const Engine::SimpleMetaBundle& bundle) {
  if (!current_pipeline_.get() || current_pipeline_->id() != pipeline_id)
    return;

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
  ret->set_mono_playback(mono_playback_);

  ret->AddBufferConsumer(this);
  foreach (BufferConsumer* consumer, buffer_consumers_) {
    ret->AddBufferConsumer(consumer);
  }

  connect(ret.get(), SIGNAL(EndOfStreamReached(int, bool)), SLOT(EndOfStreamReached(int, bool)));
  connect(ret.get(), SIGNAL(Error(int, QString,int,int)), SLOT(HandlePipelineError(int, QString,int,int)));
  connect(ret.get(), SIGNAL(MetadataFound(int, Engine::SimpleMetaBundle)),
          SLOT(NewMetaData(int, Engine::SimpleMetaBundle)));
  connect(ret.get(), SIGNAL(destroyed()), SLOT(ClearScopeBuffers()));
  connect(ret.get(), SIGNAL(BufferingStarted()), SLOT(BufferingStarted()));
  connect(ret.get(), SIGNAL(BufferingProgress(int)), SLOT(BufferingProgress(int)));
  connect(ret.get(), SIGNAL(BufferingFinished()), SLOT(BufferingFinished()));

  return ret;
}

shared_ptr<GstEnginePipeline> GstEngine::CreatePipeline(const QUrl& url,
                                                        qint64 end_nanosec) {
  shared_ptr<GstEnginePipeline> ret = CreatePipeline();

  if (url.scheme() == "hypnotoad") {
    ret->InitFromString(kHypnotoadPipeline);
    return ret;
  }

  if (url.scheme() == "enterprise") {
    ret->InitFromString(kEnterprisePipeline);
    return ret;
  }

  if (!ret->InitFromUrl(url, end_nanosec))
    ret.reset();

  return ret;
}

qint64 GstEngine::PruneScope() {
  if (!current_pipeline_)
    return 0;

  // get the position playing in the audio device
  const qint64 pos = current_pipeline_->position();
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
  disconnect(pipeline.get(), SIGNAL(MetadataFound(int,Engine::SimpleMetaBundle)), this, 0);
  disconnect(pipeline.get(), SIGNAL(EndOfStreamReached(int,bool)), this, 0);
  connect(pipeline.get(), SIGNAL(EndOfStreamReached(int,bool)), SLOT(BackgroundStreamFinished()));

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
    qLog(Warning) << "Could not set thread to PLAYING.";
    background_streams_.remove(stream_id);
  }
}

int GstEngine::AddBackgroundStream(const QUrl& url) {
  shared_ptr<GstEnginePipeline> pipeline = CreatePipeline(url, 0);
  if (!pipeline) {
    return -1;
  }
  pipeline->SetVolume(30);
  pipeline->SetNextUrl(url, 0, 0);
  return AddBackgroundStream(pipeline);
}

void GstEngine::StopBackgroundStream(int id) {
  background_streams_.remove(id);  // Removes last shared_ptr reference.
}

void GstEngine::BackgroundStreamFinished() {
  GstEnginePipeline* pipeline = qobject_cast<GstEnginePipeline*>(sender());
  pipeline->SetNextUrl(pipeline->url(), 0, 0);
}

void GstEngine::SetBackgroundStreamVolume(int id, int volume) {
  shared_ptr<GstEnginePipeline> pipeline = background_streams_[id];
  Q_ASSERT(pipeline);
  pipeline->SetVolume(volume);
}

void GstEngine::BufferingStarted() {
  if (buffering_task_id_ != -1) {
    task_manager_->SetTaskFinished(buffering_task_id_);
  }

  buffering_task_id_ = task_manager_->StartTask(tr("Buffering"));
  task_manager_->SetTaskProgress(buffering_task_id_, 0, 100);
}

void GstEngine::BufferingProgress(int percent) {
  task_manager_->SetTaskProgress(buffering_task_id_, percent, 100);
}

void GstEngine::BufferingFinished() {
  if (buffering_task_id_ != -1) {
    task_manager_->SetTaskFinished(buffering_task_id_);
    buffering_task_id_ = -1;
  }
}
