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

#include "bufferconsumer.h"
#include "gstelementdeleter.h"
#include "gstengine.h"
#include "gstenginepipeline.h"
#include "core/logging.h"

#include <QtConcurrentRun>

const int GstEnginePipeline::kGstStateTimeoutNanosecs = 10000000;
const int GstEnginePipeline::kFaderFudgeMsec = 2000;

const int GstEnginePipeline::kEqBandCount = 10;
const int GstEnginePipeline::kEqBandFrequencies[] = {
  60, 170, 310, 600, 1000, 3000, 6000, 12000, 14000, 16000};

int GstEnginePipeline::sId = 1;
GstElementDeleter* GstEnginePipeline::sElementDeleter = NULL;


GstEnginePipeline::GstEnginePipeline(GstEngine* engine)
  : QObject(NULL),
    engine_(engine),
    id_(sId++),
    valid_(false),
    sink_(GstEngine::kAutoSink),
    segment_start_(0),
    segment_start_received_(false),
    emit_track_ended_on_segment_start_(false),
    eq_enabled_(false),
    eq_preamp_(0),
    rg_enabled_(false),
    rg_mode_(0),
    rg_preamp_(0.0),
    rg_compression_(true),
    buffer_duration_nanosec_(1 * kNsecPerSec),
    end_offset_nanosec_(-1),
    next_beginning_offset_nanosec_(-1),
    next_end_offset_nanosec_(-1),
    ignore_next_seek_(false),
    ignore_tags_(false),
    pipeline_is_initialised_(false),
    pipeline_is_connected_(false),
    pending_seek_nanosec_(-1),
    volume_percent_(100),
    volume_modifier_(1.0),
    fader_(NULL),
    pipeline_(NULL),
    tcpsrc_(NULL),
    uridecodebin_(NULL),
    audiobin_(NULL),
    queue_(NULL),
    audioconvert_(NULL),
    rgvolume_(NULL),
    rglimiter_(NULL),
    audioconvert2_(NULL),
    equalizer_(NULL),
    volume_(NULL),
    audioscale_(NULL),
    audiosink_(NULL)
{
  if (!sElementDeleter) {
    sElementDeleter = new GstElementDeleter;
  }

  for (int i=0 ; i<kEqBandCount ; ++i)
    eq_band_gains_ << 0;
}

