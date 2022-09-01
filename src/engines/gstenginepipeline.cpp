/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

// To turn on logging of bus callbacks, run Clementine with:
// --log-levels GstEnginePipelineCallbacks:3

#include "gstenginepipeline.h"

#include <QCoreApplication>
#include <QDir>
#include <QPair>
#include <QRegExp>
#include <limits>

#include "bufferconsumer.h"
#include "config.h"
#include "core/concurrentrun.h"
#include "core/logging.h"
#include "core/mac_startup.h"
#include "core/signalchecker.h"
#include "core/utilities.h"
#include "gstelementdeleter.h"
#include "gstengine.h"
#ifdef HAVE_AUDIOCD
#include "devices/cddadevice.h"
#endif
#include "internet/core/internetmodel.h"

const int GstEnginePipeline::kGstStateTimeoutNanosecs = 10000000;
const int GstEnginePipeline::kFaderFudgeMsec = 2000;

const int GstEnginePipeline::kEqBandCount = 10;
const int GstEnginePipeline::kEqBandFrequencies[] = {
    60, 170, 310, 600, 1000, 3000, 6000, 12000, 14000, 16000};

GstElementDeleter* GstEnginePipeline::sElementDeleter = nullptr;

GstEnginePipeline::GstEnginePipeline(GstEngine* engine)
    : GstPipelineBase("audio"),
      engine_(engine),
      valid_(false),
      sink_(GstEngine::kAutoSink),
      segment_start_(0),
      segment_start_received_(false),
      emit_track_ended_on_stream_start_(false),
      emit_track_ended_on_time_discontinuity_(false),
      last_buffer_offset_(0),
      eq_enabled_(false),
      eq_preamp_(0),
      stereo_balance_(0.0f),
      rg_enabled_(false),
      rg_mode_(0),
      rg_preamp_(0.0),
      rg_compression_(true),
      buffer_duration_nanosec_(1 * kNsecPerSec),
      buffer_min_fill_(33),
      buffering_(false),
      mono_playback_(false),
      sample_rate_(GstEngine::kAutoSampleRate),
      end_offset_nanosec_(-1),
      next_beginning_offset_nanosec_(-1),
      next_end_offset_nanosec_(-1),
      ignore_next_seek_(false),
      ignore_tags_(false),
      pipeline_is_initialised_(false),
      pipeline_is_connected_(false),
      pending_seek_nanosec_(-1),
      last_known_position_ns_(0),
      volume_percent_(100),
      volume_modifier_(1.0),
      uridecodebin_(nullptr),
      audiobin_(nullptr),
      queue_(nullptr),
      audioconvert_(nullptr),
      rgvolume_(nullptr),
      rglimiter_(nullptr),
      audioconvert2_(nullptr),
      equalizer_(nullptr),
      stereo_panorama_(nullptr),
      volume_(nullptr),
      audioscale_(nullptr),
      audiosink_(nullptr),
      capsfilter_(nullptr),
      tee_(nullptr),
      tee_probe_pad_(nullptr),
      tee_audio_pad_(nullptr) {
  if (!sElementDeleter) {
    sElementDeleter = new GstElementDeleter;
  }

  for (int i = 0; i < kEqBandCount; ++i) eq_band_gains_ << 0;
}

void GstEnginePipeline::set_output_device(const QString& sink,
                                          const QVariant& device) {
  sink_ = sink;
  device_ = device;
}

void GstEnginePipeline::set_replaygain(bool enabled, int mode, float preamp,
                                       bool compression) {
  rg_enabled_ = enabled;
  rg_mode_ = mode;
  rg_preamp_ = preamp;
  rg_compression_ = compression;
}

void GstEnginePipeline::set_buffer_duration_nanosec(
    qint64 buffer_duration_nanosec) {
  buffer_duration_nanosec_ = buffer_duration_nanosec;
}

void GstEnginePipeline::set_buffer_min_fill(int percent) {
  buffer_min_fill_ = percent;
}

void GstEnginePipeline::set_mono_playback(bool enabled) {
  mono_playback_ = enabled;
}

void GstEnginePipeline::set_sample_rate(int rate) { sample_rate_ = rate; }

bool GstEnginePipeline::ReplaceDecodeBin(GstElement* new_bin) {
  if (!new_bin) return false;

  // Destroy the old elements if they are set
  // Note that the caller to this function MUST schedule the old uridecodebin_
  // for deletion in the main thread.
  if (uridecodebin_) {
    gst_bin_remove(GST_BIN(pipeline_), uridecodebin_);
  }

  uridecodebin_ = new_bin;
  segment_start_ = 0;
  segment_start_received_ = false;
  pipeline_is_connected_ = false;
  gst_bin_add(GST_BIN(pipeline_), uridecodebin_);

  return true;
}

bool GstEnginePipeline::ReplaceDecodeBin(const QUrl& url) {
  GstElement* new_bin = CreateDecodeBinFromUrl(url);
  return ReplaceDecodeBin(new_bin);
}

QByteArray GstEnginePipeline::GstUriFromUrl(const QUrl& url) {
#ifdef HAVE_AUDIOCD
  if (url.scheme() == "cdda") return CddaDevice::TrackUrlToStr(url).toUtf8();
#endif
  return Utilities::GetUriForGstreamer(url);
}

GstElement* GstEnginePipeline::CreateDecodeBinFromUrl(const QUrl& url) {
  GstElement* new_bin = nullptr;
  QByteArray uri = GstUriFromUrl(url);
  new_bin = engine_->CreateElement("uridecodebin");
  if (!new_bin) return nullptr;
  g_object_set(G_OBJECT(new_bin), "uri", uri.constData(), nullptr);
  CHECKED_GCONNECT(G_OBJECT(new_bin), "drained", &SourceDrainedCallback, this);
  CHECKED_GCONNECT(G_OBJECT(new_bin), "pad-added", &NewPadCallback, this);
  CHECKED_GCONNECT(G_OBJECT(new_bin), "notify::source", &SourceSetupCallback,
                   this);
  return new_bin;
}

