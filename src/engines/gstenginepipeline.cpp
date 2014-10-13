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

#include <limits>

#include <QCoreApplication>
#include <QDir>
#include <QUuid>

#include "bufferconsumer.h"
#include "config.h"
#include "gstelementdeleter.h"
#include "gstengine.h"
#include "gstenginepipeline.h"
#include "core/concurrentrun.h"
#include "core/logging.h"
#include "core/mac_startup.h"
#include "core/signalchecker.h"
#include "core/utilities.h"
#include "internet/internetmodel.h"
#include "internet/spotifyserver.h"
#include "internet/spotifyservice.h"

const int GstEnginePipeline::kGstStateTimeoutNanosecs = 10000000;
const int GstEnginePipeline::kFaderFudgeMsec = 2000;

const int GstEnginePipeline::kEqBandCount = 10;
const int GstEnginePipeline::kEqBandFrequencies[] = {
    60, 170, 310, 600, 1000, 3000, 6000, 12000, 14000, 16000};

int GstEnginePipeline::sId = 1;
GstElementDeleter* GstEnginePipeline::sElementDeleter = nullptr;

GstEnginePipeline::GstEnginePipeline(GstEngine* engine)
    : QObject(nullptr),
      engine_(engine),
      id_(sId++),
      valid_(false),
      sink_(GstEngine::kAutoSink),
      segment_start_(0),
      segment_start_received_(false),
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
      end_offset_nanosec_(-1),
      spotify_offset_(0),
      next_beginning_offset_nanosec_(-1),
      next_end_offset_nanosec_(-1),
      ignore_next_seek_(false),
      ignore_tags_(false),
      pipeline_is_initialised_(false),
      pipeline_is_connected_(false),
      pending_seek_nanosec_(-1),
      next_uri_set_(false),
      volume_percent_(100),
      volume_modifier_(1.0),
      pipeline_(nullptr),
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
      audiosink_(nullptr) {
  if (!sElementDeleter) {
    sElementDeleter = new GstElementDeleter;
  }

  for (int i = 0; i < kEqBandCount; ++i) eq_band_gains_ << 0;

  // FIXME Currently useless Spotify hack: we currently don't know what to do
  // when the seek is completed. We should flush the current buffers, but see
  // comments in SpotifySeekCompleted for why that doesn't work.
  // If we fix and reactivate this code, we will have another problem though:
  // calling server() try to login the user. If the user doesn't use Spotify, it
  // will receive an error message. We should have a lightweight version of
  // server() that just return it (or NULL) without trying to create it IMO to
  // avoid this issue.
  //if (InternetModel::Service<SpotifyService>()->IsBlobInstalled()) {
  //  connect(InternetModel::Service<SpotifyService>()->server(), SIGNAL(SeekCompleted()),
  //      SLOT(SpotifySeekCompleted()));
  //}
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

bool GstEnginePipeline::InitDecodeBin(GstElement* decode_bin) {
  if (!decode_bin) return false;

  pipeline_ = gst_pipeline_new("pipeline");

  gst_bin_add(GST_BIN(pipeline_), decode_bin);

  if (!InitAudioBin()) return false;
  gst_bin_add(GST_BIN(pipeline_), audiobin_);

  gst_element_link(decode_bin, audiobin_);

  return true;
}

GstElement* GstEnginePipeline::CreateSpotifyBin(const QUrl& url) {
  GstElement* new_bin = gst_bin_new("spotify_bin");

  // Create elements
  GstElement* src = engine_->CreateElement("tcpserversrc", new_bin);
  GstElement* gdp = engine_->CreateElement("gdpdepay", new_bin);
  if (!src || !gdp) return nullptr;

  // Pick a port number
  const int port = Utilities::PickUnusedPort();
  g_object_set(G_OBJECT(src), "host", "127.0.0.1", nullptr);
  g_object_set(G_OBJECT(src), "port", port, nullptr);

  // Link the elements
  gst_element_link(src, gdp);

  // Add a ghost pad
  GstPad* pad = gst_element_get_static_pad(gdp, "src");
  gst_element_add_pad(GST_ELEMENT(new_bin), gst_ghost_pad_new("src", pad));
  gst_object_unref(GST_OBJECT(pad));

  // Tell spotify to start sending data to us.
  InternetModel::Service<SpotifyService>()->server()->StartPlaybackLater(
      url.toString(), port);
  spotify_offset_ = 0;

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
  //   queue ! audioconvert ! <caps32>
  //         ! ( rgvolume ! rglimiter ! audioconvert2 ) ! tee
  // rgvolume and rglimiter are only created when replaygain is enabled.

  // After the tee the pipeline splits.  One split is converted to 16-bit int
  // samples for the scope, the other is kept as float32 and sent to the
  // speaker.
  //   tee1 ! probe_queue ! probe_converter ! <caps16> ! probe_sink
  //   tee2 ! audio_queue ! equalizer_preamp ! equalizer ! volume ! audioscale
  //        ! convert ! audiosink

  // Audio bin
  audiobin_ = gst_bin_new("audiobin");

  // Create the sink
  if (!(audiosink_ = engine_->CreateElement(sink_, audiobin_))) return false;

  if (g_object_class_find_property(G_OBJECT_GET_CLASS(audiosink_), "device") &&
      !device_.toString().isEmpty()) {
    switch (device_.type()) {
      case QVariant::Int:
        g_object_set(G_OBJECT(audiosink_),
                     "device", device_.toInt(),
                     nullptr);
        break;
      case QVariant::String:
        g_object_set(G_OBJECT(audiosink_),
                     "device", device_.toString().toUtf8().constData(),
                     nullptr);
        break;

      #ifdef Q_OS_WIN32
      case QVariant::ByteArray: {
        GUID guid = QUuid(device_.toByteArray());
        g_object_set(G_OBJECT(audiosink_),
                     "device", &guid,
                     nullptr);
        break;
      }
      #endif  // Q_OS_WIN32

      default:
        qLog(Warning) << "Unknown device type" << device_;
        break;
    }
  }

  // Create all the other elements
  GstElement* tee, *probe_queue, *probe_converter, *probe_sink, *audio_queue,
      *convert;

  queue_ = engine_->CreateElement("queue2", audiobin_);
  audioconvert_ = engine_->CreateElement("audioconvert", audiobin_);
  tee = engine_->CreateElement("tee", audiobin_);

  probe_queue = engine_->CreateElement("queue", audiobin_);
  probe_converter = engine_->CreateElement("audioconvert", audiobin_);
  probe_sink = engine_->CreateElement("fakesink", audiobin_);

  audio_queue = engine_->CreateElement("queue", audiobin_);
  equalizer_preamp_ = engine_->CreateElement("volume", audiobin_);
  equalizer_ = engine_->CreateElement("equalizer-nbands", audiobin_);
  stereo_panorama_ = engine_->CreateElement("audiopanorama", audiobin_);
  volume_ = engine_->CreateElement("volume", audiobin_);
  audioscale_ = engine_->CreateElement("audioresample", audiobin_);
  convert = engine_->CreateElement("audioconvert", audiobin_);

  if (!queue_ || !audioconvert_ || !tee || !probe_queue || !probe_converter ||
      !probe_sink || !audio_queue || !equalizer_preamp_ || !equalizer_ ||
      !stereo_panorama_ || !volume_ || !audioscale_ || !convert) {
    return false;
  }

  // Create the replaygain elements if it's enabled.  event_probe is the
  // audioconvert element we attach the probe to, which will change depending
  // on whether replaygain is enabled.  convert_sink is the element after the
  // first audioconvert, which again will change.
  GstElement* event_probe = audioconvert_;
  GstElement* convert_sink = tee;

  if (rg_enabled_) {
    rgvolume_ = engine_->CreateElement("rgvolume", audiobin_);
    rglimiter_ = engine_->CreateElement("rglimiter", audiobin_);
    audioconvert2_ = engine_->CreateElement("audioconvert", audiobin_);
    event_probe = audioconvert2_;
    convert_sink = rgvolume_;

    if (!rgvolume_ || !rglimiter_ || !audioconvert2_) {
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
  gst_pad_add_probe(
      pad, GST_PAD_PROBE_TYPE_EVENT_UPSTREAM, &EventHandoffCallback, this, NULL);
  gst_object_unref(pad);

  // Configure the fakesink properly
  g_object_set(G_OBJECT(probe_sink), "sync", TRUE, nullptr);

  // Set the equalizer bands
  g_object_set(G_OBJECT(equalizer_), "num-bands", 10, nullptr);

  int last_band_frequency = 0;
  for (int i = 0; i < kEqBandCount; ++i) {
    GstObject* band = GST_OBJECT(gst_child_proxy_get_child_by_index(
        GST_CHILD_PROXY(equalizer_), i));

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

  gst_element_link_many(queue_, audioconvert_, convert_sink, nullptr);

  // Link the elements with special caps
  // The scope path through the tee gets 16-bit ints.
  GstCaps* caps16 = gst_caps_new_simple ("audio/x-raw",
                                         "format", G_TYPE_STRING, "S16LE",
                                         NULL);
  gst_element_link_filtered(probe_converter, probe_sink, caps16);
  gst_caps_unref(caps16);

  // Link the outputs of tee to the queues on each path.
  gst_pad_link(gst_element_get_request_pad(tee, "src_%u"),
               gst_element_get_static_pad(probe_queue, "sink"));
  gst_pad_link(gst_element_get_request_pad(tee, "src_%u"),
               gst_element_get_static_pad(audio_queue, "sink"));

  // Link replaygain elements if enabled.
  if (rg_enabled_) {
    gst_element_link_many(rgvolume_, rglimiter_, audioconvert2_, tee, nullptr);
  }

  // Link everything else.
  gst_element_link(probe_queue, probe_converter);
  gst_element_link_many(audio_queue, equalizer_preamp_, equalizer_,
                        stereo_panorama_, volume_, audioscale_, convert,
                        audiosink_, nullptr);

  // Add probes and handlers.
  gst_pad_add_probe(gst_element_get_static_pad(probe_converter, "src"),
                    GST_PAD_PROBE_TYPE_BUFFER,
                    HandoffCallback, this, nullptr);
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)),
                           BusCallbackSync, this, nullptr);
  bus_cb_id_ = gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)),
                                 BusCallback, this);

  return true;
}