void GstEnginePipeline::set_output_device(const QString &sink, const QString &device) {
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

void GstEnginePipeline::set_buffer_duration_nanosec(qint64 buffer_duration_nanosec) {
  buffer_duration_nanosec_ = buffer_duration_nanosec;
}

bool GstEnginePipeline::ReplaceDecodeBin(GstElement* new_bin,
                                         GstElement* new_tcpsrc) {
  if (!new_bin) return false;

  // Destroy the old elements if they are set
  // Note that the caller to this function MUST schedule the old uridecodebin_
  // or tcpsrc_ for deletion in the main thread.
  if (uridecodebin_) {
    gst_bin_remove(GST_BIN(pipeline_), uridecodebin_);
  }
  if (tcpsrc_) {
    gst_bin_remove(GST_BIN(pipeline_), tcpsrc_);
  }

  uridecodebin_ = new_bin;
  tcpsrc_ = new_tcpsrc;
  segment_start_ = 0;
  segment_start_received_ = false;
  pipeline_is_connected_ = false;
  gst_bin_add(GST_BIN(pipeline_), uridecodebin_);

  if (new_tcpsrc) {
    gst_bin_add(GST_BIN(pipeline_), tcpsrc_);
    gst_element_link(tcpsrc_, uridecodebin_);
  }

  return true;
}

bool GstEnginePipeline::ReplaceDecodeBin(const QUrl& url) {
  if (url.scheme() == "tcp") {
    qLog(Info) << "Listening on TCP port" << url.port();

    // Hackity hack
    GstElement* src = engine_->CreateElement("tcpserversrc");
    g_object_set(G_OBJECT(src), "host", url.host().toUtf8().constData(), NULL);
    g_object_set(G_OBJECT(src), "port", url.port(), NULL);

    GstElement* decodebin = engine_->CreateElement("decodebin2");
    g_signal_connect(G_OBJECT(decodebin), "drained", G_CALLBACK(SourceDrainedCallback), this);
    g_signal_connect(G_OBJECT(decodebin), "pad-added", G_CALLBACK(NewPadCallback), this);

    return ReplaceDecodeBin(decodebin, src);
  } else {
    GstElement* new_bin = engine_->CreateElement("uridecodebin");
    g_object_set(G_OBJECT(new_bin), "uri", url.toEncoded().constData(), NULL);
    g_object_set(G_OBJECT(new_bin), "buffer-duration", buffer_duration_nanosec_, NULL);
    g_object_set(G_OBJECT(new_bin), "download", true, NULL);
    g_object_set(G_OBJECT(new_bin), "use-buffering", true, NULL);
    g_signal_connect(G_OBJECT(new_bin), "drained", G_CALLBACK(SourceDrainedCallback), this);
    g_signal_connect(G_OBJECT(new_bin), "pad-added", G_CALLBACK(NewPadCallback), this);
    g_signal_connect(G_OBJECT(new_bin), "notify::source", G_CALLBACK(SourceSetupCallback), this);
    return ReplaceDecodeBin(new_bin);
  }
}

GstElement* GstEnginePipeline::CreateDecodeBinFromString(const char* pipeline) {
  GError* error = NULL;
  GstElement* bin = gst_parse_bin_from_description(pipeline, TRUE, &error);

  if (error) {
    QString message = QString::fromLocal8Bit(error->message);
    int domain = error->domain;
    int code = error->code;
    g_error_free(error);

    qLog(Warning) << message;
    emit Error(id(), message, domain, code);

    return NULL;
  } else {
    return bin;
  }
}

bool GstEnginePipeline::Init() {
  // Here we create all the parts of the gstreamer pipeline - from the source
  // to the sink.  The parts of the pipeline are split up into bins:
  //   uri decode bin -> audio bin
  // The uri decode bin is a gstreamer builtin that automatically picks the
  // right type of source and decoder for the URI.
  // The audio bin gets created here and contains:
  //   queue -> audioconvert -> rgvolume -> rglimiter -> equalizer_preamp ->
  //   equalizer -> volume -> audioscale -> audioconvert -> audiosink

  // Audio bin
  audiobin_ = gst_bin_new("audiobin");
  gst_bin_add(GST_BIN(pipeline_), audiobin_);

  if (!(audiosink_ = engine_->CreateElement(sink_, audiobin_)))
    return false;

  if (GstEngine::DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(sink_) && !device_.isEmpty())
    g_object_set(G_OBJECT(audiosink_), "device", device_.toUtf8().constData(), NULL);

  if (!(queue_ = engine_->CreateElement("queue", audiobin_))) { return false; }
  if (!(equalizer_preamp_ = engine_->CreateElement("volume", audiobin_))) { return false; }
  if (!(equalizer_ = engine_->CreateElement("equalizer-nbands", audiobin_))) { return false; }
  if (!(audioconvert_ = engine_->CreateElement("audioconvert", audiobin_))) { return false; }
  if (!(volume_ = engine_->CreateElement("volume", audiobin_))) { return false; }
  if (!(audioscale_ = engine_->CreateElement("audioresample", audiobin_))) { return false; }
  GstElement* scope_element = audioconvert_;

  if (rg_enabled_) {
    if (!(rgvolume_ = engine_->CreateElement("rgvolume", audiobin_))) { return false; }
    if (!(rglimiter_ = engine_->CreateElement("rglimiter", audiobin_))) { return false; }
    if (!(audioconvert2_ = engine_->CreateElement("audioconvert", audiobin_))) { return false; }
    scope_element = audioconvert2_;

    // Set replaygain settings
    g_object_set(G_OBJECT(rgvolume_), "album-mode", rg_mode_, NULL);
    g_object_set(G_OBJECT(rgvolume_), "pre-amp", double(rg_preamp_), NULL);
    g_object_set(G_OBJECT(rglimiter_), "enabled", int(rg_compression_), NULL);
  }

  GstPad* pad = gst_element_get_pad(queue_, "sink");
  gst_element_add_pad(audiobin_, gst_ghost_pad_new("sink", pad));
  gst_object_unref(pad);

  // Add a data probe on the src pad of the audioconvert element for our scope.
  // We do it here because we want pre-equalized and pre-volume samples
  // so that our visualization are not affected by them
  pad = gst_element_get_pad(scope_element, "src");
  gst_pad_add_buffer_probe(pad, G_CALLBACK(HandoffCallback), this);
  gst_pad_add_event_probe(pad, G_CALLBACK(EventHandoffCallback), this);
  gst_object_unref (pad);

  // Set the equalizer bands
  g_object_set(G_OBJECT(equalizer_), "num-bands", 10, NULL);

  int last_band_frequency = 0;
  for (int i=0 ; i<kEqBandCount ; ++i) {
    GstObject* band = gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(equalizer_), i);

    const float frequency = kEqBandFrequencies[i];
    const float bandwidth = frequency - last_band_frequency;
    last_band_frequency = frequency;

    g_object_set(G_OBJECT(band), "freq", frequency,
                                "bandwidth", bandwidth,
                                "gain", 0.0f, NULL);
    g_object_unref(G_OBJECT(band));
  }

  // Set the buffer duration.  We set this on the queue as well as on the
  // decode bin (in ReplaceDecodeBin()) because setting it on the decode bin
  // only affects network sources.
  g_object_set(G_OBJECT(queue_), "max-size-time", buffer_duration_nanosec_, NULL);

  // Ensure we get the right type out of audioconvert for our scope
  GstCaps* caps = gst_caps_new_simple ("audio/x-raw-int",
      "width", G_TYPE_INT, 16,
      "signed", G_TYPE_BOOLEAN, true,
      NULL);
  gst_element_link_filtered(scope_element, equalizer_preamp_, caps);
  gst_caps_unref(caps);

  // Add an extra audioconvert at the end as osxaudiosink supports only one format.
  GstElement* convert = engine_->CreateElement("audioconvert", audiobin_);
  if (!convert) { return false; }

  gst_element_link(queue_, audioconvert_);
  if (rg_enabled_)
    gst_element_link_many(audioconvert_, rgvolume_, rglimiter_, audioconvert2_, NULL);
  gst_element_link_many(equalizer_preamp_, equalizer_, volume_, audioscale_, convert, audiosink_, NULL);

  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), BusCallbackSync, this);
  bus_cb_id_ = gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), BusCallback, this);
  return true;
}

