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
#include <QThread>
#include <QUrl>

#include "core/logging.h"
#include "core/signalchecker.h"
#include "core/utilities.h"

bool MoodbarPipeline::sIsAvailable = false;

MoodbarPipeline::MoodbarPipeline(const QUrl& local_filename)
  : QObject(nullptr),
    local_filename_(local_filename),
    pipeline_(nullptr),
    convert_element_(nullptr),
    success_(false)
{
}

MoodbarPipeline::~MoodbarPipeline() {
  Cleanup();
}

bool MoodbarPipeline::IsAvailable() {
  if (!sIsAvailable) {
    GstElementFactory* factory = gst_element_factory_find("fftwspectrum");
    if (!factory) {
      return false;
    }
    gst_object_unref(factory);

    factory = gst_element_factory_find("moodbar");
    if (!factory) {
      return false;
    }
    gst_object_unref(factory);

    sIsAvailable = true;
  }

  return sIsAvailable;
}

GstElement* MoodbarPipeline::CreateElement(const QString& factory_name) {
  GstElement* ret = gst_element_factory_make(factory_name.toAscii().constData(), nullptr);

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

  GstElement* decodebin    = CreateElement("uridecodebin");
  convert_element_         = CreateElement("audioconvert");
  GstElement* fftwspectrum = CreateElement("fftwspectrum");
  GstElement* moodbar      = CreateElement("moodbar");
  GstElement* appsink      = CreateElement("appsink");

  if (!decodebin || !convert_element_ || !fftwspectrum || !moodbar || !appsink) {
    pipeline_ = nullptr;
    emit Finished(false);
    return;
  }

  // Join them together
  gst_element_link_many(convert_element_, fftwspectrum, moodbar, appsink, nullptr);

  // Set properties
  g_object_set(decodebin, "uri", local_filename_.toEncoded().constData(), nullptr);
  g_object_set(fftwspectrum, "def-size", 2048,
                             "def-step", 1024,
                             "hiquality", true, nullptr);
  g_object_set(moodbar, "height", 1,
                        "max-width", 1000, nullptr);

  // Connect signals
  CHECKED_GCONNECT(decodebin, "pad-added", &NewPadCallback, this);
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), BusCallbackSync, this);

  // Set appsink callbacks
  GstAppSinkCallbacks callbacks;
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.new_buffer = NewBufferCallback;

  gst_app_sink_set_callbacks(reinterpret_cast<GstAppSink*>(appsink), &callbacks, this, nullptr);

  // Start playing
  gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}

void MoodbarPipeline::ReportError(GstMessage* msg) {
  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  QString message = QString::fromLocal8Bit(error->message);

  g_error_free(error);
  free(debugs);

  qLog(Error) << "Error processing" << local_filename_ << ":" << message;
}

void MoodbarPipeline::NewPadCallback(GstElement*, GstPad* pad, gpointer data) {
  MoodbarPipeline* self = reinterpret_cast<MoodbarPipeline*>(data);
  GstPad* const audiopad = gst_element_get_static_pad(
      self->convert_element_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Warning) << "audiopad is already linked, unlinking old pad";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);
}

GstFlowReturn MoodbarPipeline::NewBufferCallback(GstAppSink* app_sink, gpointer data) {
  MoodbarPipeline* self = reinterpret_cast<MoodbarPipeline*>(data);

  GstBuffer* buffer = gst_app_sink_pull_buffer(app_sink);
  self->data_.append(reinterpret_cast<const char*>(buffer->data), buffer->size);
  gst_buffer_unref(buffer);

  return GST_FLOW_OK;
}

GstBusSyncReply MoodbarPipeline::BusCallbackSync(GstBus*, GstMessage* msg, gpointer data) {
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
  success_ = success;
  emit Finished(success);
}

void MoodbarPipeline::Cleanup() {
  Q_ASSERT(QThread::currentThread() == thread());
  Q_ASSERT(QThread::currentThread() != qApp->thread());

  if (pipeline_) {
    gst_bus_set_sync_handler(
        gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), nullptr,nullptr);
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(pipeline_);
    pipeline_ = nullptr;
  }
}
