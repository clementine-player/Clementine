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

#include "chromaprinter.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QThread>
#include <QtDebug>
#include <QTime>

#include <chromaprint.h>

#include "core/logging.h"
#include "core/signalchecker.h"
#include "core/timeconstants.h"

static const int kDecodeRate = 11025;
static const int kDecodeChannels = 1;

Chromaprinter::Chromaprinter(const QString& filename)
  : filename_(filename),
    event_loop_(NULL),
    convert_element_(NULL),
    finishing_(false) {
}

Chromaprinter::~Chromaprinter() {
}

GstElement* Chromaprinter::CreateElement(const QString &factory_name,
                                       GstElement *bin) {
  GstElement* ret = gst_element_factory_make(
      factory_name.toAscii().constData(),
      factory_name.toAscii().constData());

  if (ret && bin)
    gst_bin_add(GST_BIN(bin), ret);

  if (!ret) {
    qLog(Warning) << "Couldn't create the gstreamer element" << factory_name;
  }

  return ret;
}

QString Chromaprinter::CreateFingerprint() {
  Q_ASSERT(QThread::currentThread() != qApp->thread());

  buffer_.open(QIODevice::WriteOnly);

  GMainContext* context = g_main_context_new();
  g_main_context_push_thread_default(context);
  event_loop_ = g_main_loop_new(context, FALSE);

  pipeline_ = gst_pipeline_new("pipeline");
  GstElement* src      = CreateElement("filesrc", pipeline_);
  GstElement* decode   = CreateElement("decodebin2", pipeline_);
  GstElement* convert  = CreateElement("audioconvert", pipeline_);
  GstElement* resample = CreateElement("audioresample", pipeline_);
  GstElement* sink     = CreateElement("appsink", pipeline_);

  if (!src || !decode || !convert || !resample || !sink) {
    return QString();
  }

  convert_element_ = convert;

  // Connect the elements
  gst_element_link_many(src, decode, NULL);
  gst_element_link_many(convert, resample, NULL);

  // Chromaprint expects mono floats at a sample rate of 11025Hz.
  GstCaps* caps = gst_caps_new_simple(
      "audio/x-raw-int",
      "width", G_TYPE_INT, 16,
      "channels", G_TYPE_INT, kDecodeChannels,
      "rate", G_TYPE_INT, kDecodeRate,
      NULL);
  gst_element_link_filtered(resample, sink, caps);
  gst_caps_unref(caps);

  GstAppSinkCallbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.new_buffer = NewBufferCallback;
  gst_app_sink_set_callbacks(reinterpret_cast<GstAppSink*>(sink), &callbacks, this, NULL);
  g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);
  g_object_set(G_OBJECT(sink), "emit-signals", TRUE, NULL);

  // Set the filename
  g_object_set(src, "location", filename_.toUtf8().constData(), NULL);

  // Connect signals
  CHECKED_GCONNECT(decode, "new-decoded-pad", &NewPadCallback, this);
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), BusCallbackSync, this);
  guint bus_callback_id = gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), BusCallback, this);

  QTime time;
  time.start();

  // Start playing
  gst_element_set_state(pipeline_, GST_STATE_PLAYING);

  g_main_loop_run(event_loop_);
  g_main_loop_unref(event_loop_);
  g_main_context_unref(context);

  int decode_time = time.restart();

  buffer_.close();
  QByteArray data = buffer_.data();

  ChromaprintContext* chromaprint = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
  chromaprint_start(chromaprint, kDecodeRate, kDecodeChannels);
  chromaprint_feed(chromaprint, reinterpret_cast<void*>(data.data()), data.size() / 2);
  chromaprint_finish(chromaprint);

  void* fprint = NULL;
  int size = 0;
  int ret = chromaprint_get_raw_fingerprint(chromaprint, &fprint, &size);
  QByteArray fingerprint;
  if (ret == 1) {
    void* encoded = NULL;
    int encoded_size = 0;
    chromaprint_encode_fingerprint(
        fprint, size, CHROMAPRINT_ALGORITHM_DEFAULT, &encoded, &encoded_size, 1);

    fingerprint.append(reinterpret_cast<char*>(encoded), encoded_size);

    chromaprint_dealloc(fprint);
    chromaprint_dealloc(encoded);
  }
  chromaprint_free(chromaprint);
  int codegen_time = time.elapsed();

  qLog(Debug) << "Decode time:" << decode_time << "Codegen time:" << codegen_time;

  // Cleanup
  callbacks.new_buffer = NULL;
  gst_app_sink_set_callbacks(reinterpret_cast<GstAppSink*>(sink), &callbacks, this, NULL);
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), NULL, NULL);
  g_source_remove(bus_callback_id);
  gst_element_set_state(pipeline_, GST_STATE_NULL);
  gst_object_unref(pipeline_);

  return fingerprint;
}

void Chromaprinter::NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer data) {
  Chromaprinter* instance = reinterpret_cast<Chromaprinter*>(data);
  GstPad* const audiopad = gst_element_get_static_pad(
      instance->convert_element_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Warning) << "audiopad is already linked, unlinking old pad";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);
}

void Chromaprinter::ReportError(GstMessage* msg) {
  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  QString message = QString::fromLocal8Bit(error->message);

  g_error_free(error);
  free(debugs);

  qLog(Error) << "Error processing" << filename_ << ":" << message;
}

gboolean Chromaprinter::BusCallback(GstBus*, GstMessage* msg, gpointer data) {
  Chromaprinter* instance = reinterpret_cast<Chromaprinter*>(data);
  if (instance->finishing_) {
    return GST_BUS_DROP;
  }

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR:
      instance->ReportError(msg);
      g_main_loop_quit(instance->event_loop_);
      break;

    default:
      break;
  }
  return GST_BUS_DROP;
}

GstBusSyncReply Chromaprinter::BusCallbackSync(GstBus*, GstMessage* msg, gpointer data) {
  Chromaprinter* instance = reinterpret_cast<Chromaprinter*>(data);
  if (instance->finishing_) {
    return GST_BUS_PASS;
  }

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      g_main_loop_quit(instance->event_loop_);
      break;

    case GST_MESSAGE_ERROR:
      instance->ReportError(msg);
      g_main_loop_quit(instance->event_loop_);
      break;

    default:
      break;
  }
  return GST_BUS_PASS;
}

GstFlowReturn Chromaprinter::NewBufferCallback(GstAppSink* app_sink, gpointer self) {
  Chromaprinter* me = reinterpret_cast<Chromaprinter*>(self);
  if (me->finishing_) {
    return GST_FLOW_OK;
  }

  GstBuffer* buffer = gst_app_sink_pull_buffer(app_sink);
  me->buffer_.write(reinterpret_cast<const char*>(buffer->data), buffer->size);
  gst_buffer_unref(buffer);

  gint64 pos = 0;
  GstFormat format = GST_FORMAT_TIME;
  gboolean ret = gst_element_query_position(me->pipeline_, &format, &pos);
  if (ret && pos > 30 * kNsecPerSec) {
    me->finishing_ = true;
    g_main_loop_quit(me->event_loop_);
  }
  return GST_FLOW_OK;
}
