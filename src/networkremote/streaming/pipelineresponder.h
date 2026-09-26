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

#ifndef NETWORKREMOTE_STREAMING_PIPELINERESPONDER_H_
#define NETWORKREMOTE_STREAMING_PIPELINERESPONDER_H_

#include <gst/gst.h>

#include <QObject>

#include "streamitemtable.h"

class QTcpSocket;
class QTimer;

// Streams an item through a GStreamer pipeline (remux or encode) as a chunked
// HTTP response. Deletes itself when the socket closes.
//
//   uridecodebin(caps = plan.decode_caps) ! <plan.tail> ! appsink
//
// See docs/design/remote-streaming.md section 5.2.
class PipelineResponder : public QObject {
  Q_OBJECT

 public:
  // Takes ownership of |socket|. |start_ms| is where to start within the
  // item, for seeking.
  PipelineResponder(QTcpSocket* socket, const StreamItem& item, qint64 start_ms,
                    bool head_only);
  ~PipelineResponder();

  // The gst-launch description for |plan|, without the source URI.
  static QString Description(const StreamPlan& plan);

 private slots:
  void Pump();

 private:
  static void SourceSetupCallback(GstElement* decodebin, GstElement* source,
                                  gpointer self);

  bool HandleBusMessages();
  void StartSeek();
  void WriteHead();
  void Fail(const QString& message);
  void Finish();

  QTcpSocket* socket_;
  StreamItem item_;
  qint64 start_nanosec_;
  qint64 stop_nanosec_;  // -1: to the end

  GstElement* pipeline_;
  GstElement* appsink_;
  QTimer* timer_;

  bool waiting_to_seek_;
  bool head_written_;
  bool finished_;
};

#endif  // NETWORKREMOTE_STREAMING_PIPELINERESPONDER_H_