GstElement* GstEnginePipeline::CreateDecodeBinFromString(const char* pipeline) {
  GError* error = nullptr;
  GstElement* bin = gst_parse_bin_from_description(pipeline, TRUE, &error);

  if (error) {
    QString message = QString::fromLocal8Bit(error->message);
    int domain = error->domain;
    int code = error->code;
    g_error_free(error);

    qLog(Warning) << message;
    emit Error(id(), message, domain, code);

    return nullptr;
  } else {
    return bin;
  }
}

bool GstEnginePipeline::InitAudioBin() {
  // Here we create all the parts of the gstreamer pipeline - from the source
  // to the sink.  The parts of the pipeline are split up into bins:
  //   uri decode bin -> audio bin
  // The uri decode bin is a gstreamer builtin that automatically picks the
  // right type of source and decoder for the URI.

  // The audio bin gets created here and contains:
  //   queue ! audioconvert
  //         ! ( rgvolume ! rglimiter ! audioconvert2 ) ! capsfilter ! tee
  // rgvolume and rglimiter are only created when replaygain is enabled.

  // After the tee the pipeline splits.  One split is converted to 16-bit int
  // samples for the scope, the other is kept as float32 and sent to the
  // speaker.
  //   tee1 ! probe_queue ! probe_converter ! <caps16> ! probe_sink
  //   tee2 ! audio_queue ! equalizer_preamp ! equalizer ! volume ! audioscale
  //        ! convert ! audiosink

  gst_segment_init(&last_decodebin_segment_, GST_FORMAT_TIME);

  // Audio bin
  audiobin_ = gst_bin_new("audiobin");
  // Floating reference is transferred to pipeline
  gst_bin_add(GST_BIN(pipeline_), audiobin_);

  // Create the sink
  if (!(audiosink_ = engine_->CreateElement(sink_, audiobin_))) {
    qLog(Error) << "Failed to create audio sink";
    return false;
  }

  if (g_object_class_find_property(G_OBJECT_GET_CLASS(audiosink_), "device") &&
      !device_.toString().isEmpty()) {
    switch (device_.type()) {
      case QVariant::Int:
        g_object_set(G_OBJECT(audiosink_), "device", device_.toInt(), nullptr);
        break;
      case QVariant::LongLong:
        g_object_set(G_OBJECT(audiosink_), "device", device_.toLongLong(),
                     nullptr);
        break;
      case QVariant::String:
        g_object_set(G_OBJECT(audiosink_), "device",
                     device_.toString().toUtf8().constData(), nullptr);
        break;
      case QVariant::ByteArray: {
        g_object_set(G_OBJECT(audiosink_), "device",
                     device_.toByteArray().constData(), nullptr);
        break;
      }

      default:
        qLog(Warning) << "Unknown device type" << device_;
        break;
    }
  }

  // Create all the other elements
  GstElement *probe_queue, *probe_converter, *probe_sink, *audio_queue,
      *convert;

  queue_ = engine_->CreateElement("queue2", audiobin_);
  audioconvert_ = engine_->CreateElement("audioconvert", audiobin_);
  tee_ = engine_->CreateElement("tee", audiobin_);

  probe_queue = engine_->CreateElement("queue2", audiobin_);
  probe_converter = engine_->CreateElement("audioconvert", audiobin_);
  probe_sink = engine_->CreateElement("fakesink", audiobin_);

  audio_queue = engine_->CreateElement("queue", audiobin_);
  equalizer_preamp_ = engine_->CreateElement("volume", audiobin_);
  equalizer_ = engine_->CreateElement("equalizer-nbands", audiobin_);
  stereo_panorama_ = engine_->CreateElement("audiopanorama", audiobin_);
  volume_ = engine_->CreateElement("volume", audiobin_);
  audioscale_ = engine_->CreateElement("audioresample", audiobin_);
  convert = engine_->CreateElement("audioconvert", audiobin_);
  capsfilter_ = engine_->CreateElement("capsfilter", audiobin_);

  if (!queue_ || !audioconvert_ || !tee_ || !probe_queue || !probe_converter ||
      !probe_sink || !audio_queue || !equalizer_preamp_ || !equalizer_ ||
      !stereo_panorama_ || !volume_ || !audioscale_ || !convert ||
      !capsfilter_) {
    qLog(Error) << "Failed to create elements";
    return false;
  }

  // Create the replaygain elements if it's enabled.  event_probe is the
  // audioconvert element we attach the probe to, which will change depending
  // on whether replaygain is enabled.  tee_src is the element that links to
  // the tee.
  GstElement* event_probe = audioconvert_;
  GstElement* tee_src = audioconvert_;

  if (rg_enabled_) {
    rgvolume_ = engine_->CreateElement("rgvolume", audiobin_);
    rglimiter_ = engine_->CreateElement("rglimiter", audiobin_);
    audioconvert2_ = engine_->CreateElement("audioconvert", audiobin_);
    event_probe = audioconvert2_;
    tee_src = audioconvert2_;

    if (!rgvolume_ || !rglimiter_ || !audioconvert2_) {
      qLog(Error) << "Failed to create rg elements";
      return false;
    }

    // Set replaygain settings
    g_object_set(G_OBJECT(rgvolume_), "album-mode", rg_mode_, nullptr);
    g_object_set(G_OBJECT(rgvolume_), "pre-amp", double(rg_preamp_), nullptr);
    g_object_set(G_OBJECT(rglimiter_), "enabled", int(rg_compression_),
                 nullptr);
  }

  // Create a pad on the outside of the audiobin and connect it to the pad of
  // the first element.
  GstPad* pad = gst_element_get_static_pad(queue_, "sink");
  gst_element_add_pad(audiobin_, gst_ghost_pad_new("sink", pad));
  gst_object_unref(pad);

  // Add a data probe on the src pad of the audioconvert element for our scope.
  // We do it here because we want pre-equalized and pre-volume samples
  // so that our visualization are not be affected by them.
  pad = gst_element_get_static_pad(event_probe, "src");
  gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_EVENT_UPSTREAM,
                    &EventHandoffCallback, this, NULL);
  gst_object_unref(pad);

  // Configure the fakesink properly
  g_object_set(G_OBJECT(probe_sink), "sync", TRUE, nullptr);

  // Setting the equalizer bands:
  //
  // GStreamer's GstIirEqualizerNBands sets up shelve filters for the first and
  // last bands as corner cases. That was causing the "inverted slider" bug.
  // As a workaround, we create two dummy bands at both ends of the spectrum.
  // This causes the actual first and last adjustable bands to be
  // implemented using band-pass filters.

  g_object_set(G_OBJECT(equalizer_), "num-bands", 10 + 2, nullptr);

  // Dummy first band (bandwidth 0, cutting below 20Hz):
  GstObject* first_band = GST_OBJECT(
      gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(equalizer_), 0));
  g_object_set(G_OBJECT(first_band), "freq", 20.0, "bandwidth", 0, "gain", 0.0f,
               nullptr);
  g_object_unref(G_OBJECT(first_band));

  // Dummy last band (bandwidth 0, cutting over 20KHz):
  GstObject* last_band = GST_OBJECT(gst_child_proxy_get_child_by_index(
      GST_CHILD_PROXY(equalizer_), kEqBandCount + 1));
  g_object_set(G_OBJECT(last_band), "freq", 20000.0, "bandwidth", 0, "gain",
               0.0f, nullptr);
  g_object_unref(G_OBJECT(last_band));

  int last_band_frequency = 0;
  for (int i = 0; i < kEqBandCount; ++i) {
    const int index_in_eq = i + 1;
    GstObject* band = GST_OBJECT(gst_child_proxy_get_child_by_index(
        GST_CHILD_PROXY(equalizer_), index_in_eq));

    const float frequency = kEqBandFrequencies[i];
    const float bandwidth = frequency - last_band_frequency;
    last_band_frequency = frequency;

    g_object_set(G_OBJECT(band), "freq", frequency, "bandwidth", bandwidth,
                 "gain", 0.0f, nullptr);
    g_object_unref(G_OBJECT(band));
  }

  // Set the stereo balance.
  g_object_set(G_OBJECT(stereo_panorama_), "panorama", stereo_balance_,
               nullptr);

  // Set the buffer duration.  We set this on this queue instead of the
  // decode bin (in ReplaceDecodeBin()) because setting it on the decode bin
  // only affects network sources.
  // Disable the default buffer and byte limits, so we only buffer based on
  // time.
  g_object_set(G_OBJECT(queue_), "max-size-buffers", 0, nullptr);
  g_object_set(G_OBJECT(queue_), "max-size-bytes", 0, nullptr);
  g_object_set(G_OBJECT(queue_), "max-size-time", buffer_duration_nanosec_,
               nullptr);
  g_object_set(G_OBJECT(queue_), "low-percent", buffer_min_fill_, nullptr);

  if (buffer_duration_nanosec_ > 0) {
    g_object_set(G_OBJECT(queue_), "use-buffering", true, nullptr);
  }

  g_object_set(G_OBJECT(probe_queue), "max-size-buffers", 0, nullptr);
  g_object_set(G_OBJECT(probe_queue), "max-size-bytes", 0, nullptr);
  g_object_set(G_OBJECT(probe_queue), "max-size-time", 0, nullptr);

  gst_element_link(queue_, audioconvert_);

  GstCaps* caps16 = gst_caps_new_simple("audio/x-raw", "format", G_TYPE_STRING,
                                        "S16LE", nullptr);
  gst_element_link_filtered(probe_converter, probe_sink, caps16);
  gst_caps_unref(caps16);

  // Link the outputs of tee to the queues on each path.
  pad = gst_element_get_static_pad(probe_queue, "sink");
  tee_probe_pad_ = gst_element_get_request_pad(tee_, "src_%u");
  gst_pad_link(tee_probe_pad_, pad);
  gst_object_unref(pad);

  pad = gst_element_get_static_pad(audio_queue, "sink");
  tee_audio_pad_ = gst_element_get_request_pad(tee_, "src_%u");
  gst_pad_link(tee_audio_pad_, pad);
  gst_object_unref(pad);

  // Link replaygain elements if enabled.
  if (rg_enabled_) {
    gst_element_link_many(audioconvert_, rgvolume_, rglimiter_, audioconvert2_,
                          nullptr);
  }

  // Link the trunk to the tee via filter.
  gst_element_link_many(tee_src, capsfilter_, tee_, nullptr);

  // If the user has selected a format, then set it now.
  if (format_ != GstEngine::kOutFormatDetect) {
    SetOutputFormat(format_);
  }

  // Link the analyzer output of the tee
  gst_element_link(probe_queue, probe_converter);

  gst_element_link_many(audio_queue, equalizer_preamp_, equalizer_,
                        stereo_panorama_, volume_, audioscale_, convert,
                        nullptr);

  // We only limit the media type to raw audio.
  // Let the audio output of the tee autonegotiate the bit depth and format.
  GstCaps* caps = gst_caps_new_empty_simple("audio/x-raw");

  // Add caps for fixed sample rate and mono, but only if requested
  if (sample_rate_ != GstEngine::kAutoSampleRate && sample_rate_ > 0) {
    gst_caps_set_simple(caps, "rate", G_TYPE_INT, sample_rate_, nullptr);
  }

  if (mono_playback_) {
    gst_caps_set_simple(caps, "channels", G_TYPE_INT, 1, nullptr);
  }

  gst_element_link_filtered(convert, audiosink_, caps);
  gst_caps_unref(caps);

  // Add probes and handlers.
  pad = gst_element_get_static_pad(probe_converter, "src");
  gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, HandoffCallback, this,
                    nullptr);
  gst_object_unref(pad);
  GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
  gst_bus_set_sync_handler(bus, BusCallbackSync, this, nullptr);
  gst_bus_add_watch(bus, BusCallback, this);
  gst_object_unref(bus);

  return true;
}

