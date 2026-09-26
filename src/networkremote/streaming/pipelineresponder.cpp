/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "pipelineresponder.h"

#include <gst/app/gstappsink.h>

#include <QCoreApplication>
#include <QTcpSocket>
#include <QTimer>

#include "core/logging.h"
#include "core/timeconstants.h"
#include "mediahttpserver.h"

namespace {

// Keep this much queued in the socket; the appsink's own queue then fills and
// blocks the pipeline, so a file-backed pipeline stays a bounded distance
// ahead of the renderer.
const qint64 kHighWater = 512 * 1024;
const int kMaxQueuedBuffers = 200;
const int kPumpIntervalMsec = 20;

}  // namespace

QString PipelineResponder::Description(const StreamPlan& plan) {
  return QString(
             "uridecodebin name=decode caps=\"%1\" ! %2 ! "
             "appsink name=sink sync=false max-buffers=%3")
      .arg(plan.decode_caps, plan.tail)
      .arg(kMaxQueuedBuffers);
}

PipelineResponder::PipelineResponder(QTcpSocket* socket, const StreamItem& item,
                                     qint64 start_ms, bool head_only)
    : socket_(socket),
      item_(item),
      start_nanosec_(item.plan.beginning_nanosec + start_ms * kNsecPerMsec),
      stop_nanosec_(-1),
      pipeline_(nullptr),
      appsink_(nullptr),
      timer_(new QTimer(this)),
      waiting_to_seek_(false),
      head_written_(false),
      finished_(false) {
  socket_->setParent(this);
  connect(socket_, SIGNAL(disconnected()), SLOT(deleteLater()));

  if (head_only) {
    WriteHead();
    socket_->disconnectFromHost();
    return;
  }

  // Tracks cut from a larger file (cue sheets) stop at their end marker.
  if (item.song.has_cue() && item.plan.end_nanosec > 0) {
    stop_nanosec_ = item.plan.end_nanosec;
  }

  const QString description = Description(item.plan);
  qLog(Debug) << "Stream pipeline:" << description;

  GError* error = nullptr;
  pipeline_ = gst_parse_launch(description.toUtf8().constData(), &error);
  if (error) {
    const QString message = QString::fromUtf8(error->message);
    g_error_free(error);
    if (pipeline_) {
      gst_object_unref(pipeline_);
      pipeline_ = nullptr;
    }
    Fail("Couldn't build the pipeline: " + message);
    return;
  }

  GstElement* decode = gst_bin_get_by_name(GST_BIN(pipeline_), "decode");
  g_object_set(decode, "uri", item.req.MediaUrl().toEncoded().constData(),
               nullptr);
  g_signal_connect(decode, "source-setup", G_CALLBACK(SourceSetupCallback),
                   this);
  gst_object_unref(decode);

  appsink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "sink");

  // Seeking needs a prerolled pipeline, so start paused when we need one.
  waiting_to_seek_ = start_nanosec_ > 0 || stop_nanosec_ > 0;
  gst_element_set_state(
      pipeline_, waiting_to_seek_ ? GST_STATE_PAUSED : GST_STATE_PLAYING);

  connect(timer_, SIGNAL(timeout()), SLOT(Pump()));
  connect(socket_, SIGNAL(bytesWritten(qint64)), SLOT(Pump()));
  timer_->start(kPumpIntervalMsec);
}

PipelineResponder::~PipelineResponder() {
  if (pipeline_) {
    // Also unblocks an appsink that's waiting for room in its queue.
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(pipeline_);
  }
  if (appsink_) gst_object_unref(appsink_);
}