bool GstEnginePipeline::InitFromString(const QString& pipeline) {
  GstElement* new_bin =
      CreateDecodeBinFromString(pipeline.toAscii().constData());

  return InitDecodeBin(new_bin);
}

bool GstEnginePipeline::InitFromUrl(const QUrl& url, qint64 end_nanosec) {
  if (url.scheme() == "cdda" && !url.path().isEmpty()) {
    // Currently, Gstreamer can't handle input CD devices inside cdda URL. So
    // we handle them ourselve: we extract the track number and re-create an
    // URL with only cdda:// + the track number (which can be handled by
    // Gstreamer). We keep the device in mind, and we will set it later using
    // SourceSetupCallback
    QStringList path = url.path().split('/');
    url_ = QUrl(QString("cdda://%1").arg(path.takeLast()));
    source_device_ = path.join("/");
  } else {
    url_ = url;
  }
  end_offset_nanosec_ = end_nanosec;

  if (url_.scheme() == "spotify") {
    GstElement* new_bin = CreateSpotifyBin(url_);
    return InitDecodeBin(new_bin);
  }

  pipeline_ = engine_->CreateElement("playbin");
  g_object_set(G_OBJECT(pipeline_), "uri", url_.toEncoded().constData(),
               nullptr);
  CHECKED_GCONNECT(G_OBJECT(pipeline_), "about-to-finish",
                   &AboutToFinishCallback, this);

  CHECKED_GCONNECT(G_OBJECT(pipeline_), "pad-added", &NewPadCallback, this);
  CHECKED_GCONNECT(G_OBJECT(pipeline_), "notify::source", &SourceSetupCallback,
                   this);

  if (!InitAudioBin()) return false;

  // Set playbin's sink to be our costum audio-sink.
  g_object_set(GST_OBJECT(pipeline_), "audio-sink", audiobin_, NULL);
  pipeline_is_connected_ = true;
  return true;
}