void GstEnginePipeline::MaybeLinkDecodeToAudio() {
  if (!uridecodebin_ || !audiobin_) return;

  GstPad* pad = gst_element_get_static_pad(uridecodebin_, "src");
  if (!pad) return;

  gst_object_unref(pad);
  gst_element_link(uridecodebin_, audiobin_);
}

bool GstEnginePipeline::InitFromString(const QString& pipeline) {
  if (!Init()) return false;

  GstElement* new_bin =
      CreateDecodeBinFromString(pipeline.toLatin1().constData());
  if (!new_bin) {
    return false;
  }

  if (!ReplaceDecodeBin(new_bin)) {
    gst_object_unref(GST_OBJECT(new_bin));
    return false;
  }

  if (!InitAudioBin()) return false;
  return gst_element_link(new_bin, audiobin_);
}

bool GstEnginePipeline::InitFromReq(const MediaPlaybackRequest& req,
                                    qint64 end_nanosec) {
  if (!Init()) return false;

  current_ = req;
  QUrl url = current_.url_;
#ifdef HAVE_AUDIOCD
  if (url.scheme() == "cdda" && !url.path().isEmpty()) {
    // Currently, Gstreamer can't handle input CD devices inside cdda URL. So
    // we handle them ourself: we extract the track number and re-create an
    // URL with only cdda:// + the track number (which can be handled by
    // Gstreamer). We keep the device in mind, and we will set it later using
    // SourceSetupCallback
    QStringList path = url.path().split('/');
    url = QUrl(QString("cdda://%1").arg(path.takeLast()));
    source_device_ = path.join("/");
  }
#endif
  end_offset_nanosec_ = end_nanosec;

  // Decode bin
  if (!ReplaceDecodeBin(url)) return false;

  if (!InitAudioBin()) return false;

  // Link decoder and audio bins if decoder bin already has a src pad.
  MaybeLinkDecodeToAudio();

  return true;
}

