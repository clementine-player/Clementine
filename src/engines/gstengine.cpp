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

#include "gstengine.h"

#include <math.h>
#include <cmath>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <vector>

#include <QTimer>
#include <QRegExp>
#include <QFile>
#include <QSettings>
#include <QCoreApplication>
#include <QTimeLine>
#include <QDir>
#include <QtConcurrentRun>

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

#include "config.h"
#include "devicefinder.h"
#include "gstenginepipeline.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "core/utilities.h"

#ifdef HAVE_MOODBAR
#include "gst/moodbar/plugin.h"
#endif

#ifdef HAVE_LIBPULSE
#include "engines/pulsedevicefinder.h"
#endif

#ifdef Q_OS_DARWIN
#include "engines/osxdevicefinder.h"
#endif

#ifdef Q_OS_WIN32
#include "engines/directsounddevicefinder.h"
#endif

#ifdef Q_OS_DARWIN
#include "core/mac_startup.h"
#endif

#ifdef Q_OS_DARWIN
#undef signals
#include <gio/gio.h>
#endif

using std::shared_ptr;
using std::vector;

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
      latest_buffer_(nullptr),
      equalizer_enabled_(false),
      stereo_balance_(0.0f),
      rg_enabled_(false),
      rg_mode_(0),
      rg_preamp_(0.0),
      rg_compression_(true),
      buffer_duration_nanosec_(1 * kNsecPerSec),  // 1s
      buffer_min_fill_(33),
      mono_playback_(false),
      sample_rate_(kAutoSampleRate),
      seek_timer_(new QTimer(this)),
      timer_id_(-1),
      next_element_id_(0),
      is_fading_out_to_pause_(false),
      has_faded_out_(false),
      scope_chunk_(0),
      have_new_buffer_(false) {
  seek_timer_->setSingleShot(true);
  seek_timer_->setInterval(kSeekDelayNanosec / kNsecPerMsec);
  connect(seek_timer_, SIGNAL(timeout()), SLOT(SeekNow()));

  ReloadSettings();

#ifdef Q_OS_DARWIN
  QDir resources_dir(mac::GetResourcesPath());
  QString ca_cert_path = resources_dir.filePath("cacert.pem");
  GError* error = nullptr;
  tls_database_ = g_tls_file_database_new(ca_cert_path.toUtf8().data(), &error);
#endif
}

GstEngine::~GstEngine() {
  EnsureInitialised();

  current_pipeline_.reset();

  qDeleteAll(device_finders_);

#ifdef Q_OS_DARWIN
  g_object_unref(tls_database_);
#endif
}

bool GstEngine::Init() {
  initialising_ = QtConcurrent::run(this, &GstEngine::InitialiseGstreamer);
  return true;
}

void GstEngine::InitialiseGstreamer() {
  gst_init(nullptr, nullptr);

  gst_pb_utils_init();

#ifdef HAVE_MOODBAR
  gstfastspectrum_register_static();
#endif

  QSet<QString> plugin_names;
  for (const PluginDetails& plugin : GetPluginList("Sink/Audio")) {
    plugin_names.insert(plugin.name);
  }

  QList<DeviceFinder*> device_finders;
#ifdef HAVE_LIBPULSE
  device_finders.append(new PulseDeviceFinder);
#endif
#ifdef Q_OS_DARWIN
  device_finders.append(new OsxDeviceFinder);
#endif
#ifdef Q_OS_WIN32
  device_finders.append(new DirectSoundDeviceFinder);
#endif

  for (DeviceFinder* finder : device_finders) {
    if (!plugin_names.contains(finder->gstreamer_sink())) {
      qLog(Info) << "Skipping DeviceFinder for" << finder->gstreamer_sink()
                 << "known plugins:" << plugin_names;
      delete finder;
      continue;
    }
    if (!finder->Initialise()) {
      qLog(Warning) << "Failed to initialise DeviceFinder for"
                    << finder->gstreamer_sink();
      delete finder;
      continue;
    }

    device_finders_.append(finder);
  }
}

