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

#ifndef MEDIAPIPELINE_H
#define MEDIAPIPELINE_H

#include <QtGlobal>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

class MediaPipeline {
 public:
  MediaPipeline(int port, quint64 length_msec);
  ~MediaPipeline();

  bool is_initialised() const { return pipeline_; }
  bool is_accepting_data() const { return accepting_data_; }
  bool Init(int sample_rate, int channels);

  void WriteData(const char* data, qint64 length);
  void EndStream();

 private:
  static void NeedDataCallback(GstAppSrc* src, guint length, void* data);
  static void EnoughDataCallback(GstAppSrc* src, void* data);
  static gboolean SeekDataCallback(GstAppSrc* src, guint64 offset, void* data);

 private:
  Q_DISABLE_COPY(MediaPipeline)

  const int port_;
  const quint64 length_msec_;

  bool accepting_data_;

  GstElement* pipeline_;
  GstAppSrc* appsrc_;
  GstElement* tcpsink_;

  quint64 byte_rate_;
  quint64 offset_bytes_;
};

#endif  // MEDIAPIPELINE_H