bool GstEnginePipeline::InitFromString(const QString& pipeline) {
  pipeline_ = gst_pipeline_new("pipeline");

  GstElement* new_bin = CreateDecodeBinFromString(pipeline.toAscii().constData());
  if (!new_bin) {
    return false;
  }

  if (!ReplaceDecodeBin(new_bin)) return false;

  if (!Init()) return false;
  return gst_element_link(new_bin, audiobin_);
}

bool GstEnginePipeline::InitFromUrl(const QUrl &url, qint64 end_nanosec) {
  pipeline_ = gst_pipeline_new("pipeline");
  
  if (url.scheme() == "cdda") {
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

  // Decode bin
  if (!ReplaceDecodeBin(url_)) return false;

  return Init();
}

GstEnginePipeline::~GstEnginePipeline() {
  if (pipeline_) {
    gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), NULL, NULL);
    g_source_remove(bus_cb_id_);
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline_));
  }
}



gboolean GstEnginePipeline::BusCallback(GstBus*, GstMessage* msg, gpointer self) {
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

GstBusSyncReply GstEnginePipeline::BusCallbackSync(GstBus*, GstMessage* msg, gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);

  qLog(Debug) << instance->id() << "sync bus message" << GST_MESSAGE_TYPE_NAME(msg);

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

    default:
      break;
  }

  return GST_BUS_PASS;
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

  if (!redirect_url_.isEmpty() && debugstr.contains(
      "A redirect message was posted on the bus and should have been handled by the application.")) {
    // mmssrc posts a message on the bus *and* makes an error message when it
    // wants to do a redirect.  We handle the message, but now we have to
    // ignore the error too.
    return;
  }

  qLog(Error) << id() << debugstr;

  emit Error(id(), message, domain, code);
}

void GstEnginePipeline::TagMessageReceived(GstMessage* msg) {
  GstTagList* taglist = NULL;
  gst_message_parse_tag(msg, &taglist);

  Engine::SimpleMetaBundle bundle;
  bundle.title = ParseTag(taglist, GST_TAG_TITLE);
  bundle.artist = ParseTag(taglist, GST_TAG_ARTIST);
  bundle.comment = ParseTag(taglist, GST_TAG_COMMENT);
  bundle.album = ParseTag(taglist, GST_TAG_ALBUM);

  gst_tag_list_free(taglist);

  if (ignore_tags_)
    return;

  if (!bundle.title.isEmpty() || !bundle.artist.isEmpty() ||
      !bundle.comment.isEmpty() || !bundle.album.isEmpty())
    emit MetadataFound(id(), bundle);
}