void GstEngine::ReloadSettings() {
  Engine::Base::ReloadSettings();

  QSettings s;
  s.beginGroup(kSettingsGroup);

  sink_ = s.value("sink", kAutoSink).toString();
  device_ = s.value("device");

  if (sink_.isEmpty()) sink_ = kAutoSink;

  rg_enabled_ = s.value("rgenabled", false).toBool();
  rg_mode_ = s.value("rgmode", 0).toInt();
  rg_preamp_ = s.value("rgpreamp", 0.0).toDouble();
  rg_compression_ = s.value("rgcompression", true).toBool();

  buffer_duration_nanosec_ =
      s.value("bufferduration", 4000).toLongLong() * kNsecPerMsec;

  buffer_min_fill_ = s.value("bufferminfill", 33).toInt();

  mono_playback_ = s.value("monoplayback", false).toBool();
  sample_rate_ = s.value("samplerate", kAutoSampleRate).toInt();
}

qint64 GstEngine::position_nanosec() const {
  if (!current_pipeline_) return 0;

  const qint64 result = current_pipeline_->position() - beginning_nanosec_;
  return qint64(qMax(0ll, result));
}

qint64 GstEngine::length_nanosec() const {
  if (!current_pipeline_) return 0;

  const qint64 result = end_nanosec_ - beginning_nanosec_;

  if (result > 0) {
    return result;
  } else {
    // Get the length from the pipeline if we don't know.
    return current_pipeline_->length();
  }
}