GstEnginePipeline::~GstEnginePipeline() {
  if (pipeline_) {
    gst_bus_set_sync_handler(
        gst_pipeline_get_bus(GST_PIPELINE(pipeline_)),
        nullptr, nullptr, nullptr);
    g_source_remove(bus_cb_id_);
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline_));
  }
}

gboolean GstEnginePipeline::BusCallback(GstBus*, GstMessage* msg,
                                        gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);

  qLog(Debug) << instance->id() << "bus message" << GST_MESSAGE_TYPE_NAME(msg);

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

  qLog(Debug) << instance->id() << "sync bus message"
              << GST_MESSAGE_TYPE_NAME(msg);

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
      instance->StreamStartMessageReceived();
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

void GstEnginePipeline::StreamStartMessageReceived() {
  if (next_uri_set_) {
    next_uri_set_ = false;

    url_ = next_url_;
    end_offset_nanosec_ = next_end_offset_nanosec_;
    next_url_ = QUrl();
    next_beginning_offset_nanosec_ = 0;
    next_end_offset_nanosec_ = 0;

    emit EndOfStreamReached(id(), true);
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
  free(debugs);

  if (pipeline_is_initialised_ && next_uri_set_ &&
      (domain == GST_RESOURCE_ERROR || domain == GST_STREAM_ERROR)) {
    // A track is still playing and the next uri is not playable. We ignore the
    // error here so it can play until the end.
    // But there is no message send to the bus when the current track finishes,
    // we have to add an EOS ourself.
    qLog(Debug) << "Ignoring error when loading next track";
    GstPad* sinkpad = gst_element_get_static_pad(audiobin_, "sink");
    gst_pad_send_event(sinkpad, gst_event_new_eos());
    gst_object_unref(sinkpad);
    return;
  }

  if (!redirect_url_.isEmpty() &&
      debugstr.contains(
          "A redirect message was posted on the bus and should have been "
          "handled by the application.")) {
    // mmssrc posts a message on the bus *and* makes an error message when it
    // wants to do a redirect.  We handle the message, but now we have to
    // ignore the error too.
    return;
  }

  qLog(Error) << id() << debugstr;

  emit Error(id(), message, domain, code);
}

void GstEnginePipeline::TagMessageReceived(GstMessage* msg) {
  GstTagList* taglist = nullptr;
  gst_message_parse_tag(msg, &taglist);

  Engine::SimpleMetaBundle bundle;
  bundle.title = ParseTag(taglist, GST_TAG_TITLE);
  bundle.artist = ParseTag(taglist, GST_TAG_ARTIST);
  bundle.comment = ParseTag(taglist, GST_TAG_COMMENT);
  bundle.album = ParseTag(taglist, GST_TAG_ALBUM);

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

    if (next_uri_set_ && new_state == GST_STATE_READY) {
      // Revert uri and go back to PLAY state again
      next_uri_set_ = false;
      g_object_set(G_OBJECT(pipeline_), "uri", url_.toEncoded().constData(),
                   nullptr);
      SetState(GST_STATE_PLAYING);
    }
  }
}