void PipelineResponder::SourceSetupCallback(GstElement*, GstElement* source,
                                            gpointer self) {
  // Same as GstEnginePipeline::SourceSetupCallback: the renderer fetches
  // from us, and we fetch from upstream with Clementine's credentials.
  PipelineResponder* instance = reinterpret_cast<PipelineResponder*>(self);
  GObjectClass* klass = G_OBJECT_GET_CLASS(source);

  if (g_object_class_find_property(klass, "user-agent")) {
    const QString user_agent =
        QString("%1 %2").arg(QCoreApplication::applicationName(),
                             QCoreApplication::applicationVersion());
    g_object_set(source, "user-agent", user_agent.toUtf8().constData(),
                 nullptr);
  }

  const MediaPlaybackRequest::HeaderList& headers =
      instance->item_.req.headers_;
  if (!headers.isEmpty() &&
      g_object_class_find_property(klass, "extra-headers")) {
    GstStructure* gheaders = gst_structure_new_empty("headers");
    for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
      gst_structure_set(gheaders, it.key().constData(), G_TYPE_STRING,
                        it.value().constData(), nullptr);
    }
    g_object_set(source, "extra-headers", gheaders, nullptr);
    gst_structure_free(gheaders);
  }
}

bool PipelineResponder::HandleBusMessages() {
  GstBus* bus = gst_element_get_bus(pipeline_);
  bool ok = true;

  while (GstMessage* msg = gst_bus_pop(bus)) {
    switch (GST_MESSAGE_TYPE(msg)) {
      case GST_MESSAGE_ERROR: {
        GError* error = nullptr;
        gst_message_parse_error(msg, &error, nullptr);
        Fail(QString::fromUtf8(error->message));
        g_error_free(error);
        ok = false;
        break;
      }
      case GST_MESSAGE_ASYNC_DONE:
        if (waiting_to_seek_) StartSeek();
        break;
      default:
        break;
    }
    gst_message_unref(msg);
    if (!ok) break;
  }

  gst_object_unref(bus);
  return ok;
}

void PipelineResponder::StartSeek() {
  waiting_to_seek_ = false;
  const GstSeekFlags flags =
      GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE);
  if (!gst_element_seek(
          pipeline_, 1.0, GST_FORMAT_TIME, flags, GST_SEEK_TYPE_SET,
          start_nanosec_,
          stop_nanosec_ > 0 ? GST_SEEK_TYPE_SET : GST_SEEK_TYPE_NONE,
          stop_nanosec_ > 0 ? stop_nanosec_ : 0)) {
    qLog(Warning) << "Stream pipeline couldn't seek to" << start_nanosec_;
  }
  gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}

void PipelineResponder::WriteHead() {
  if (head_written_) return;
  head_written_ = true;

  QByteArray head = "HTTP/1.1 200 OK\r\n";
  head += "Content-Type: " + item_.plan.mime_type.toLatin1() + "\r\n";
  // The output's size isn't known until it's finished.
  head += "Transfer-Encoding: chunked\r\n";
  head += "Accept-Ranges: none\r\n";
  head += "Cache-Control: no-store\r\n";
  head += "Connection: close\r\n\r\n";
  socket_->write(head);
}

void PipelineResponder::Pump() {
  if (finished_ || !pipeline_) return;
  if (!HandleBusMessages()) return;

  while (socket_->bytesToWrite() < kHighWater) {
    GstSample* sample = gst_app_sink_try_pull_sample(GST_APP_SINK(appsink_), 0);
    if (!sample) {
      if (gst_app_sink_is_eos(GST_APP_SINK(appsink_))) Finish();
      return;
    }

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;
    if (buffer && gst_buffer_map(buffer, &map, GST_MAP_READ)) {
      if (map.size > 0) {
        WriteHead();
        socket_->write(QByteArray::number(qulonglong(map.size), 16) + "\r\n");
        socket_->write(reinterpret_cast<const char*>(map.data), map.size);
        socket_->write("\r\n");
      }
      gst_buffer_unmap(buffer, &map);
    }
    gst_sample_unref(sample);
  }
}

void PipelineResponder::Fail(const QString& message) {
  qLog(Warning) << "Stream pipeline for" << item_.req.MediaUrl()
                << "failed:" << message;
  finished_ = true;
  timer_->stop();

  if (head_written_) {
    // Too late for an error status; ending without the final chunk tells
    // the renderer the response is incomplete.
    socket_->abort();
  } else {
    MediaHttpServer::WriteError(socket_, 502, "Bad Gateway");
  }
}

void PipelineResponder::Finish() {
  finished_ = true;
  timer_->stop();
  WriteHead();
  socket_->write("0\r\n\r\n");
  socket_->disconnectFromHost();
}
