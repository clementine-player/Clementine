/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <gst/gst.h>

#include <QEvent>
#include <QMetaType>
#include <QObject>
#include <QStringList>
#include <memory>

#include "core/song.h"
#include "engines/gstpipelinebase.h"

struct TranscoderPreset {
  TranscoderPreset() : type_(Song::Type_Unknown) {}
  TranscoderPreset(Song::FileType type, const QString& name,
                   const QString& extension, const QString& codec_mimetype,
                   const QString& muxer_mimetype_ = QString());

  Song::FileType type_;
  QString name_;
  QString extension_;
  QString codec_mimetype_;
  QString muxer_mimetype_;
};
Q_DECLARE_METATYPE(TranscoderPreset)

class Transcoder : public QObject {
  Q_OBJECT

 public:
  Transcoder(QObject* parent = nullptr, const QString& settings_postfix = "");

  static TranscoderPreset PresetForFileType(Song::FileType type);
  static QList<TranscoderPreset> GetAllPresets();
  static Song::FileType PickBestFormat(QList<Song::FileType> supported);

  int max_threads() const { return max_threads_; }
  void set_max_threads(int count) { max_threads_ = count; }

  void AddJob(const QString& input, const TranscoderPreset& preset,
              const QString& output = QString());
  void AddTemporaryJob(const QString& input, const TranscoderPreset& preset);

  QMap<QString, float> GetProgress() const;
  int QueuedJobsCount() const { return queued_jobs_.count(); }

  GstPipelineModel* model() { return model_; }

  void Start();
  void Cancel();
  void DumpGraph(int id);

 signals:
  void JobComplete(const QString& input, const QString& output, bool success);
  void LogLine(const QString& message);
  void AllJobsComplete();

 protected:
  bool event(QEvent* e);

 private:
  // The description of a file to transcode - lives in the main thread.
  struct Job {
    QString input;
    QString output;
    TranscoderPreset preset;
  };

  // State held by a job and shared across gstreamer callbacks - lives in the
  // job's thread.
  class JobState : public GstPipelineBase {
   public:
    JobState(const Job& job, Transcoder* parent)
        : GstPipelineBase("transcode"),
          job_(job),
          parent_(parent),
          convert_element_(nullptr) {}

    void PostFinished(bool success);
    void ReportError(GstMessage* msg);

    GstElement* Pipeline() { return pipeline_; }
    QString GetDisplayName();

    Job job_;
    Transcoder* parent_;
    GstElement* convert_element_;
  };

  // Event passed from a GStreamer callback to the Transcoder when a job
  // finishes.
  struct JobFinishedEvent : public QEvent {
    JobFinishedEvent(JobState* state, bool success);

    static int sEventType;

    JobState* state_;
    bool success_;
  };

  enum StartJobStatus {
    StartedSuccessfully,
    FailedToStart,
    NoMoreJobs,
    AllThreadsBusy,
  };

  StartJobStatus MaybeStartNextJob();
  bool StartJob(const Job& job);

  GstElement* CreateElement(const QString& factory_name,
                            GstElement* bin = nullptr,
                            const QString& name = QString());
  GstElement* CreateElementForMimeType(const QString& element_type,
                                       const QString& mime_type,
                                       GstElement* bin = nullptr);
  void SetElementProperties(const QString& name, GObject* element);

  static void NewPadCallback(GstElement*, GstPad* pad, gpointer data);
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage* msg,
                                         gpointer data);

 private:
  typedef QList<std::shared_ptr<JobState>> JobStateList;

  int max_threads_;
  QList<Job> queued_jobs_;
  JobStateList current_jobs_;
  QString settings_postfix_;
  GstPipelineModel* model_;
};

#endif  // TRANSCODER_H
