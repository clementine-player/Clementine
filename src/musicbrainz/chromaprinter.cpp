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

static const int kDecodeRate = 11025;
static const int kDecodeChannels = 1;
static const int kPlayLengthSecs = 30;
static const int kTimeoutSecs = 10;

Chromaprinter::Chromaprinter(const QString& filename)
    : filename_(filename), convert_element_(nullptr) {}

Chromaprinter::~Chromaprinter() {}

GstElement* Chromaprinter::CreateElement(const QString& factory_name,
                                         GstElement* bin) {
  GstElement* ret = gst_element_factory_make(
      factory_name.toAscii().constData(), factory_name.toAscii().constData());

  if (ret && bin) gst_bin_add(GST_BIN(bin), ret);

  if (!ret) {
    qLog(Warning) << "Couldn't create the gstreamer element" << factory_name;
  }

  return ret;
}

QString Chromaprinter::CreateFingerprint() {
  Q_ASSERT(QThread::currentThread() != qApp->thread());

  buffer_.open(QIODevice::WriteOnly);

  GstElement* pipeline = gst_pipeline_new("pipeline");
  GstElement* src = CreateElement("filesrc", pipeline);
  GstElement* decode = CreateElement("decodebin", pipeline);
  GstElement* convert = CreateElement("audioconvert", pipeline);
  GstElement* resample = CreateElement("audioresample", pipeline);
  GstElement* sink = CreateElement("appsink", pipeline);

  if (!src || !decode || !convert || !resample || !sink) {
    return QString();
  }

  convert_element_ = convert;

  // Connect the elements
  gst_element_link_many(src, decode, nullptr);
  gst_element_link_many(convert, resample, nullptr);

  // Chromaprint expects mono 16-bit ints at a sample rate of 11025Hz.
  GstCaps* caps = gst_caps_new_simple(
      "audio/x-raw", "format", G_TYPE_STRING, "S16LE", "channels", G_TYPE_INT,
      kDecodeChannels, "rate", G_TYPE_INT, kDecodeRate, NULL);
  gst_element_link_filtered(resample, sink, caps);
  gst_caps_unref(caps);

  GstAppSinkCallbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.new_sample = NewBufferCallback;
  gst_app_sink_set_callbacks(reinterpret_cast<GstAppSink*>(sink), &callbacks,
                             this, nullptr);
  g_object_set(G_OBJECT(sink), "sync", FALSE, nullptr);
  g_object_set(G_OBJECT(sink), "emit-signals", TRUE, nullptr);

  // Set the filename
  g_object_set(src, "location", filename_.toUtf8().constData(), nullptr);

  // Connect signals
  GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  CHECKED_GCONNECT(decode, "pad-added", &NewPadCallback, this);

  // Play only first x seconds
  gst_element_set_state(pipeline, GST_STATE_PAUSED);
  // wait for state change before seeking
  gst_element_get_state(pipeline, nullptr, nullptr, kTimeoutSecs * GST_SECOND);
  gst_element_seek(pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                   GST_SEEK_TYPE_SET, 0 * GST_SECOND, GST_SEEK_TYPE_SET,
                   kPlayLengthSecs * GST_SECOND);

  QTime time;
  time.start();

  // Start playing
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  // Wait until EOS or error
  GstMessage* msg = gst_bus_timed_pop_filtered(
      bus, kTimeoutSecs * GST_SECOND,
      static_cast<GstMessageType>(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));

  if (msg != nullptr) {
    if (msg->type == GST_MESSAGE_ERROR) {
      // Report error
      GError* error = nullptr;
      gchar* debugs = nullptr;

      gst_message_parse_error(msg, &error, &debugs);
      QString message = QString::fromLocal8Bit(error->message);

      g_error_free(error);
      free(debugs);

      qLog(Debug) << "Error processing" << filename_ << ":" << message;
    }
    gst_message_unref(msg);
  }

  int decode_time = time.restart();

  buffer_.close();

  // Generate fingerprint from recorded buffer data
  QByteArray data = buffer_.data();

  ChromaprintContext* chromaprint =
      chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
  chromaprint_start(chromaprint, kDecodeRate, kDecodeChannels);
  chromaprint_feed(chromaprint, reinterpret_cast<int16_t *>(data.data()),
                   data.size() / 2);
  chromaprint_finish(chromaprint);

  int size = 0;

#if CHROMAPRINT_VERSION_MAJOR >= 1 && CHROMAPRINT_VERSION_MINOR >= 4
  u_int32_t *fprint = nullptr;
  char *encoded = nullptr;
#else
  void *fprint = nullptr;
  void *encoded = nullptr;
#endif

  int ret = chromaprint_get_raw_fingerprint(chromaprint, &fprint, &size);

  QByteArray fingerprint;
  if (ret == 1) {
    int encoded_size = 0;
    chromaprint_encode_fingerprint(fprint, size, CHROMAPRINT_ALGORITHM_DEFAULT,
                                   &encoded, &encoded_size, 1);

    fingerprint.append(reinterpret_cast<char*>(encoded), encoded_size);

    chromaprint_dealloc(fprint);
    chromaprint_dealloc(encoded);
  }
  chromaprint_free(chromaprint);
  int codegen_time = time.elapsed();

  qLog(Debug) << "Decode time:" << decode_time
              << "Codegen time:" << codegen_time;

  // Cleanup
  callbacks.new_sample = nullptr;
  gst_object_unref(bus);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);

  return fingerprint;
}

void Chromaprinter::NewPadCallback(GstElement*, GstPad* pad, gpointer data) {
  Chromaprinter* instance = reinterpret_cast<Chromaprinter*>(data);
  GstPad* const audiopad =
      gst_element_get_static_pad(instance->convert_element_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Warning) << "audiopad is already linked, unlinking old pad";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);
}

GstFlowReturn Chromaprinter::NewBufferCallback(GstAppSink* app_sink,
                                               gpointer self) {
  Chromaprinter* me = reinterpret_cast<Chromaprinter*>(self);

  GstSample* sample = gst_app_sink_pull_sample(app_sink);
  GstBuffer* buffer = gst_sample_get_buffer(sample);
  GstMapInfo map;
  gst_buffer_map(buffer, &map, GST_MAP_READ);
  me->buffer_.write(reinterpret_cast<const char*>(map.data), map.size);
  gst_buffer_unmap(buffer, &map);
  gst_buffer_unref(buffer);

  return GST_FLOW_OK;
}
