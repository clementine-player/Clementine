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

#include "echoprinter.h"
#include "core/logging.h"
#include "core/timeconstants.h"

#include "3rdparty/echoprint-codegen/src/Codegen.h"

#include <QDir>
#include <QEventLoop>
#include <QtDebug>
#include <QTime>

Echoprinter::Echoprinter(const QString& filename)
  : filename_(filename),
    event_loop_(NULL),
    convert_element_(NULL),
    finishing_(false)
{
  buffer_.open(QIODevice::WriteOnly);
}

Echoprinter::~Echoprinter() {
}

GstElement* Echoprinter::CreateElement(const QString &factory_name,
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

QString Echoprinter::CreateFingerprint() {
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

  // Echoprint expects mono floats at a sample rate of 11025Hz.
  GstCaps* caps = gst_caps_new_simple(
      "audio/x-raw-float",
      "width", G_TYPE_INT, 32,
      "channels", G_TYPE_INT, 1,
      "rate", G_TYPE_INT, 11025,
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
  g_object_set(src, "location", filename_.toLocal8Bit().constData(), NULL);

  // Connect signals
  g_signal_connect(decode, "new-decoded-pad", G_CALLBACK(NewPadCallback), this);
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
  Codegen codegen(reinterpret_cast<const float*>(data.constData()),
                  data.size() / sizeof(float), 0);
  QString fingerprint = QString::fromAscii(codegen.getCodeString().c_str());
  int codegen_time = time.elapsed();

  qLog(Debug) << "Decode time:" << decode_time << "Codegen time:" << codegen_time;

  qLog(Debug) << "Echoprint:" << fingerprint;

  // Cleanup
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), NULL, NULL);
  g_source_remove(bus_callback_id);
  gst_object_unref(pipeline_);

  return fingerprint;
}

void Echoprinter::NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer data) {
  Echoprinter* instance = reinterpret_cast<Echoprinter*>(data);
  GstPad* const audiopad = gst_element_get_pad(instance->convert_element_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Warning) << "audiopad is already linked, unlinking old pad";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);
}

void Echoprinter::ReportError(GstMessage* msg) {
  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  QString message = QString::fromLocal8Bit(error->message);

  g_error_free(error);
  free(debugs);

  qLog(Error) << "Error processing" << filename_ << ":" << message;
}

gboolean Echoprinter::BusCallback(GstBus*, GstMessage* msg, gpointer data) {
  Echoprinter* instance = reinterpret_cast<Echoprinter*>(data);

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

GstBusSyncReply Echoprinter::BusCallbackSync(GstBus*, GstMessage* msg, gpointer data) {
  Echoprinter* instance = reinterpret_cast<Echoprinter*>(data);

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

GstFlowReturn Echoprinter::NewBufferCallback(GstAppSink* app_sink, gpointer self) {
  Echoprinter* me = reinterpret_cast<Echoprinter*>(self);
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