GstEnginePipeline::~GstEnginePipeline() {
  if (pipeline_) {
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_remove_watch(bus);

    gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
    gst_object_unref(bus);

    gst_element_set_state(pipeline_, GST_STATE_NULL);

    if (tee_) {
      // Request pads are not automatically released and dereferenced. They
      // should only be released when the pipeline is in a null or ready state.
      if (tee_probe_pad_) {
        gst_element_release_request_pad(tee_, tee_probe_pad_);
        gst_object_unref(tee_probe_pad_);
      }
      if (tee_audio_pad_) {
        gst_element_release_request_pad(tee_, tee_audio_pad_);
        gst_object_unref(tee_audio_pad_);
      }
    }
  }
}

gboolean GstEnginePipeline::BusCallback(GstBus*, GstMessage* msg,
                                        gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);

  qLogCat(Debug, "GstEnginePipelineCallbacks")
      << instance->id() << "bus message" << GST_MESSAGE_TYPE_NAME(msg);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR:
      instance->ErrorMessageReceived(msg);
      break;

    case GST_MESSAGE_TAG:
      instance->TagMessageReceived(msg);
      break;

    case GST_MESSAGE_STATE_CHANGED:
      instance->StateChangedMessageReceived(msg);
      break;

    default:
      break;
  }

  return FALSE;
}

GstBusSyncReply GstEnginePipeline::BusCallbackSync(GstBus*, GstMessage* msg,
                                                   gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);

  qLogCat(Debug, "GstEnginePipelineCallbacks")
      << instance->id() << "sync bus message" << GST_MESSAGE_TYPE_NAME(msg);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      emit instance->EndOfStreamReached(instance->id(), false);
      break;

    case GST_MESSAGE_TAG:
      instance->TagMessageReceived(msg);
      break;

    case GST_MESSAGE_ERROR:
      instance->ErrorMessageReceived(msg);
      break;

    case GST_MESSAGE_ELEMENT:
      instance->ElementMessageReceived(msg);
      break;

    case GST_MESSAGE_STATE_CHANGED:
      instance->StateChangedMessageReceived(msg);
      break;

    case GST_MESSAGE_BUFFERING:
      instance->BufferingMessageReceived(msg);
      break;

    case GST_MESSAGE_STREAM_STATUS:
      instance->StreamStatusMessageReceived(msg);
      break;

    case GST_MESSAGE_STREAM_START:
      if (instance->emit_track_ended_on_stream_start_) {
        qLog(Debug) << "New segment started, EOS will signal on next buffer "
                       "discontinuity";
        instance->emit_track_ended_on_stream_start_ = false;
        instance->emit_track_ended_on_time_discontinuity_ = true;
      }
      break;

    default:
      break;
  }

  return GST_BUS_PASS;
}

void GstEnginePipeline::StreamStatusMessageReceived(GstMessage* msg) {
  GstStreamStatusType type;
  GstElement* owner;
  gst_message_parse_stream_status(msg, &type, &owner);

  if (type == GST_STREAM_STATUS_TYPE_CREATE) {
    const GValue* val = gst_message_get_stream_status_object(msg);
    if (G_VALUE_TYPE(val) == GST_TYPE_TASK) {
      GstTask* task = static_cast<GstTask*>(g_value_get_object(val));
      gst_task_set_enter_callback(task, &TaskEnterCallback, this, NULL);
    }
  }
}

void GstEnginePipeline::TaskEnterCallback(GstTask*, GThread*, gpointer) {
  // Bump the priority of the thread only on OS X

#ifdef Q_OS_DARWIN
  sched_param param;
  memset(&param, 0, sizeof(param));

  param.sched_priority = 99;
  pthread_setschedparam(pthread_self(), SCHED_RR, &param);
#endif
}

void GstEnginePipeline::ElementMessageReceived(GstMessage* msg) {
  const GstStructure* structure = gst_message_get_structure(msg);

  if (gst_structure_has_name(structure, "redirect")) {
    const char* uri = gst_structure_get_string(structure, "new-location");

    // Set the redirect URL.  In mmssrc redirect messages come during the
    // initial state change to PLAYING, so callers can pick up this URL after
    // the state change has failed.
    redirect_url_ = QUrl::fromEncoded(uri);
  }
}

