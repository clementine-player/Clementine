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

#include <QObject>
#include <QStringList>
#include <QEvent>
#include <QMetaType>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include "core/song.h"


struct TranscoderPreset {
  TranscoderPreset() : type_(Song::Type_Unknown) {}
  TranscoderPreset(Song::FileType type,
                   const QString& name,
                   const QString& extension,
                   const QString& codec_mimetype,
                   const QString& muxer_mimetype_ = QString());

  Song::FileType type_;
  QString name_;
  QString extension_;
  QString codec_mimetype_;
  QString muxer_mimetype_;
};
Q_DECLARE_METATYPE(TranscoderPreset);


class Transcoder : public QObject {
  Q_OBJECT

 public:
  Transcoder(QObject* parent = 0);

  static TranscoderPreset PresetForFileType(Song::FileType type);
  static QList<TranscoderPreset> GetAllPresets();
  static Song::FileType PickBestFormat(QList<Song::FileType> supported);

  int max_threads() const { return max_threads_; }
  void set_max_threads(int count) { max_threads_ = count; }

  void AddJob(const QString& input, const TranscoderPreset& preset,
              const QString& output = QString());

  QMap<QString, float> GetProgress() const;
  int QueuedJobsCount() const { return queued_jobs_.count(); }

 public slots:
  void Start();
  void Cancel();

 signals:
  void JobComplete(const QString& filename, bool success);
  void LogLine(const QString& message);
  void AllJobsComplete();
  void JobOutputName(const QString& filename);

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
  struct JobState {
    JobState(const Job& job, Transcoder* parent)
      : job_(job), parent_(parent), pipeline_(NULL), convert_element_(NULL),
        bus_callback_id_(0) {}
    ~JobState();

    void PostFinished(bool success);
    void ReportError(GstMessage* msg);

    Job job_;
    Transcoder* parent_;
    GstElement* pipeline_;
    GstElement* convert_element_;
    int bus_callback_id_;
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

  GstElement* CreateElement(const QString& factory_name, GstElement* bin = NULL,
                            const QString& name = QString());
  GstElement* CreateElementForMimeType(const QString& element_type,
                                       const QString& mime_type,
                                       GstElement* bin = NULL);
  void SetElementProperties(const QString& name, GObject* element);

  static void NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer data);
  static gboolean BusCallback(GstBus*, GstMessage* msg, gpointer data);
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage* msg, gpointer data);

 private:
  typedef QList<boost::shared_ptr<JobState> > JobStateList;

  int max_threads_;
  QList<Job> queued_jobs_;
  JobStateList current_jobs_;
};

#endif // TRANSCODER_H
