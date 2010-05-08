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

#ifndef TRANSCODER_H
#define TRANSCODER_H

#include <gst/gst.h>

#include <QObject>
#include <QStringList>
#include <QEvent>
#include <QMetaType>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

class Transcoder;

class TranscoderFormat {
  friend class Transcoder;

 public:
  virtual ~TranscoderFormat() {}

  virtual QString name() const = 0;
  virtual QString file_extension() const = 0;

  virtual QStringList gst_elements() const = 0;
};
Q_DECLARE_METATYPE(const TranscoderFormat*);


class Transcoder : public QObject {
  Q_OBJECT

 public:
  Transcoder(QObject* parent = 0);
  ~Transcoder();

  QList<const TranscoderFormat*> formats() const;
  int max_threads() const { return max_threads_; }

  void set_max_threads(int count) { max_threads_ = count; }

  void AddJob(const QString& input, const TranscoderFormat* output_format,
              const QString& output = QString());

 public slots:
  void Start();
  void Cancel();

 signals:
  void JobComplete(const QString& filename, bool success);
  void LogLine(const QString& message);
  void AllJobsComplete();

 protected:
  bool event(QEvent* e);

 private:
  // The description of a file to transcode - lives in the main thread.
  struct Job {
    QString input;
    QString output;
    const TranscoderFormat* output_format;
  };

  // State held by a job and shared across gstreamer callbacks - lives in the
  // job's thread.
  struct JobState {
    JobState(const Job& job, Transcoder* parent)
      : job_(job), parent_(parent), convert_element_(NULL), bus_callback_id_(0) {}

    void PostFinished(bool success);
    void ReportError(GstMessage* msg);

    Job job_;
    Transcoder* parent_;
    boost::shared_ptr<GstElement> pipeline_;
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
  GstElement* CreateBin(const QStringList& elements);

  static void NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer data);
  static gboolean BusCallback(GstBus*, GstMessage* msg, gpointer data);
  static GstBusSyncReply BusCallbackSync(GstBus*, GstMessage* msg, gpointer data);

 private:
  typedef QList<boost::shared_ptr<JobState> > JobStateList;

  int max_threads_;
  QList<TranscoderFormat*> formats_;
  QList<Job> queued_jobs_;
  JobStateList current_jobs_;
};

#endif // TRANSCODER_H