void GstEnginePipeline::ErrorMessageReceived(GstMessage* msg) {
  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  QString message = QString::fromLocal8Bit(error->message);
  QString debugstr = QString::fromLocal8Bit(debugs);
  int domain = error->domain;
  int code = error->code;
  g_error_free(error);
  g_free(debugs);

  if (!redirect_url_.isEmpty() &&
      debugstr.contains(
          "A redirect message was posted on the bus and should have been "
          "handled by the application.")) {
    // mmssrc posts a message on the bus *and* makes an error message when it
    // wants to do a redirect.  We handle the message, but now we have to
    // ignore the error too.
    return;
  }

  for (const QString& l : debugstr.split("\n")) {
    // Messages may contain URLs with auth info in query strings.
    qLog(Error) << id() << Utilities::ScrubUrlQueries(l);
  }

  emit Error(id(), message, domain, code);
}

namespace {

/*
 * Streams served by Akamai tend to have a weird tag format embedded.
 *
 * Example:
 * All Things Dance - text="Evolution" song_spot="T" MediaBaseId="0"
 * itunesTrackId="0" amgTrackId="0" amgArtistId="0" TAID="0" TPID="0"
 * cartcutId="0"
 */
QPair<QString, QString> ParseAkamaiTag(const QString& tag) {
  QRegExp re("(.*) - text=\"([^\"]+)");
  re.indexIn(tag);
  if (re.capturedTexts().length() >= 3) {
    return qMakePair(re.cap(1), re.cap(2));
  }
  return qMakePair(tag, QString());
}

bool IsAkamaiTag(const QString& tag) { return tag.contains("- text=\""); }
}  // namespace

void GstEnginePipeline::TagMessageReceived(GstMessage* msg) {
  GstTagList* taglist = nullptr;
  gst_message_parse_tag(msg, &taglist);

  Engine::SimpleMetaBundle bundle;
  bundle.title = ParseTag(taglist, GST_TAG_TITLE);
  if (IsAkamaiTag(bundle.title)) {
    QPair<QString, QString> artistTitlePair = ParseAkamaiTag(bundle.title);
    bundle.artist = artistTitlePair.first;
    bundle.title = artistTitlePair.second;
  } else {
    bundle.artist = ParseTag(taglist, GST_TAG_ARTIST);
    bundle.comment = ParseTag(taglist, GST_TAG_COMMENT);
    bundle.album = ParseTag(taglist, GST_TAG_ALBUM);
  }

  gst_tag_list_free(taglist);

  if (ignore_tags_) return;

  if (!bundle.title.isEmpty() || !bundle.artist.isEmpty() ||
      !bundle.comment.isEmpty() || !bundle.album.isEmpty())
    emit MetadataFound(id(), bundle);
}

QString GstEnginePipeline::ParseTag(GstTagList* list, const char* tag) const {
  gchar* data = nullptr;
  bool success = gst_tag_list_get_string(list, tag, &data);

  QString ret;
  if (success && data) {
    ret = QString::fromUtf8(data);
    g_free(data);
  }
  return ret.trimmed();
}

void GstEnginePipeline::StateChangedMessageReceived(GstMessage* msg) {
  if (msg->src != GST_OBJECT(pipeline_)) {
    // We only care about state changes of the whole pipeline.
    return;
  }

  GstState old_state, new_state, pending;
  gst_message_parse_state_changed(msg, &old_state, &new_state, &pending);

  if (!pipeline_is_initialised_ &&
      (new_state == GST_STATE_PAUSED || new_state == GST_STATE_PLAYING)) {
    pipeline_is_initialised_ = true;
    if (pending_seek_nanosec_ != -1 && pipeline_is_connected_) {
      QMetaObject::invokeMethod(this, "Seek", Qt::QueuedConnection,
                                Q_ARG(qint64, pending_seek_nanosec_));
    }
  }

  if (pipeline_is_initialised_ && new_state != GST_STATE_PAUSED &&
      new_state != GST_STATE_PLAYING) {
    pipeline_is_initialised_ = false;
  }
}

void GstEnginePipeline::BufferingMessageReceived(GstMessage* msg) {
  // Only handle buffering messages from the queue2 element in audiobin - not
  // the one that's created automatically by uridecodebin.
  if (GST_ELEMENT(GST_MESSAGE_SRC(msg)) != queue_) {
    return;
  }

  // If we are loading new next track, we don't have to pause the playback.
  // The buffering is for the next track and not the current one.
  if (emit_track_ended_on_stream_start_) {
    qLog(Debug) << "Buffering next track";
    return;
  }

  int percent = 0;
  gst_message_parse_buffering(msg, &percent);

  const GstState current_state = state();

  if (percent == 0 && current_state == GST_STATE_PLAYING && !buffering_) {
    buffering_ = true;
    emit BufferingStarted();

    SetState(GST_STATE_PAUSED);
  } else if (percent == 100 && buffering_) {
    buffering_ = false;
    emit BufferingFinished();

    SetState(GST_STATE_PLAYING);
  } else if (buffering_) {
    emit BufferingProgress(percent);
  }
}

QString GstEnginePipeline::GetAudioFormat(GstCaps* caps) {
  const guint sz = gst_caps_get_size(caps);
  for (int i = 0; i < sz; i++) {
    GstStructure* s = gst_caps_get_structure(caps, i);
    if (strcmp(gst_structure_get_name(s), "audio/x-raw") == 0) {
      const gchar* fmt = gst_structure_get_string(s, "format");
      if (fmt != nullptr) {
        return QString::fromUtf8(fmt);
      }
    }
  }
  return "";
}

