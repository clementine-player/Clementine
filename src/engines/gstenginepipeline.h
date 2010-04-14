/* This file is part of Clementine.

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

#ifndef GSTENGINEPIPELINE_H
#define GSTENGINEPIPELINE_H

#include <QObject>
#include <QUrl>
#include <QTimeLine>

#include <gst/gst.h>

#include "engine_fwd.h"

class GstEngine;

class GstEnginePipeline : public QObject {
  Q_OBJECT

 public:
  GstEnginePipeline();
  ~GstEnginePipeline();

  // Call these setters before Init
  void set_output_device(const QString& sink, const QString& device);
  void set_forwards_buffers(bool v) { forwards_buffers_ = v; }

  // Creates the pipeline, returns false on error
  bool Init(const QUrl& url);

  // Control the music playback
  bool SetState(GstState state);
  bool Seek(qint64 nanosec);
  void SetEqualizerEnabled(bool enabled);
  void SetEqualizerParams(int preamp, const QList<int>& band_gains);
  void SetVolume(int percent);
  void StartFader(int duration_msec,
                  QTimeLine::Direction direction = QTimeLine::Forward,
                  QTimeLine::CurveShape shape = QTimeLine::LinearCurve);

  // Get information about the music playback
  bool is_valid() const { return valid_; }
  qint64 position() const;
  qint64 length() const;
  GstState state() const;

 public slots:
  void SetVolumeModifier(qreal mod);

 signals:
  void EndOfStreamReached();
  void MetadataFound(const Engine::SimpleMetaBundle& bundle);
  void BufferFound(GstBuffer* buf);
  void Error(const QString& message);
  void FaderFinished();

 private:
  // Static callbacks.  The GstEnginePipeline instance is passed in the last
  // argument.
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage*, gpointer);
  static gboolean BusCallback(GstBus*, GstMessage*, gpointer);
  static void NewPadCallback(GstElement*, GstPad*, gboolean, gpointer);
  static bool HandoffCallback(GstPad*, GstBuffer*, gpointer);
  static void EventCallback(GstPad*, GstEvent*, gpointer);
  void TagMessageReceived(GstMessage*);
  QString ParseTag(GstTagList* list, const char* tag) const;

  void UpdateVolume();

 private:
  static const int kGstStateTimeoutNanosecs = 10000000;

  static const char* kHttpGstreamerSource;

  bool valid_;
  QString sink_;
  QString device_;
  bool forwards_buffers_;

  int volume_percent_;
  qreal volume_modifier_;

  QTimeLine* fader_;

  GstElement* pipeline_;

  // Bins
  // src ! decodebin ! audiobin
  GstElement* src_;
  GstElement* decodebin_;
  GstElement* audiobin_;

  // Elements in the audiobin
  // audioconvert ! equalizer ! volume ! audioscale ! audioconvert ! audiosink
  GstElement* audioconvert_;
  GstElement* equalizer_;
  GstElement* volume_;
  GstElement* audioscale_;
  GstElement* audiosink_;

  uint event_cb_id_;
};

#endif // GSTENGINEPIPELINE_H