Engine::State GstEngine::state() const {
  if (!current_pipeline_) return url_.isEmpty() ? Engine::Empty : Engine::Idle;

  switch (current_pipeline_->state()) {
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

  if (latest_buffer_ != nullptr) {
    gst_buffer_unref(latest_buffer_);
  }

  latest_buffer_ = buf;
  have_new_buffer_ = true;
}

const Engine::Scope& GstEngine::scope(int chunk_length) {
  // the new buffer could have a different size
  if (have_new_buffer_) {
    if (latest_buffer_ != nullptr) {
      scope_chunks_ = ceil(((double)GST_BUFFER_DURATION(latest_buffer_) /
                            (double)(chunk_length * kNsecPerMsec)));
    }

    // if the buffer is shorter than the chunk length
    if (scope_chunks_ <= 0) {
      scope_chunks_ = 1;
    }

    scope_chunk_ = 0;
    have_new_buffer_ = false;
  }

  if (latest_buffer_ != nullptr) {
    UpdateScope(chunk_length);
  }

  return scope_;
}

void GstEngine::UpdateScope(int chunk_length) {
  typedef Engine::Scope::value_type sample_type;

  // prevent dbz or invalid chunk size
  if (!GST_CLOCK_TIME_IS_VALID(GST_BUFFER_DURATION(latest_buffer_))) return;
  if (GST_BUFFER_DURATION(latest_buffer_) == 0) return;

  GstMapInfo map;
  gst_buffer_map(latest_buffer_, &map, GST_MAP_READ);

  // determine where to split the buffer
  int chunk_density =
      (map.size * kNsecPerMsec) / GST_BUFFER_DURATION(latest_buffer_);

  int chunk_size = chunk_length * chunk_density;

  // in case a buffer doesn't arrive in time
  if (scope_chunk_ >= scope_chunks_) {
    scope_chunk_ = 0;
    return;
  }

  const sample_type* source = reinterpret_cast<sample_type*>(map.data);
  sample_type* dest = scope_.data();
  source += (chunk_size / sizeof(sample_type)) * scope_chunk_;

  int bytes = 0;

  // make sure we don't go beyond the end of the buffer
  if (scope_chunk_ == scope_chunks_ - 1) {
    bytes = qMin(static_cast<Engine::Scope::size_type>(
                     map.size - (chunk_size * scope_chunk_)),
                 scope_.size() * sizeof(sample_type));
  } else {
    bytes = qMin(static_cast<Engine::Scope::size_type>(chunk_size),
                 scope_.size() * sizeof(sample_type));
  }

  scope_chunk_++;
  memcpy(dest, source, bytes);

  gst_buffer_unmap(latest_buffer_, &map);

  if (scope_chunk_ == scope_chunks_) {
    gst_buffer_unref(latest_buffer_);
    latest_buffer_ = nullptr;
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
                     bool force_stop_at_end, quint64 beginning_nanosec,
                     qint64 end_nanosec) {
  EnsureInitialised();

  Engine::Base::Load(url, change, force_stop_at_end, beginning_nanosec,
                     end_nanosec);

  QUrl gst_url = FixupUrl(url);

  bool crossfade =
      current_pipeline_ && ((crossfade_enabled_ && change & Engine::Manual) ||
                            (autocrossfade_enabled_ && change & Engine::Auto) ||
                            ((crossfade_enabled_ || autocrossfade_enabled_) &&
                             change & Engine::Intro));

  if (change & Engine::Auto && change & Engine::SameAlbum &&
      !crossfade_same_album_)
    crossfade = false;

  if (!crossfade && current_pipeline_ && current_pipeline_->url() == gst_url &&
      change & Engine::Auto) {
    // We're not crossfading, and the pipeline is already playing the URI we
    // want, so just do nothing.
    return true;
  }

  shared_ptr<GstEnginePipeline> pipeline =
      CreatePipeline(gst_url, force_stop_at_end ? end_nanosec : 0);
  if (!pipeline) return false;

  if (crossfade) StartFadeout();

  BufferingFinished();
  current_pipeline_ = pipeline;

  SetVolume(volume_);
  SetEqualizerEnabled(equalizer_enabled_);
  SetEqualizerParameters(equalizer_preamp_, equalizer_gains_);
  SetStereoBalance(stereo_balance_);

  // Maybe fade in this track
  if (crossfade)
    current_pipeline_->StartFader(fadeout_duration_nanosec_,
                                  QTimeLine::Forward);

  return true;
}

void GstEngine::StartFadeout() {
  if (is_fading_out_to_pause_) return;

  fadeout_pipeline_ = current_pipeline_;
  disconnect(fadeout_pipeline_.get(), 0, 0, 0);
  fadeout_pipeline_->RemoveAllBufferConsumers();

  fadeout_pipeline_->StartFader(fadeout_duration_nanosec_, QTimeLine::Backward);
  connect(fadeout_pipeline_.get(), SIGNAL(FaderFinished()),
          SLOT(FadeoutFinished()));
}

void GstEngine::StartFadeoutPause() {
  fadeout_pause_pipeline_ = current_pipeline_;
  disconnect(fadeout_pause_pipeline_.get(), SIGNAL(FaderFinished()), 0, 0);

  fadeout_pause_pipeline_->StartFader(fadeout_pause_duration_nanosec_,
                                      QTimeLine::Backward,
                                      QTimeLine::EaseInOutCurve, false);
  if (fadeout_pipeline_ && fadeout_pipeline_->state() == GST_STATE_PLAYING) {
    fadeout_pipeline_->StartFader(fadeout_pause_duration_nanosec_,
                                  QTimeLine::Backward, QTimeLine::LinearCurve,
                                  false);
  }
  connect(fadeout_pause_pipeline_.get(), SIGNAL(FaderFinished()),
          SLOT(FadeoutPauseFinished()));
  is_fading_out_to_pause_ = true;
}

bool GstEngine::Play(quint64 offset_nanosec) {
  EnsureInitialised();

  if (!current_pipeline_ || current_pipeline_->is_buffering()) return false;

  QFuture<GstStateChangeReturn> future =
      current_pipeline_->SetState(GST_STATE_PLAYING);
  NewClosure(future, this,
             SLOT(PlayDone(QFuture<GstStateChangeReturn>, quint64, int)),
             future, offset_nanosec, current_pipeline_->id());

  if (is_fading_out_to_pause_) {
    current_pipeline_->SetState(GST_STATE_PAUSED);
  }

  return true;
}

void GstEngine::PlayDone(QFuture<GstStateChangeReturn> future,
                         const quint64 offset_nanosec, const int pipeline_id) {
  GstStateChangeReturn ret = future.result();

  if (!current_pipeline_ || pipeline_id != current_pipeline_->id()) {
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

  // initial offset
  if (offset_nanosec != 0 || beginning_nanosec_ != 0) {
    Seek(offset_nanosec);
  }

  emit StateChanged(Engine::Playing);
  // we've successfully started playing a media stream with this url
  emit ValidSongRequested(url_);
}

void GstEngine::Stop(bool stop_after) {
  StopTimers();

  url_ = QUrl();  // To ensure we return Empty from state()
  beginning_nanosec_ = end_nanosec_ = 0;

  // Check if we started a fade out. If it isn't finished yet and the user
  // pressed stop, we cancel the fader and just stop the playback.
  if (is_fading_out_to_pause_) {
    disconnect(current_pipeline_.get(), SIGNAL(FaderFinished()), 0, 0);
    is_fading_out_to_pause_ = false;
    has_faded_out_ = true;

    fadeout_pause_pipeline_.reset();
    fadeout_pipeline_.reset();
  }

  if (fadeout_enabled_ && current_pipeline_ && !stop_after) StartFadeout();

  current_pipeline_.reset();
  BufferingFinished();
  emit StateChanged(Engine::Empty);
}

void GstEngine::FadeoutFinished() {
  fadeout_pipeline_.reset();
  emit FadeoutFinishedSignal();
}

void GstEngine::FadeoutPauseFinished() {
  fadeout_pause_pipeline_->SetState(GST_STATE_PAUSED);
  current_pipeline_->SetState(GST_STATE_PAUSED);
  emit StateChanged(Engine::Paused);
  StopTimers();

  is_fading_out_to_pause_ = false;
  has_faded_out_ = true;
  fadeout_pause_pipeline_.reset();
  fadeout_pipeline_.reset();

  emit FadeoutFinishedSignal();
}

void GstEngine::Pause() {
  if (!current_pipeline_ || current_pipeline_->is_buffering()) return;

  // Check if we started a fade out. If it isn't finished yet and the user
  // pressed play, we inverse the fader and resume the playback.
  if (is_fading_out_to_pause_) {
    disconnect(current_pipeline_.get(), SIGNAL(FaderFinished()), 0, 0);
    current_pipeline_->StartFader(fadeout_pause_duration_nanosec_,
                                  QTimeLine::Forward, QTimeLine::EaseInOutCurve,
                                  false);
    is_fading_out_to_pause_ = false;
    has_faded_out_ = false;
    emit StateChanged(Engine::Playing);
    return;
  }

  if (current_pipeline_->state() == GST_STATE_PLAYING) {
    if (fadeout_pause_enabled_) {
      StartFadeoutPause();
    } else {
      current_pipeline_->SetState(GST_STATE_PAUSED);
      emit StateChanged(Engine::Paused);
      StopTimers();
    }
  }
}

void GstEngine::Unpause() {
  if (!current_pipeline_ || current_pipeline_->is_buffering()) return;

  if (current_pipeline_->state() == GST_STATE_PAUSED) {
    current_pipeline_->SetState(GST_STATE_PLAYING);

    // Check if we faded out last time. If yes, fade in no matter what the
    // settings say. If we pause with fadeout, deactivate fadeout and resume
    // playback, the player would be muted if not faded in.
    if (has_faded_out_) {
      disconnect(current_pipeline_.get(), SIGNAL(FaderFinished()), 0, 0);
      current_pipeline_->StartFader(fadeout_pause_duration_nanosec_,
                                    QTimeLine::Forward,
                                    QTimeLine::EaseInOutCurve, false);
      has_faded_out_ = false;
    }

    emit StateChanged(Engine::Playing);

    StartTimers();
  }
}

void GstEngine::Seek(quint64 offset_nanosec) {
  if (!current_pipeline_) return;

  seek_pos_ = beginning_nanosec_ + offset_nanosec;
  waiting_to_seek_ = true;

  if (!seek_timer_->isActive()) {
    SeekNow();
    seek_timer_->start();  // Stop us from seeking again for a little while
  }
}

void GstEngine::SeekNow() {
  if (!waiting_to_seek_) return;
  waiting_to_seek_ = false;

  if (!current_pipeline_) return;

  if (!current_pipeline_->Seek(seek_pos_)) {
    qLog(Warning) << "Seek failed";
  }
}

void GstEngine::SetEqualizerEnabled(bool enabled) {
  equalizer_enabled_ = enabled;

  if (current_pipeline_) current_pipeline_->SetEqualizerEnabled(enabled);
}

void GstEngine::SetEqualizerParameters(int preamp,
                                       const QList<int>& band_gains) {
  equalizer_preamp_ = preamp;
  equalizer_gains_ = band_gains;

  if (current_pipeline_)
    current_pipeline_->SetEqualizerParams(preamp, band_gains);
}

void GstEngine::SetStereoBalance(float value) {
  stereo_balance_ = value;

  if (current_pipeline_) current_pipeline_->SetStereoBalance(value);
}

void GstEngine::SetVolumeSW(uint percent) {
  if (current_pipeline_) current_pipeline_->SetVolume(percent);
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
  if (e->timerId() != timer_id_) return;

  if (current_pipeline_) {
    const qint64 current_position = position_nanosec();
    const qint64 current_length = length_nanosec();

    const qint64 remaining = current_length - current_position;

    const qint64 fudge =
        kTimerIntervalNanosec + 100 * kNsecPerMsec;  // Mmm fudge
    const qint64 gap = buffer_duration_nanosec_ +
                       (autocrossfade_enabled_ ? fadeout_duration_nanosec_
                                               : kPreloadGapNanosec);

    // only if we know the length of the current stream...
    if (current_length > 0) {
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
  emit StateChanged(Engine::Error);
  // unable to play media stream with this url
  emit InvalidSongRequested(url_);

  // TODO: the types of errors listed below won't be shown to user - they will
  // get logged and the current song will be skipped; instead of maintaining
  // the list we should probably:
  // - don't report any engine's errors to user (always just log and skip)
  // - come up with a less intrusive error box (not a dialog but a notification
  //   popup of some kind) and then report all errors
  if (!(domain == GST_RESOURCE_ERROR &&
        error_code == GST_RESOURCE_ERROR_NOT_FOUND) &&
      !(domain == GST_STREAM_ERROR &&
        error_code == GST_STREAM_ERROR_TYPE_NOT_FOUND) &&
      !(domain == GST_RESOURCE_ERROR &&
        error_code == GST_RESOURCE_ERROR_OPEN_READ)) {
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
  emit TrackEnded();
}

void GstEngine::NewMetaData(int pipeline_id,
                            const Engine::SimpleMetaBundle& bundle) {
  if (!current_pipeline_.get() || current_pipeline_->id() != pipeline_id)
    return;

  emit MetaData(bundle);
}

GstElement* GstEngine::CreateElement(const QString& factoryName,
                                     GstElement* bin) {
  // Make a unique name
  QString name = factoryName + "-" + QString::number(next_element_id_++);

  GstElement* element = gst_element_factory_make(
      factoryName.toAscii().constData(), name.toAscii().constData());

  if (!element) {
    emit Error(QString(
                   "GStreamer could not create the element: %1.  "
                   "Please make sure that you have installed all necessary "
                   "GStreamer plugins (e.g. OGG and MP3)").arg(factoryName));
    gst_object_unref(GST_OBJECT(bin));
    return nullptr;
  }

  if (bin) gst_bin_add(GST_BIN(bin), element);

  return element;
}

GstEngine::PluginDetailsList GstEngine::GetPluginList(
    const QString& classname) const {
  PluginDetailsList ret;

  GstRegistry* registry = gst_registry_get();
  GList* const features =
      gst_registry_get_feature_list(registry, GST_TYPE_ELEMENT_FACTORY);

  GList* p = features;
  while (p) {
    GstElementFactory* factory = GST_ELEMENT_FACTORY(p->data);
    if (QString(gst_element_factory_get_klass(factory)).contains(classname)) {
      PluginDetails details;
      details.name = QString::fromUtf8(gst_plugin_feature_get_name(p->data));
      details.description = QString::fromUtf8(gst_element_factory_get_metadata(
          factory, GST_ELEMENT_METADATA_DESCRIPTION));
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
  ret->set_buffer_min_fill(buffer_min_fill_);
  ret->set_mono_playback(mono_playback_);
  ret->set_sample_rate(sample_rate_);

  ret->AddBufferConsumer(this);
  for (BufferConsumer* consumer : buffer_consumers_) {
    ret->AddBufferConsumer(consumer);
  }

  connect(ret.get(), SIGNAL(EndOfStreamReached(int, bool)),
          SLOT(EndOfStreamReached(int, bool)));
  connect(ret.get(), SIGNAL(Error(int, QString, int, int)),
          SLOT(HandlePipelineError(int, QString, int, int)));
  connect(ret.get(), SIGNAL(MetadataFound(int, Engine::SimpleMetaBundle)),
          SLOT(NewMetaData(int, Engine::SimpleMetaBundle)));
  connect(ret.get(), SIGNAL(BufferingStarted()), SLOT(BufferingStarted()));
  connect(ret.get(), SIGNAL(BufferingProgress(int)),
          SLOT(BufferingProgress(int)));
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

  if (!ret->InitFromUrl(url, end_nanosec)) ret.reset();

  return ret;
}

void GstEngine::AddBufferConsumer(BufferConsumer* consumer) {
  buffer_consumers_ << consumer;
  if (current_pipeline_) current_pipeline_->AddBufferConsumer(consumer);
}

void GstEngine::RemoveBufferConsumer(BufferConsumer* consumer) {
  buffer_consumers_.removeAll(consumer);
  if (current_pipeline_) current_pipeline_->RemoveBufferConsumer(consumer);
}

int GstEngine::AddBackgroundStream(shared_ptr<GstEnginePipeline> pipeline) {
  // We don't want to get metadata messages or end notifications.
  disconnect(pipeline.get(),
             SIGNAL(MetadataFound(int, Engine::SimpleMetaBundle)), this, 0);
  disconnect(pipeline.get(), SIGNAL(EndOfStreamReached(int, bool)), this, 0);
  connect(pipeline.get(), SIGNAL(EndOfStreamReached(int, bool)),
          SLOT(BackgroundStreamFinished()));

  const int stream_id = next_background_stream_id_++;
  background_streams_[stream_id] = pipeline;

  QFuture<GstStateChangeReturn> future = pipeline->SetState(GST_STATE_PLAYING);
  NewClosure(future, this,
             SLOT(BackgroundStreamPlayDone(QFuture<GstStateChangeReturn>, int)),
             future, stream_id);
  return stream_id;
}

void GstEngine::BackgroundStreamPlayDone(QFuture<GstStateChangeReturn> future,
                                         int stream_id) {
  GstStateChangeReturn ret = future.result();

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

GstEngine::OutputDetailsList GstEngine::GetOutputsList() const {
  const_cast<GstEngine*>(this)->EnsureInitialised();

  OutputDetailsList ret;

  OutputDetails default_output;
  default_output.description = tr("Choose automatically");
  default_output.gstreamer_plugin_name = kAutoSink;
  ret.append(default_output);

  for (DeviceFinder* finder : device_finders_) {
    for (const DeviceFinder::Device& device : finder->ListDevices()) {
      OutputDetails output;
      output.description = device.description;
      output.icon_name = device.icon_name;
      output.gstreamer_plugin_name = finder->gstreamer_sink();
      output.device_property_value = device.device_property_value;
      ret.append(output);
    }
  }

  PluginDetailsList plugins = GetPluginList("Sink/Audio");
  // If there are only 2 plugins (autoaudiosink and the OS' default), don't add
  // any, since the OS' default would be redundant.
  if (plugins.count() > 2) {
    for (const PluginDetails& plugin : plugins) {
      if (plugin.name == kAutoSink) {
        continue;
      }

      OutputDetails output;
      output.description = tr("Default device on %1").arg(plugin.description);
      output.gstreamer_plugin_name = plugin.name;
      ret.append(output);
    }
  }

  return ret;
}