QString GstEnginePipeline::ParseTag(GstTagList* list, const char* tag) const {
  gchar* data = NULL;
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

  if (!pipeline_is_initialised_ && (new_state == GST_STATE_PAUSED || new_state == GST_STATE_PLAYING)) {
    pipeline_is_initialised_ = true;
    if (pending_seek_nanosec_ != -1 && pipeline_is_connected_) {
      QMetaObject::invokeMethod(this, "Seek", Qt::QueuedConnection,
                                Q_ARG(qint64, pending_seek_nanosec_));
    }
  }

  if (pipeline_is_initialised_ && new_state != GST_STATE_PAUSED && new_state != GST_STATE_PLAYING) {
    pipeline_is_initialised_ = false;
  }
}

void GstEnginePipeline::NewPadCallback(GstElement*, GstPad* pad, gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  GstPad* const audiopad = gst_element_get_pad(instance->audiobin_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Warning) << instance->id() << "audiopad is already linked, unlinking old pad";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);

  gst_object_unref(audiopad);

  instance->pipeline_is_connected_ = true;
  if (instance->pending_seek_nanosec_ != -1 && instance->pipeline_is_initialised_) {
    QMetaObject::invokeMethod(instance, "Seek", Qt::QueuedConnection,
                              Q_ARG(qint64, instance->pending_seek_nanosec_));
  }
}

bool GstEnginePipeline::HandoffCallback(GstPad*, GstBuffer* buf, gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);

  QList<BufferConsumer*> consumers;
  {
    QMutexLocker l(&instance->buffer_consumers_mutex_);
    consumers = instance->buffer_consumers_;
  }

  foreach (BufferConsumer* consumer, consumers) {
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
        if (instance->next_url_ == instance->url_ &&
            instance->next_beginning_offset_nanosec_ == instance->end_offset_nanosec_) {
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
          // We have a next song but we can't cheat, so move to it normally.
          instance->TransitionToNext();
        }
      } else {
        // There's no next song
        emit instance->EndOfStreamReached(instance->id(), false);
      }
    }
  }

  return true;
}

bool GstEnginePipeline::EventHandoffCallback(GstPad*, GstEvent* e, gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);

  qLog(Debug) << instance->id() << "event" << GST_EVENT_TYPE_NAME(e);

  if (GST_EVENT_TYPE(e) == GST_EVENT_NEWSEGMENT && !instance->segment_start_received_) {
    // The segment start time is used to calculate the proper offset of data
    // buffers from the start of the stream
    gint64 start = 0;
    gst_event_parse_new_segment(e, NULL, NULL, NULL, &start, NULL, NULL);
    instance->segment_start_ = start;
    instance->segment_start_received_ = true;

    if (instance->emit_track_ended_on_segment_start_) {
      instance->emit_track_ended_on_segment_start_ = false;
      emit instance->EndOfStreamReached(instance->id(), true);
    }
  }

  return true;
}

void GstEnginePipeline::SourceDrainedCallback(GstURIDecodeBin* bin, gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);

  if (instance->has_next_valid_url()) {
    instance->TransitionToNext();
  }
}

void GstEnginePipeline::SourceSetupCallback(GstURIDecodeBin* bin, GParamSpec *pspec, gpointer self) {
  GstEnginePipeline* instance = reinterpret_cast<GstEnginePipeline*>(self);
  GstElement* element;
  g_object_get(bin, "source", &element, NULL);
  if (element &&
      g_object_class_find_property(G_OBJECT_GET_CLASS(element), "device")) {
    // Gstreamer is not able to handle device in URL (refering to Gstreamer
    // documentation, this might be added in the future). Despite that, for now
    // we include device inside URL: we decompose it during Init and set device
    // here, when this callback is called.
    g_object_set(element, "device", instance->source_device().toLocal8Bit().constData(), NULL);
  }
}

void GstEnginePipeline::TransitionToNext() {
  GstElement* old_decode_bin = uridecodebin_;
  GstElement* old_tcpsrc = tcpsrc_;

  ignore_tags_ = true;

  ReplaceDecodeBin(next_url_);
  gst_element_set_state(uridecodebin_, GST_STATE_PLAYING);

  url_ = next_url_;
  end_offset_nanosec_ = next_end_offset_nanosec_;
  next_url_ = QUrl();
  next_beginning_offset_nanosec_ = 0;
  next_end_offset_nanosec_ = 0;

  // This function gets called when the source has been drained, even if the
  // song hasn't finished playing yet.  We'll get a new segment when it really
  // does finish, so emit TrackEnded then.
  emit_track_ended_on_segment_start_ = true;

  // This has to happen *after* the gst_element_set_state on the new bin to
  // fix an occasional race condition deadlock.
  sElementDeleter->DeleteElementLater(old_decode_bin);

  if (old_tcpsrc) {
    sElementDeleter->DeleteElementLater(old_tcpsrc);
  }

  ignore_tags_ = false;
}

