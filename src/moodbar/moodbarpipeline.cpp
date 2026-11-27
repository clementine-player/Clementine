/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "moodbarpipeline.h"

#include <QCoreApplication>
#include <QMutexLocker>
#include <QThread>
#include <QUrl>

#include "core/logging.h"
#include "core/signalchecker.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "engines/gstdecodeaudioonly.h"
#include "gst/moodbar/gstfastspectrum.h"
#include "moodbar/moodbarbuilder.h"

const int MoodbarPipeline::kBands = 128;

MoodbarPipeline::MoodbarPipeline(const QUrl& local_filename)
    : QObject(nullptr),
      local_filename_(local_filename),
      pipeline_(nullptr),
      convert_element_(nullptr),
      success_(false),
      running_(false) {}

MoodbarPipeline::~MoodbarPipeline() { Cleanup(); }

GstElement* MoodbarPipeline::CreateElement(const QString& factory_name) {
  GstElement* ret =
      gst_element_factory_make(factory_name.toLatin1().constData(), nullptr);

  if (ret) {
    gst_bin_add(GST_BIN(pipeline_), ret);
  } else {
    qLog(Warning) << "Unable to create gstreamer element" << factory_name;
  }

  return ret;
}

void MoodbarPipeline::Start() {
  Q_ASSERT(QThread::currentThread() != qApp->thread());

  Utilities::SetThreadIOPriority(Utilities::IOPRIO_CLASS_IDLE);

  if (pipeline_) {
    return;
  }

  pipeline_ = gst_pipeline_new("moodbar-pipeline");
  if (!pipeline_) {
    emit Finished(false);
    return;
  }

  GstElement* decodebin = CreateElement("uridecodebin");
  convert_element_ = CreateElement("audioconvert");
  GstElement* spectrum = CreateElement("fastspectrum");
  GstElement* fakesink = CreateElement("fakesink");

  if (!decodebin || !convert_element_ || !spectrum || !fakesink) {
    // CreateElement() already added partial elements to pipeline_; unref it.
    gst_object_unref(pipeline_);
    pipeline_ = nullptr;
    emit Finished(false);
    return;
  }

  // Join them together
  if (!gst_element_link(convert_element_, spectrum) ||
      !gst_element_link(spectrum, fakesink)) {
    qLog(Error) << "Failed to link elements";
    gst_object_unref(pipeline_);
    pipeline_ = nullptr;
    emit Finished(false);
    return;
  }

  {
    // Publish builder_ and running_ under mutex_ before starting the pipeline
    // so GStreamer streaming callbacks cannot observe half-initialized state.
    QMutexLocker l(&mutex_);
    builder_.reset(new MoodbarBuilder);
    running_ = true;
  }

  // Set properties
  QByteArray uri = Utilities::GetUriForGstreamer(local_filename_);
  GstDecodeAudioOnly(decodebin);
  g_object_set(decodebin, "uri", uri.constData(), nullptr);
  g_object_set(spectrum, "bands", kBands, nullptr);

  GstFastSpectrum* fast_spectrum = GST_FASTSPECTRUM(spectrum);
  fast_spectrum->output_callback = [this](double* magnitudes, int size) {
    QMutexLocker l(&mutex_);
    if (running_ && builder_) {
      builder_->AddFrame(magnitudes, size);
    }
  };

  // Connect signals
  CHECKED_GCONNECT(decodebin, "pad-added", &NewPadCallback, this);
  GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
  gst_bus_set_sync_handler(bus, BusCallbackSync, this, nullptr);
  gst_object_unref(bus);

  // Start playing
  gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}

void MoodbarPipeline::ReportError(GstMessage* msg) {
  // Initialize out-pointers and use g_free() to match GLib's allocator.
  GError* error = nullptr;
  gchar* debugs = nullptr;

  gst_message_parse_error(msg, &error, &debugs);
  QString message = error ? QString::fromLocal8Bit(error->message) : QString();

  g_clear_error(&error);
  g_free(debugs);

  qLog(Error) << "Error processing" << local_filename_ << ":" << message;
}

void MoodbarPipeline::NewPadCallback(GstElement*, GstPad* pad, gpointer data) {
  MoodbarPipeline* self = reinterpret_cast<MoodbarPipeline*>(data);

  // Ignore non-audio pads (e.g., embedded cover art) so they do not unlink
  // the active audio stream.
  GstCaps* caps = gst_pad_get_current_caps(pad);
  if (!caps) caps = gst_pad_query_caps(pad, nullptr);
  bool is_audio = false;
  int rate = 0;
  if (caps && gst_caps_get_size(caps) > 0) {
    GstStructure* structure = gst_caps_get_structure(caps, 0);
    if (structure) {
      const gchar* name = gst_structure_get_name(structure);
      is_audio = name && g_str_has_prefix(name, "audio/");
      if (is_audio) {
        gst_structure_get_int(structure, "rate", &rate);
      }
    }
  }
  if (caps) gst_caps_unref(caps);
  if (!is_audio) return;

  {
    QMutexLocker l(&self->mutex_);
    if (!self->running_) {
      qLog(Warning)
          << "Received gstreamer callback after pipeline has stopped.";
      return;
    }
  }

  GstPad* const audiopad =
      gst_element_get_static_pad(self->convert_element_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Warning) << "audiopad is already linked, unlinking old pad";
    // gst_pad_unlink() takes (src, sink): upstream peer first, then audiopad.
    GstPad* peer = GST_PAD_PEER(audiopad);
    if (peer) gst_pad_unlink(peer, audiopad);
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);

  QMutexLocker l(&self->mutex_);
  if (self->running_ && self->builder_ != nullptr) {
    self->builder_->Init(kBands, rate);
  } else {
    qLog(Error) << "Builder does not exist";
  }
}

GstBusSyncReply MoodbarPipeline::BusCallbackSync(GstBus*, GstMessage* msg,
                                                 gpointer data) {
  MoodbarPipeline* self = reinterpret_cast<MoodbarPipeline*>(data);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      self->Stop(true);
      break;

    case GST_MESSAGE_ERROR:
      self->ReportError(msg);
      self->Stop(false);
      break;

    default:
      break;
  }
  return GST_BUS_PASS;
}

void MoodbarPipeline::Stop(bool success) {
  QByteArray result;
  {
    QMutexLocker l(&mutex_);
    // Guard against duplicate Stop() calls (e.g. ERROR + EOS or Cleanup()).
    if (!running_) return;
    running_ = false;
    if (builder_ != nullptr) {
      result = builder_->Finish(1000);
      builder_.reset();
    }
  }
  data_ = result;
  // Treat empty output (e.g. no audio frames) as failure so nothing caches
  // an empty moodbar.
  success_ = success && !data_.isEmpty();

  emit Finished(success_);
}

void MoodbarPipeline::Cleanup() {
  Q_ASSERT(QThread::currentThread() == thread());
  Q_ASSERT(QThread::currentThread() != qApp->thread());

  {
    // Stop callbacks before releasing mutex_ so gst_element_set_state() can
    // join streaming threads without deadlocking on mutex_.
    QMutexLocker l(&mutex_);
    running_ = false;
  }
  if (pipeline_) {
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
    gst_object_unref(bus);

    // Join streaming threads in GST_STATE_NULL before resetting builder_.
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(pipeline_);
    pipeline_ = nullptr;
  }
  QMutexLocker l(&mutex_);
  builder_.reset();
}