void GstEnginePipeline::BufferingMessageReceived(GstMessage* msg) {
  // Only handle buffering messages from the queue2 element in audiobin - not
  // the one that's created automatically by uridecodebin.
  if (GST_ELEMENT(GST_MESSAGE_SRC(msg)) != queue_) {
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

void GstEnginePipeline::NewPadCallback(GstElement*, GstPad* pad,
                                       gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  GstPad* const audiopad =
      gst_element_get_static_pad(instance->audiobin_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Warning) << instance->id()
                  << "audiopad is already linked, unlinking old pad";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);

  gst_object_unref(audiopad);

  instance->pipeline_is_connected_ = true;
  if (instance->pending_seek_nanosec_ != -1 &&
      instance->pipeline_is_initialised_) {
    QMetaObject::invokeMethod(instance, "Seek", Qt::QueuedConnection,
                              Q_ARG(qint64, instance->pending_seek_nanosec_));
  }
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
      if (instance->has_next_valid_url() &&
          instance->next_url_ == instance->url_ &&
          instance->next_beginning_offset_nanosec_ ==
              instance->end_offset_nanosec_) {
        // The "next" song is actually the next segment of this file - so
        // cheat and keep on playing, but just tell the Engine we've moved on.
        instance->end_offset_nanosec_ = instance->next_end_offset_nanosec_;
        instance->next_url_ = QUrl();
        instance->next_beginning_offset_nanosec_ = 0;
        instance->next_end_offset_nanosec_ = 0;

        // GstEngine will try to seek to the start of the new section, but
        // we're already there so ignore it.
        instance->ignore_next_seek_ = true;
        emit instance->EndOfStreamReached(instance->id(), true);
      } else {
        // There's no next song or we can't keep on playing.
        emit instance->EndOfStreamReached(instance->id(), false);
      }
    }
  }

  return GST_PAD_PROBE_OK;
}