qint64 GstEnginePipeline::position() const {
  GstFormat fmt = GST_FORMAT_TIME;
  gint64 value = 0;
  gst_element_query_position(pipeline_, &fmt, &value);

  return value;
}

qint64 GstEnginePipeline::length() const {
  GstFormat fmt = GST_FORMAT_TIME;
  gint64 value = 0;
  gst_element_query_duration(pipeline_,  &fmt, &value);

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
  return QtConcurrent::run(&gst_element_set_state, pipeline_, state);
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
  return gst_element_seek_simple(pipeline_, GST_FORMAT_TIME,
                                 GST_SEEK_FLAG_FLUSH, nanosec);
}

void GstEnginePipeline::SetEqualizerEnabled(bool enabled) {
  eq_enabled_ = enabled;
  UpdateEqualizer();
}

void GstEnginePipeline::SetEqualizerParams(int preamp, const QList<int>& band_gains) {
  eq_preamp_ = preamp;
  eq_band_gains_ = band_gains;
  UpdateEqualizer();
}

void GstEnginePipeline::UpdateEqualizer() {
  // Update band gains
  for (int i=0 ; i<kEqBandCount ; ++i) {
    float gain = eq_enabled_ ? eq_band_gains_[i] : 0.0;
    if (gain < 0)
      gain *= 0.24;
    else
      gain *= 0.12;

    GstObject* band = gst_child_proxy_get_child_by_index(GST_CHILD_PROXY(equalizer_), i);
    g_object_set(G_OBJECT(band), "gain", gain, NULL);
    g_object_unref(G_OBJECT(band));
  }

  // Update preamp
  float preamp = 1.0;
  if (eq_enabled_)
    preamp = float(eq_preamp_ + 100) * 0.01;  // To scale from 0.0 to 2.0

  g_object_set(G_OBJECT(equalizer_preamp_), "volume", preamp, NULL);
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
  g_object_set(G_OBJECT(volume_), "volume", vol, NULL);
}

void GstEnginePipeline::StartFader(qint64 duration_nanosec,
                                   QTimeLine::Direction direction,
                                   QTimeLine::CurveShape shape) {
  const int duration_msec = duration_nanosec / kNsecPerMsec;

  // If there's already another fader running then start from the same time
  // that one was already at.
  int start_time = direction == QTimeLine::Forward ? 0 : duration_msec;
  if (fader_ && fader_->state() == QTimeLine::Running)
    start_time = fader_->currentTime();

  fader_.reset(new QTimeLine(duration_msec, this));
  connect(fader_.get(), SIGNAL(valueChanged(qreal)), SLOT(SetVolumeModifier(qreal)));
  connect(fader_.get(), SIGNAL(finished()), SLOT(FaderTimelineFinished()));
  fader_->setDirection(direction);
  fader_->setCurveShape(shape);
  fader_->setCurrentTime(start_time);
  fader_->resume();

  fader_fudge_timer_.stop();

  SetVolumeModifier(fader_->currentValue());
}

void GstEnginePipeline::FaderTimelineFinished() {
  fader_.reset();

  // Wait a little while longer before emitting the finished signal (and
  // probably distroying the pipeline) to account for delays in the audio
  // server/driver.
  fader_fudge_timer_.start(kFaderFudgeMsec, this);
}

void GstEnginePipeline::timerEvent(QTimerEvent* e) {
  if (e->timerId() == fader_fudge_timer_.timerId()) {
    fader_fudge_timer_.stop();
    emit FaderFinished();
    return;
  }

  QObject::timerEvent(e);
}

void GstEnginePipeline::AddBufferConsumer(BufferConsumer *consumer) {
  QMutexLocker l(&buffer_consumers_mutex_);
  buffer_consumers_ << consumer;
}

void GstEnginePipeline::RemoveBufferConsumer(BufferConsumer *consumer) {
  QMutexLocker l(&buffer_consumers_mutex_);
  buffer_consumers_.removeAll(consumer);
}

void GstEnginePipeline::RemoveAllBufferConsumers() {
  QMutexLocker l(&buffer_consumers_mutex_);
  buffer_consumers_.clear();
}

void GstEnginePipeline::SetNextUrl(const QUrl& url,
                                   qint64 beginning_nanosec,
                                   qint64 end_nanosec) {
  next_url_ = url;
  next_beginning_offset_nanosec_ = beginning_nanosec;
  next_end_offset_nanosec_ = end_nanosec;
}
