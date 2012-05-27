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

#ifndef MOODBARPIPELINE_H
#define MOODBARPIPELINE_H

#include <QBuffer>
#include <QMutex>
#include <QObject>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

// Creates moodbar data for a single local music file.
class MoodbarPipeline : public QObject {
  Q_OBJECT
  
public:
  MoodbarPipeline(const QString& local_filename);
  ~MoodbarPipeline();
  
  static bool IsAvailable();
  
  bool success() const { return success_; }
  const QByteArray& data() const { return data_; }

public slots:
  void Start();

signals:
  void Finished(bool success);
  
private:
  GstElement* CreateElement(const QString& factory_name);
  
  void ReportError(GstMessage* message);
  void Stop(bool success);
  void Cleanup();
  
  static void NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer data);
  static GstFlowReturn NewBufferCallback(GstAppSink* app_sink, gpointer self);
  static gboolean BusCallback(GstBus*, GstMessage* msg, gpointer data);
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage* msg, gpointer data);
  
private:
  static bool sIsAvailable;
  static QMutex sFftwMutex;
  
  QString local_filename_;
  GstElement* pipeline_;
  GstElement* convert_element_;
  
  bool success_;
  QByteArray data_;
};

#endif // MOODBARPIPELINE_H