void GstEnginePipeline::NewPadCallback(GstElement*, GstPad* pad,
                                       gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  GstPad* const audiopad =
      gst_element_get_static_pad(instance->audiobin_, "sink");

  qLog(Debug) << "Decoder bin pad added:" << GST_PAD_NAME(pad);

  // Make sure the audio bin isn't already linked to something.
  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Warning) << instance->id()
                  << "audiopad is already linked, unlinking old pad";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  // See what the decoder bin wants to output.
  GstCaps* caps = gst_pad_get_current_caps(pad);
  if (caps) {
    gchar* caps_str = gst_caps_to_string(caps);
    qLog(Debug) << "Initial decoder caps:" << caps_str;
    g_free(caps_str);

    if (instance->format_ != GstEngine::kOutFormatDetect) {
      // Caps were set when the pipeline was constructed.
    } else if (instance->pipeline_is_initialised_) {
      qLog(Debug)
          << "Ignoring native format since pipeline is already running.";
    } else {
      QString fmt = GetAudioFormat(caps);

      // The output branch only handles F32LE and S16LE. If the source is S16LE,
      // then use that throughout the pipeline. Otherwise, use F32LE.
      if (fmt == GstEngine::kOutFormatS16LE) {
        instance->SetOutputFormat(GstEngine::kOutFormatS16LE);
      } else {
        instance->SetOutputFormat(GstEngine::kOutFormatF32LE);
      }
    }
    gst_caps_unref(caps);
  }

  // Link decodebin's sink pad to audiobin's src pad.
  if (gst_pad_link(pad, audiopad) != GST_PAD_LINK_OK) {
    qLog(Error) << "Failed to link decoder to audio bin.";
  }
  gst_object_unref(audiopad);

  // Offset the timestamps on all the buffers coming out of the decodebin so
  // they line up exactly with the end of the last buffer from the old
  // decodebin.
  // "Running time" is the time since the last flushing seek.
  GstClockTime running_time = gst_segment_to_running_time(
      &instance->last_decodebin_segment_, GST_FORMAT_TIME,
      instance->last_decodebin_segment_.position);
  gst_pad_set_offset(pad, running_time);

  // Add a probe to the pad so we can update last_decodebin_segment_.
  gst_pad_add_probe(
      pad,
      static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BUFFER |
                                   GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM |
                                   GST_PAD_PROBE_TYPE_EVENT_FLUSH),
      DecodebinProbe, instance, nullptr);

  instance->pipeline_is_connected_ = true;
  if (instance->pending_seek_nanosec_ != -1 &&
      instance->pipeline_is_initialised_) {
    QMetaObject::invokeMethod(instance, "Seek", Qt::QueuedConnection,
                              Q_ARG(qint64, instance->pending_seek_nanosec_));
  }
}

GstPadProbeReturn GstEnginePipeline::DecodebinProbe(GstPad* pad,
                                                    GstPadProbeInfo* info,
                                                    gpointer data) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(data);
  const GstPadProbeType info_type = GST_PAD_PROBE_INFO_TYPE(info);

  if (info_type & GST_PAD_PROBE_TYPE_BUFFER) {
    // The decodebin produced a buffer.  Record its end time, so we can offset
    // the buffers produced by the next decodebin when transitioning to the next
    // song.
    GstBuffer* buffer = GST_PAD_PROBE_INFO_BUFFER(info);

    GstClockTime timestamp = GST_BUFFER_TIMESTAMP(buffer);
    GstClockTime duration = GST_BUFFER_DURATION(buffer);
    if (timestamp == GST_CLOCK_TIME_NONE) {
      timestamp = instance->last_decodebin_segment_.position;
    }

    if (duration != GST_CLOCK_TIME_NONE) {
      timestamp += duration;
    }

    instance->last_decodebin_segment_.position = timestamp;
  } else if (info_type & GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM) {
    GstEvent* event = GST_PAD_PROBE_INFO_EVENT(info);
    GstEventType event_type = GST_EVENT_TYPE(event);

    if (event_type == GST_EVENT_SEGMENT) {
      // A new segment started, we need to save this to calculate running time
      // offsets later.
      gst_event_copy_segment(event, &instance->last_decodebin_segment_);
    } else if (event_type == GST_EVENT_FLUSH_START) {
      // A flushing seek resets the running time to 0, so remove any offset
      // we set on this pad before.
      gst_pad_set_offset(pad, 0);
    }
  }

  return GST_PAD_PROBE_OK;
}

GstPadProbeReturn GstEnginePipeline::HandoffCallback(GstPad*,
                                                     GstPadProbeInfo* info,
                                                     gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  GstBuffer* buf = gst_pad_probe_info_get_buffer(info);

  QList<BufferConsumer*> consumers;
  {
    QMutexLocker l(&instance->buffer_consumers_mutex_);
    consumers = instance->buffer_consumers_;
  }

  for (BufferConsumer* consumer : consumers) {
    gst_buffer_ref(buf);
    consumer->ConsumeBuffer(buf, instance->id());
  }

  // Calculate the end time of this buffer so we can stop playback if it's
  // after the end time of this song.
  if (instance->end_offset_nanosec_ > 0) {
    quint64 start_time = GST_BUFFER_TIMESTAMP(buf) - instance->segment_start_;
    quint64 duration = GST_BUFFER_DURATION(buf);
    quint64 end_time = start_time + duration;

    if (end_time > instance->end_offset_nanosec_) {
      if (instance->has_next_valid_url()) {
        if (instance->next_.url_ == instance->current_.url_ &&
            instance->next_beginning_offset_nanosec_ ==
                instance->end_offset_nanosec_) {
          // The "next" song is actually the next segment of this file - so
          // cheat and keep on playing, but just tell the Engine we've moved on.
          instance->end_offset_nanosec_ = instance->next_end_offset_nanosec_;
          instance->next_ = MediaPlaybackRequest();
          instance->next_beginning_offset_nanosec_ = 0;
          instance->next_end_offset_nanosec_ = 0;

          // GstEngine will try to seek to the start of the new section, but
          // we're already there so ignore it.
          instance->ignore_next_seek_ = true;
          emit instance->EndOfStreamReached(instance->id(), true);
        } else {
          // We have a next song but we can't cheat, so move to it normally.
          instance->TransitionToNext();
        }
      } else {
        // There's no next song
        emit instance->EndOfStreamReached(instance->id(), false);
      }
    }
  }

  if (instance->emit_track_ended_on_time_discontinuity_) {
    if (GST_BUFFER_FLAG_IS_SET(buf, GST_BUFFER_FLAG_DISCONT) ||
        GST_BUFFER_OFFSET(buf) < instance->last_buffer_offset_ ||
        !GST_BUFFER_OFFSET_IS_VALID(buf)) {
      qLog(Debug) << "Buffer discontinuity - emitting EOS";
      instance->emit_track_ended_on_time_discontinuity_ = false;
      emit instance->EndOfStreamReached(instance->id(), true);
    }
  }

  instance->last_buffer_offset_ = GST_BUFFER_OFFSET(buf);

  return GST_PAD_PROBE_OK;
}