GstPadProbeReturn GstEnginePipeline::EventHandoffCallback(GstPad*,
                                                          GstPadProbeInfo* info,
                                                          gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  GstEvent* e = gst_pad_probe_info_get_event(info);

  qLog(Debug) << instance->id() << "event" << GST_EVENT_TYPE_NAME(e);

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

void GstEnginePipeline::AboutToFinishCallback(GstPlayBin* bin, gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  if (instance->has_next_valid_url() && !instance->next_uri_set_ &&
      instance->url_.scheme() != "spotify") {
    // Set the next uri. When the current song ends it will be played
    // automatically and a STREAM_START message is send to the bus.
    // When the next uri is not playable an error message is send when the
    // pipeline goes to PLAY (or PAUSE) state or immediately if it is currently
    // in PLAY state.
    instance->next_uri_set_ = true;
    g_object_set(G_OBJECT(instance->pipeline_), "uri",
                 instance->next_url_.toEncoded().constData(), nullptr);
  }
}

void GstEnginePipeline::SourceSetupCallback(GstPlayBin* bin, GParamSpec* pspec,
                                            gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  GstElement* element;
  g_object_get(bin, "source", &element, nullptr);
  if (!element) {
    return;
  }

  if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "device") &&
      !instance->source_device().isEmpty()) {
    // Gstreamer is not able to handle device in URL (refering to Gstreamer
    // documentation, this might be added in the future). Despite that, for now
    // we include device inside URL: we decompose it during Init and set device
    // here, when this callback is called.
    g_object_set(element, "device",
                 instance->source_device().toLocal8Bit().constData(), nullptr);
  }
  if (g_object_class_find_property(G_OBJECT_GET_CLASS(element),
                                   "extra-headers") &&
      instance->url().host().contains("grooveshark")) {
    // Grooveshark streaming servers will answer with a 400 error 'Bad request'
    // if we don't specify 'Range' field in HTTP header.
    // Maybe it could be useful in some other cases, but for now, I prefer to
    // keep this grooveshark specific.
    GstStructure* headers;
    headers = gst_structure_new("extra-headers", "Range", G_TYPE_STRING,
                                "bytes=0-", nullptr);
    g_object_set(element, "extra-headers", headers, nullptr);
    gst_structure_free(headers);
  }

  if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "user-agent")) {
    QString user_agent =
        QString("%1 %2").arg(QCoreApplication::applicationName(),
                             QCoreApplication::applicationVersion());
    g_object_set(element, "user-agent", user_agent.toUtf8().constData(),
                 nullptr);

#ifdef Q_OS_DARWIN
    // Override the CA cert path for Soup on Mac to our shipped version.
    QDir resources_dir(mac::GetResourcesPath());
    QString ca_cert_path = resources_dir.filePath("cacert.pem");
    g_object_set(element, "ssl-use-system-ca-file", false, nullptr);
    g_object_set(element, "ssl-ca-file", ca_cert_path.toUtf8().data(), nullptr);
#endif
  }

  // If the pipeline was buffering we stop that now.
  if (instance->buffering_) {
    instance->buffering_ = false;
    emit instance->BufferingFinished();
    instance->SetState(GST_STATE_PLAYING);
  }
}

