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
#include <QBasicTimer>

#include <gst/gst.h>

#include "engine_fwd.h"

class GstEngine;

struct GstURIDecodeBin;

class GstEnginePipeline : public QObject {
  Q_OBJECT

 public:
  GstEnginePipeline(GstEngine* engine);
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

  // If this is set then it will be loaded automatically when playback finishes
  // for gapless playback
  void SetNextUrl(const QUrl& url) { next_url_ = url; }

  // Get information about the music playback
  QUrl url() const { return url_; }
  bool is_valid() const { return valid_; }
  qint64 position() const;
  qint64 length() const;
  GstState state() const;

 public slots:
  void SetVolumeModifier(qreal mod);

 signals:
  void EndOfStreamReached(bool has_next_track);
  void MetadataFound(const Engine::SimpleMetaBundle& bundle);
  void BufferFound(GstBuffer* buf);
  void Error(const QString& message);
  void FaderFinished();

 protected:
  void timerEvent(QTimerEvent *);

 private:
  // Static callbacks.  The GstEnginePipeline instance is passed in the last
  // argument.
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage*, gpointer);
  static gboolean BusCallback(GstBus*, GstMessage*, gpointer);
  static void NewPadCallback(GstElement*, GstPad*, gpointer);
  static bool HandoffCallback(GstPad*, GstBuffer*, gpointer);
  static void EventCallback(GstPad*, GstEvent*, gpointer);
  static void SourceDrainedCallback(GstURIDecodeBin*, gpointer);
  static bool StopUriDecodeBin(gpointer bin);
  void TagMessageReceived(GstMessage*);
  QString ParseTag(GstTagList* list, const char* tag) const;
  void ErrorMessageReceived(GstMessage*);

  void UpdateVolume();
  bool ReplaceDecodeBin(const QUrl& url);

 private slots:
  void FaderTimelineFinished();

 private:
  static const int kGstStateTimeoutNanosecs;
  static const int kFaderFudgeMsec;

  GstEngine* engine_;

  bool valid_;
  QString sink_;
  QString device_;
  bool forwards_buffers_;

  QUrl url_;
  QUrl next_url_;

  int volume_percent_;
  qreal volume_modifier_;

  QTimeLine* fader_;
  QBasicTimer fader_fudge_timer_;

  GstElement* pipeline_;

  // Bins
  // uridecodebin ! audiobin
  GstElement* uridecodebin_;
  GstElement* audiobin_;

  // Elements in the audiobin
  // audioconvert ! equalizer ! volume ! audioscale ! audioconvert ! audiosink
  GstElement* audioconvert_;
  GstElement* equalizer_;
  GstElement* volume_;
  GstElement* audioscale_;
  GstElement* audiosink_;

  uint bus_cb_id_;
};

#endif // GSTENGINEPIPELINE_H