GstPadProbeReturn GstEnginePipeline::EventHandoffCallback(GstPad*,
                                                          GstPadProbeInfo* info,
                                                          gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  GstEvent* e = gst_pad_probe_info_get_event(info);

  qLogCat(Debug, "GstEnginePipelineCallbacks")
      << instance->id() << "event" << GST_EVENT_TYPE_NAME(e);

  switch (GST_EVENT_TYPE(e)) {
    case GST_EVENT_SEGMENT:
      if (!instance->segment_start_received_) {
        // The segment start time is used to calculate the proper offset of data
        // buffers from the start of the stream
        const GstSegment* segment = nullptr;
        gst_event_parse_segment(e, &segment);
        instance->segment_start_ = segment->start;
        instance->segment_start_received_ = true;
      }
      break;

    default:
      break;
  }

  return GST_PAD_PROBE_OK;
}

void GstEnginePipeline::SourceDrainedCallback(GstURIDecodeBin* bin,
                                              gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);

  if (instance->has_next_valid_url() &&
      // I'm not sure why, but calling this when previous track is a local song
      // and the next track is a Spotify song is buggy: the Spotify song will
      // not start or with some offset. So just do nothing here: when the song
      // finished, EndOfStreamReached/TrackEnded will be emitted anyway so
      // NextItem will be called.
      !(instance->current_.url_.scheme() != "spotify" &&
        instance->next_.url_.scheme() == "spotify")) {
    instance->TransitionToNext();
  }
}

void GstEnginePipeline::SourceSetupCallback(GstURIDecodeBin* bin,
                                            GParamSpec* pspec, gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  GstElement* element;
  g_object_get(bin, "source", &element, nullptr);
  if (!element) {
    return;
  }

  if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "device") &&
      !instance->source_device().isEmpty()) {
    // Gstreamer is not able to handle device in URL (referring to Gstreamer
    // documentation, this might be added in the future). Despite that, for now
    // we include device inside URL: we decompose it during Init and set device
    // here, when this callback is called.
    g_object_set(element, "device",
                 instance->source_device().toLocal8Bit().constData(), nullptr);
  }

  if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "user-agent")) {
    QString user_agent =
        QString("%1 %2").arg(QCoreApplication::applicationName(),
                             QCoreApplication::applicationVersion());
    g_object_set(element, "user-agent", user_agent.toUtf8().constData(),
                 nullptr);

#ifdef Q_OS_DARWIN
    g_object_set(element, "tls-database", instance->engine_->tls_database(),
                 nullptr);
    g_object_set(element, "ssl-use-system-ca-file", false, nullptr);
    g_object_set(element, "ssl-strict", TRUE, nullptr);
#endif
  }

  if (g_object_class_find_property(G_OBJECT_GET_CLASS(element),
                                   "extra-headers")) {
    if (!instance->current_.headers_.empty()) {
      GstStructure* gheaders = gst_structure_new_empty("headers");
      QMapIterator<QByteArray, QByteArray> i(instance->current_.headers_);
      while (i.hasNext()) {
        i.next();
        qLog(Debug) << "Adding header" << i.key();
        gst_structure_set(gheaders, i.key().constData(), G_TYPE_STRING,
                          i.value().constData(), nullptr);
      }
      g_object_set(element, "extra-headers", gheaders, nullptr);
      gst_structure_free(gheaders);
    }
  }

  g_object_unref(element);
}

void GstEnginePipeline::TransitionToNext() {
  GstElement* old_decode_bin = uridecodebin_;

  ignore_tags_ = true;

  if (!ReplaceDecodeBin(next_.url_)) {
    qLog(Error) << "ReplaceDecodeBin failed with " << next_.url_;
    return;
  }
  gst_element_set_state(uridecodebin_, GST_STATE_PLAYING);
  MaybeLinkDecodeToAudio();

  current_ = next_;
  end_offset_nanosec_ = next_end_offset_nanosec_;
  next_ = MediaPlaybackRequest();
  next_beginning_offset_nanosec_ = 0;
  next_end_offset_nanosec_ = 0;

  // This function gets called when the source has been drained, even if the
  // song hasn't finished playing yet.  We'll get a new stream when it really
  // does finish, so emit TrackEnded then.
  emit_track_ended_on_stream_start_ = true;

  // This has to happen *after* the gst_element_set_state on the new bin to
  // fix an occasional race condition deadlock.
  sElementDeleter->DeleteElementLater(old_decode_bin);

  ignore_tags_ = false;
}

qint64 GstEnginePipeline::position() const {
  if (pipeline_is_initialised_)
    gst_element_query_position(pipeline_, GST_FORMAT_TIME,
                               &last_known_position_ns_);

  return last_known_position_ns_;
}

qint64 GstEnginePipeline::length() const {
  gint64 value = 0;
  gst_element_query_duration(pipeline_, GST_FORMAT_TIME, &value);

  return value;
}

GstState GstEnginePipeline::state() const {
  GstState s, sp;
  if (gst_element_get_state(pipeline_, &s, &sp, kGstStateTimeoutNanosecs) ==
      GST_STATE_CHANGE_FAILURE)
    return GST_STATE_NULL;

  return s;
}

QFuture<GstStateChangeReturn> GstEnginePipeline::SetState(GstState state) {
  return ConcurrentRun::Run<GstStateChangeReturn, GstElement*, GstState>(
      &set_state_threadpool_, &gst_element_set_state, pipeline_, state);
}

