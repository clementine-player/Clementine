#include "streampipeline.h"

#include <gst/app/gstappsink.h>

#include "core/logging.h"

const char* StreamPipeline::kPipeline =
    "audioconvert ! vorbisenc ! oggmux";

StreamPipeline::StreamPipeline(QObject* parent)
  : QIODevice(parent),
    pipeline_(NULL) {
}

StreamPipeline::~StreamPipeline() {
  gst_object_unref(pipeline_);
}

void StreamPipeline::Init(const Song& song) {
  pipeline_ = gst_pipeline_new("pipeline");
  GstElement* uridecodebin = CreateElement("uridecodebin", pipeline_);
  qLog(Debug) << "Streaming:" << song.url();
  g_object_set(
      G_OBJECT(uridecodebin), "uri", song.url().toString().toUtf8().constData(), NULL);
  g_signal_connect(
      G_OBJECT(uridecodebin), "pad-added", G_CALLBACK(NewPadCallback), this);

  GError* error = NULL;
  convert_bin_ = gst_parse_bin_from_description(kPipeline, TRUE, &error);
  gst_bin_add(GST_BIN(pipeline_), convert_bin_);

  gst_element_set_state(uridecodebin, GST_STATE_PLAYING);

  app_sink_ = CreateElement("appsink", pipeline_);
  g_object_set(G_OBJECT(app_sink_), "emit-signals", TRUE, NULL);
  g_signal_connect(
      G_OBJECT(app_sink_), "new-buffer", G_CALLBACK(NewBufferCallback), this);

  qLog(Debug) << "Linking appsink:" << gst_element_link(convert_bin_, app_sink_);

  gst_bus_set_sync_handler(
      gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), BusCallbackSync, this);
  bus_cb_id_ = gst_bus_add_watch(
      gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), BusCallback, this);
}

qint64 StreamPipeline::readData(char* data, qint64 max_size) {
  QByteArray ret = buffer_.left(max_size);
  buffer_ = buffer_.mid(ret.size());
  memcpy(data, ret.constData(), ret.size());
  return ret.size();
}

qint64 StreamPipeline::writeData(const char* data, qint64 max_size) {
  return -1;
}

GstElement* StreamPipeline::CreateElement(
    const QString& factory_name, GstElement* bin) {
  GstElement* ret = gst_element_factory_make(
      factory_name.toAscii().constData(),
      factory_name.toAscii().constData());
  qLog(Debug) << "Created element:" << factory_name << ret;
  if (ret && bin) {
    gst_bin_add(GST_BIN(bin), ret);
  }
  return ret;
}

GstBusSyncReply StreamPipeline::BusCallbackSync(
    GstBus*, GstMessage* msg, gpointer self) {
  reinterpret_cast<StreamPipeline*>(self)->HandleMessage(msg);
  return GST_BUS_PASS;
}

gboolean StreamPipeline::BusCallback(GstBus*, GstMessage* msg, gpointer self) {
  reinterpret_cast<StreamPipeline*>(self)->HandleMessage(msg);
  return FALSE;
}

void StreamPipeline::HandleMessage(GstMessage* msg) {
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      qLog(Debug) << "Finished streaming";
      break;
    case GST_MESSAGE_ERROR: {
      GError* error;
      gchar* debugs;
      gst_message_parse_error(msg, &error, &debugs);
      QString message = QString::fromLocal8Bit(error->message);
      g_error_free(error);
      free(debugs);
      qLog(Debug) << message;
      break;
    }
    default:
      break;
  }
}

void StreamPipeline::NewBufferCallback(GstElement* app_sink, gpointer self) {
  StreamPipeline* me = reinterpret_cast<StreamPipeline*>(self);

  GstBuffer* buffer = gst_app_sink_pull_buffer((GstAppSink*)app_sink);
  me->buffer_.append((const char*)buffer->data, buffer->size);
  gst_buffer_unref(buffer);

  emit me->readyRead();
}

void StreamPipeline::NewPadCallback(
    GstElement* decodebin, GstPad* pad, gpointer self) {
  qLog(Debug) << "Linking pads";
  StreamPipeline* me = reinterpret_cast<StreamPipeline*>(self);
  GstPad* const audiopad = gst_element_get_pad(me->convert_bin_, "sink");

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);

  QMetaObject::invokeMethod(me, "StartPipeline", Qt::QueuedConnection);
}

void StreamPipeline::StartPipeline() {
  gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}