qint64 GstEnginePipeline::position() const {
  gint64 value = 0;
  gst_element_query_position(pipeline_, GST_FORMAT_TIME, &value);

  if (url_.scheme() == "spotify") {
    value += spotify_offset_;
  }

  return value;
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
  if (url_.scheme() == "spotify" && !buffering_) {
    const GstState current_state = this->state();

    if (state == GST_STATE_PAUSED && current_state == GST_STATE_PLAYING) {
      SpotifyService* spotify = InternetModel::Service<SpotifyService>();

      // Need to schedule this in the spotify service's thread
      QMetaObject::invokeMethod(spotify, "SetPaused", Qt::QueuedConnection,
                                Q_ARG(bool, true));
    } else if (state == GST_STATE_PLAYING &&
               current_state == GST_STATE_PAUSED) {
      SpotifyService* spotify = InternetModel::Service<SpotifyService>();

      // Need to schedule this in the spotify service's thread
      QMetaObject::invokeMethod(spotify, "SetPaused", Qt::QueuedConnection,
                                Q_ARG(bool, false));
    }
  }
  return ConcurrentRun::Run<GstStateChangeReturn, GstElement*, GstState>(
      &set_state_threadpool_, &gst_element_set_state, pipeline_, state);
}

bool GstEnginePipeline::Seek(qint64 nanosec) {
  if (ignore_next_seek_) {
    ignore_next_seek_ = false;
    return true;
  }

  if (url_.scheme() == "spotify" && !buffering_) {
    SpotifyService* spotify = InternetModel::Service<SpotifyService>();
    // Need to schedule this in the spotify service's thread
    QMetaObject::invokeMethod(spotify, "Seek", Qt::QueuedConnection,
                              Q_ARG(qint64, nanosec));
    // Need to reset spotify_offset_ to get the real pipeline position, as it is
    // used in position()
    spotify_offset_ = nanosec - position();
    return true;
  }

  if (!pipeline_is_connected_ || !pipeline_is_initialised_) {
    pending_seek_nanosec_ = nanosec;
    return true;
  }

  if (next_uri_set_) {
    qDebug() << "MYTODO: gstenginepipeline.seek: seeking after Transition";

    pending_seek_nanosec_ = nanosec;
    SetState(GST_STATE_READY);
    return true;
  }

  pending_seek_nanosec_ = -1;
  return gst_element_seek_simple(pipeline_, GST_FORMAT_TIME,
                                 GST_SEEK_FLAG_FLUSH, nanosec);
}

void GstEnginePipeline::SpotifySeekCompleted() {
  qLog(Debug) << "Spotify Seek completed";
  // FIXME: we should clear buffers to start playing data from seek point right
  // now (currently there is small delay) but I didn't managed to tell gstreamer
  // to do this without breaking the streaming completely...
  // Funny thing to notice: for me the delay varies when changing buffer size,
  // but a larger buffer doesn't necessary increase the delay.
  // FIXME: also, this method is never called currently (see constructor)
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

    GstObject* band = GST_OBJECT(
        gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(equalizer_), i));
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
  // probably distroying the pipeline) to account for delays in the audio
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

void GstEnginePipeline::SetNextUrl(const QUrl& url, qint64 beginning_nanosec,
                                   qint64 end_nanosec) {
  next_url_ = url;
  next_beginning_offset_nanosec_ = beginning_nanosec;
  next_end_offset_nanosec_ = end_nanosec;
}
