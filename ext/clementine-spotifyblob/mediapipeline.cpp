/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// Note: this file is licensed under the Apache License instead of GPL because
// it is used by the Spotify blob which links against libspotify and is not GPL
// compatible.

#include "mediapipeline.h"
#include "core/logging.h"
#include "core/timeconstants.h"

#include <cstring>

MediaPipeline::MediaPipeline(int port, quint64 length_msec)
    : port_(port),
      length_msec_(length_msec),
      accepting_data_(true),
      pipeline_(nullptr),
      appsrc_(nullptr),
      byte_rate_(1),
      offset_bytes_(0) {}

MediaPipeline::~MediaPipeline() {
  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline_));
  }
}

bool MediaPipeline::Init(int sample_rate, int channels) {
  if (is_initialised()) return false;

  pipeline_ = gst_pipeline_new("pipeline");

  // Create elements
  appsrc_ = GST_APP_SRC(gst_element_factory_make("appsrc", nullptr));
  GstElement* gdppay = gst_element_factory_make("gdppay", nullptr);
  tcpsink_ = gst_element_factory_make("tcpclientsink", nullptr);

  if (!pipeline_ || !appsrc_ || !tcpsink_) {
    if (pipeline_) {
      gst_object_unref(GST_OBJECT(pipeline_));
      pipeline_ = nullptr;
    }
    if (appsrc_) {
      gst_object_unref(GST_OBJECT(appsrc_));
      appsrc_ = nullptr;
    }
    if (gdppay) {
      gst_object_unref(GST_OBJECT(gdppay));
    }
    if (tcpsink_) {
      gst_object_unref(GST_OBJECT(tcpsink_));
      tcpsink_ = nullptr;
    }
    return false;
  }

  // Add elements to the pipeline and link them
  gst_bin_add(GST_BIN(pipeline_), GST_ELEMENT(appsrc_));
  gst_bin_add(GST_BIN(pipeline_), gdppay);
  gst_bin_add(GST_BIN(pipeline_), tcpsink_);
  gst_element_link_many(GST_ELEMENT(appsrc_), gdppay, tcpsink_, nullptr);

  // Set the sink's port
  g_object_set(G_OBJECT(tcpsink_), "host", "127.0.0.1", nullptr);
  g_object_set(G_OBJECT(tcpsink_), "port", port_, nullptr);

  // Try to send 5 seconds of audio in advance to initially fill Clementine's
  // buffer.
  // Commented for now as otherwise the seek will take too long.
  //g_object_set(G_OBJECT(tcpsink_), "ts-offset", qint64(-5 * kNsecPerSec),
  //             nullptr);

  // We know the time of each buffer
  g_object_set(G_OBJECT(appsrc_), "format", GST_FORMAT_TIME, nullptr);

  // Spotify only pushes data to us every 100ms, so keep the appsrc half full
  // to prevent tiny stalls.
  g_object_set(G_OBJECT(appsrc_), "min-percent", 50, nullptr);

  // Set callbacks for when to start/stop pushing data
  GstAppSrcCallbacks callbacks;
  callbacks.enough_data = EnoughDataCallback;
  callbacks.need_data = NeedDataCallback;
  callbacks.seek_data = SeekDataCallback;

  gst_app_src_set_callbacks(appsrc_, &callbacks, this, nullptr);

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
  const int endianness = G_BIG_ENDIAN;
#elif Q_BYTE_ORDER == Q_LITTLE_ENDIAN
  const int endianness = G_LITTLE_ENDIAN;
#endif

  // Set caps
  GstCaps* caps = gst_caps_new_simple(
      "audio/x-raw-int", "endianness", G_TYPE_INT, endianness, "signed",
      G_TYPE_BOOLEAN, TRUE, "width", G_TYPE_INT, 16, "depth", G_TYPE_INT, 16,
      "rate", G_TYPE_INT, sample_rate, "channels", G_TYPE_INT, channels,
      nullptr);

  gst_app_src_set_caps(appsrc_, caps);
  gst_caps_unref(caps);

  // Set size
  byte_rate_ = quint64(sample_rate) * channels * 2;
  const quint64 bytes = byte_rate_ * length_msec_ / 1000;
  gst_app_src_set_size(appsrc_, bytes);

  // Ready to go
  return gst_element_set_state(pipeline_, GST_STATE_PLAYING) !=
         GST_STATE_CHANGE_FAILURE;
}

void MediaPipeline::WriteData(const char* data, qint64 length) {
  if (!is_initialised()) return;

  GstBuffer* buffer = gst_buffer_new_and_alloc(length);

  memcpy(GST_BUFFER_DATA(buffer), data, length);

  GST_BUFFER_OFFSET(buffer) = offset_bytes_;
  GST_BUFFER_TIMESTAMP(buffer) = offset_bytes_ * kNsecPerSec / byte_rate_;
  GST_BUFFER_DURATION(buffer) = length * kNsecPerSec / byte_rate_;

  offset_bytes_ += length;
  GST_BUFFER_OFFSET_END(buffer) = offset_bytes_;

  gst_app_src_push_buffer(appsrc_, buffer);
}

void MediaPipeline::EndStream() {
  if (!is_initialised()) return;

  gst_app_src_end_of_stream(appsrc_);
}

void MediaPipeline::NeedDataCallback(GstAppSrc* src, guint length, void* data) {
  MediaPipeline* me = reinterpret_cast<MediaPipeline*>(data);
  me->accepting_data_ = true;
}

void MediaPipeline::EnoughDataCallback(GstAppSrc* src, void* data) {
  MediaPipeline* me = reinterpret_cast<MediaPipeline*>(data);
  me->accepting_data_ = false;
}

gboolean MediaPipeline::SeekDataCallback(GstAppSrc* src, guint64 offset,
                                         void* data) {
  // MediaPipeline* me = reinterpret_cast<MediaPipeline*>(data);

  qLog(Debug) << "Gstreamer wants seek to" << offset;
  return false;
}