bool GstEnginePipeline::Seek(qint64 nanosec) {
  if (ignore_next_seek_) {
    ignore_next_seek_ = false;
    return true;
  }

  if (!pipeline_is_connected_ || !pipeline_is_initialised_) {
    pending_seek_nanosec_ = nanosec;
    return true;
  }

  pending_seek_nanosec_ = -1;
  last_known_position_ns_ = nanosec;
  return gst_element_seek_simple(pipeline_, GST_FORMAT_TIME,
                                 GST_SEEK_FLAG_FLUSH, nanosec);
}

void GstEnginePipeline::SetEqualizerEnabled(bool enabled) {
  eq_enabled_ = enabled;
  UpdateEqualizer();
}

void GstEnginePipeline::SetEqualizerParams(int preamp,
                                           const QList<int>& band_gains) {
  eq_preamp_ = preamp;
  eq_band_gains_ = band_gains;
  UpdateEqualizer();
}

void GstEnginePipeline::SetStereoBalance(float value) {
  stereo_balance_ = value;
  UpdateStereoBalance();
}

void GstEnginePipeline::UpdateEqualizer() {
  // Update band gains
  for (int i = 0; i < kEqBandCount; ++i) {
    float gain = eq_enabled_ ? eq_band_gains_[i] : 0.0;
    if (gain < 0)
      gain *= 0.24;
    else
      gain *= 0.12;

    const int index_in_eq = i + 1;
    // Offset because of the first dummy band we created.
    GstObject* band = GST_OBJECT(gst_child_proxy_get_child_by_index(
        GST_CHILD_PROXY(equalizer_), index_in_eq));
    g_object_set(G_OBJECT(band), "gain", gain, nullptr);
    g_object_unref(G_OBJECT(band));
  }

  // Update preamp
  float preamp = 1.0;
  if (eq_enabled_)
    preamp = float(eq_preamp_ + 100) * 0.01;  // To scale from 0.0 to 2.0

  g_object_set(G_OBJECT(equalizer_preamp_), "volume", preamp, nullptr);
}

void GstEnginePipeline::UpdateStereoBalance() {
  if (stereo_panorama_) {
    g_object_set(G_OBJECT(stereo_panorama_), "panorama", stereo_balance_,
                 nullptr);
  }
}

void GstEnginePipeline::SetVolume(int percent) {
  volume_percent_ = percent;
  UpdateVolume();
}

void GstEnginePipeline::SetVolumeModifier(qreal mod) {
  volume_modifier_ = mod;
  UpdateVolume();
}

void GstEnginePipeline::UpdateVolume() {
  float vol = double(volume_percent_) * 0.01 * volume_modifier_;
  g_object_set(G_OBJECT(volume_), "volume", vol, nullptr);
}

void GstEnginePipeline::SetOutputFormat(const QString& format) {
  qLog(Debug) << "Setting format to" << format;
  GstCaps* new_caps = gst_caps_new_simple(
      "audio/x-raw", "format", G_TYPE_STRING, format.toUtf8().data(), nullptr);
  g_object_set(capsfilter_, "caps", new_caps, nullptr);
  gst_caps_unref(new_caps);
}

void GstEnginePipeline::StartFader(qint64 duration_nanosec,
                                   QTimeLine::Direction direction,
                                   QTimeLine::CurveShape shape,
                                   bool use_fudge_timer) {
  const int duration_msec = duration_nanosec / kNsecPerMsec;

  // If there's already another fader running then start from the same time
  // that one was already at.
  int start_time = direction == QTimeLine::Forward ? 0 : duration_msec;
  if (fader_ && fader_->state() == QTimeLine::Running) {
    if (duration_msec == fader_->duration()) {
      start_time = fader_->currentTime();
    } else {
      // Calculate the position in the new fader with the same value from
      // the old fader, so no volume jumps appear
      qreal time = qreal(duration_msec) *
                   (qreal(fader_->currentTime()) / qreal(fader_->duration()));
      start_time = qRound(time);
    }
  }

  fader_.reset(new QTimeLine(duration_msec, this));
  connect(fader_.get(), SIGNAL(valueChanged(qreal)),
          SLOT(SetVolumeModifier(qreal)));
  connect(fader_.get(), SIGNAL(finished()), SLOT(FaderTimelineFinished()));
  fader_->setDirection(direction);
  fader_->setCurveShape(shape);
  fader_->setCurrentTime(start_time);
  fader_->resume();

  fader_fudge_timer_.stop();
  use_fudge_timer_ = use_fudge_timer;

  SetVolumeModifier(fader_->currentValue());
}

void GstEnginePipeline::FaderTimelineFinished() {
  fader_.reset();

  // Wait a little while longer before emitting the finished signal (and
  // probably destroying the pipeline) to account for delays in the audio
  // server/driver.
  if (use_fudge_timer_) {
    fader_fudge_timer_.start(kFaderFudgeMsec, this);
  } else {
    // Even here we cannot emit the signal directly, as it result in a
    // stutter when resuming playback. So use a quest small time, so you
    // won't notice the difference when resuming playback
    // (You get here when the pause fading is active)
    fader_fudge_timer_.start(250, this);
  }
}

void GstEnginePipeline::timerEvent(QTimerEvent* e) {
  if (e->timerId() == fader_fudge_timer_.timerId()) {
    fader_fudge_timer_.stop();
    emit FaderFinished();
    return;
  }

  QObject::timerEvent(e);
}

void GstEnginePipeline::AddBufferConsumer(BufferConsumer* consumer) {
  QMutexLocker l(&buffer_consumers_mutex_);
  buffer_consumers_ << consumer;
}

void GstEnginePipeline::RemoveBufferConsumer(BufferConsumer* consumer) {
  QMutexLocker l(&buffer_consumers_mutex_);
  buffer_consumers_.removeAll(consumer);
}

void GstEnginePipeline::RemoveAllBufferConsumers() {
  QMutexLocker l(&buffer_consumers_mutex_);
  buffer_consumers_.clear();
}

void GstEnginePipeline::SetNextReq(const MediaPlaybackRequest& req,
                                   qint64 beginning_nanosec,
                                   qint64 end_nanosec) {
  next_ = req;
  next_beginning_offset_nanosec_ = beginning_nanosec;
  next_end_offset_nanosec_ = end_nanosec;
}
