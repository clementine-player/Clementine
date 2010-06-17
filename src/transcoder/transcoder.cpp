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

#include "transcoder.h"
#include "transcoderformats.h"

#include <QtConcurrentMap>
#include <QtDebug>
#include <QFile>
#include <QCoreApplication>

#include <boost/bind.hpp>

using boost::shared_ptr;

int Transcoder::JobFinishedEvent::sEventType = -1;


GstElement* Transcoder::CreateElement(const QString &factory_name,
                                      GstElement *bin,
                                      const QString &name) {
  GstElement* ret = gst_element_factory_make(
      factory_name.toAscii().constData(),
      name.isNull() ? factory_name.toAscii().constData() : name.toAscii().constData());

  if (ret && bin)
    gst_bin_add(GST_BIN(bin), ret);

  if (!ret) {
    emit LogLine(
        tr("Could not create the GStreamer element \"%1\" -"
           " make sure you have all the required GStreamer plugins installed")
        .arg(factory_name));
  }

  return ret;
}

GstElement* Transcoder::CreateBin(const QStringList& elements) {
  GstElement* bin = gst_bin_new("outputbin");

  GstElement* last_element = NULL;
  for (int i=0 ; i<elements.count() ; ++i) {
    GstElement* element = CreateElement(elements[i], bin);
    if (!element) {
      gst_object_unref(bin);
      return NULL;
    }

    if (i == 0) {
      // If this is the first element, make it the bin's sink
      GstPad* pad = gst_element_get_pad(element, "sink");
      gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad));
      gst_object_unref(pad);
    }

    if (i == elements.count() - 1) {
      // If this is the last element, make it the bin's src
      GstPad* pad = gst_element_get_pad(element, "src");
      gst_element_add_pad(bin, gst_ghost_pad_new("src", pad));
      gst_object_unref(pad);
    }

    // Link the last element to this one
    if (last_element)
      gst_element_link(last_element, element);
    last_element = element;
  }

  return bin;
}


Transcoder::JobFinishedEvent::JobFinishedEvent(JobState *state, bool success)
  : QEvent(QEvent::Type(sEventType)),
    state_(state),
    success_(success)
{
}

void Transcoder::JobState::PostFinished(bool success) {
  QCoreApplication::postEvent(parent_, new Transcoder::JobFinishedEvent(this, success));

  if (success) {
    emit parent_->LogLine(
        tr("Successfully written %1").arg(job_.output));
  }
}


Transcoder::Transcoder(QObject* parent)
  : QObject(parent),
    max_threads_(QThread::idealThreadCount())
{
  if (JobFinishedEvent::sEventType == -1)
    JobFinishedEvent::sEventType = QEvent::registerEventType();

  formats_ << new OggVorbisTranscoder;
  formats_ << new OggSpeexTranscoder;
  formats_ << new FlacTranscoder;
  formats_ << new Mp3Transcoder;
  formats_ << new M4aTranscoder;
  formats_ << new ThreeGPTranscoder;
}

Transcoder::~Transcoder() {
  qDeleteAll(formats_);
}

QList<const TranscoderFormat*> Transcoder::formats() const {
  QList<const TranscoderFormat*> ret;
  foreach (TranscoderFormat* format, formats_)
    ret << format;
  return ret;
}

void Transcoder::AddJob(const QString &input,
                        const TranscoderFormat *output_format,
                        const QString &output) {
  Job job;
  job.input = input;
  job.output_format = output_format;

  // Use the supplied filename if there was one, otherwise take the file
  // extension off the input filename and append the correct one.
  if (!output.isEmpty())
    job.output = output;
  else
    job.output = input.section('.', 0, -2) + '.' + output_format->file_extension();

  // Never overwrite existing files
  if (QFile::exists(job.output)) {
    for (int i=0 ; ; ++i) {
      QString new_filename = QString("%1.%2").arg(job.output).arg(i);
      if (!QFile::exists(new_filename)) {
        job.output = new_filename;
        break;
      }
    }
  }

  queued_jobs_ << job;
}

void Transcoder::Start() {
  emit LogLine(tr("Transcoding %1 files using %2 threads")
               .arg(queued_jobs_.count()).arg(max_threads()));

  forever {
    StartJobStatus status = MaybeStartNextJob();
    if (status == AllThreadsBusy || status == NoMoreJobs)
      break;
  }
}

Transcoder::StartJobStatus Transcoder::MaybeStartNextJob() {
  if (current_jobs_.count() >= max_threads())
    return AllThreadsBusy;
  if (queued_jobs_.isEmpty()) {
    if (current_jobs_.isEmpty()) {
      emit AllJobsComplete();
    }

    return NoMoreJobs;
  }

  Job job = queued_jobs_.takeFirst();
  return StartJob(job) ? StartedSuccessfully : FailedToStart;
}

void Transcoder::NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer data) {
  JobState* state = reinterpret_cast<JobState*>(data);
  GstPad* const audiopad = gst_element_get_pad(state->convert_element_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qDebug() << "audiopad is already linked. Unlinking old pad.";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);
}

gboolean Transcoder::BusCallback(GstBus*, GstMessage* msg, gpointer data) {
  JobState* state = reinterpret_cast<JobState*>(data);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR:
      state->ReportError(msg);
      state->PostFinished(false);
      break;

    default:
      break;
  }
  return GST_BUS_DROP;
}

GstBusSyncReply Transcoder::BusCallbackSync(GstBus*, GstMessage* msg, gpointer data) {
  JobState* state = reinterpret_cast<JobState*>(data);
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      state->PostFinished(true);
      break;

    case GST_MESSAGE_ERROR:
      state->ReportError(msg);
      state->PostFinished(false);
      break;

    default:
      break;
  }
  return GST_BUS_PASS;
}

void Transcoder::JobState::ReportError(GstMessage* msg) {
  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  QString message = QString::fromLocal8Bit(error->message);

  g_error_free(error);
  free(debugs);

  emit parent_->LogLine(
      tr("Error processing %1: %2").arg(job_.input, message));
}

bool Transcoder::StartJob(const Job &job) {
  shared_ptr<JobState> state(new JobState(job, this));

  emit LogLine(tr("Starting %1").arg(job.input));

  // Create the pipeline.
  // This should be a scoped_ptr, but scoped_ptr doesn't support custom
  // destructors.
  state->pipeline_.reset(gst_pipeline_new("pipeline"),
                        boost::bind(gst_object_unref, _1));
  if (!state->pipeline_) return false;

  // Create all the elements
  const TranscoderFormat* f = job.output_format;
  GstElement* src     = CreateElement("filesrc", state->pipeline_.get());
  GstElement* decode  = CreateElement("decodebin2", state->pipeline_.get());
  GstElement* convert = CreateElement("audioconvert", state->pipeline_.get());
  GstElement* encode  = CreateBin(f->gst_elements());
  GstElement* sink    = CreateElement("filesink", state->pipeline_.get());

  if (!src || !decode || !convert || !encode || !sink)
    return false;

  // Join them together
  gst_bin_add(GST_BIN(state->pipeline_.get()), encode);
  gst_element_link(src, decode);
  gst_element_link_many(convert, encode, sink, NULL);

  // Set properties
  g_object_set(src, "location", job.input.toLocal8Bit().constData(), NULL);
  g_object_set(sink, "location", job.output.toLocal8Bit().constData(), NULL);

  // Set callbacks
  state->convert_element_ = convert;

  g_signal_connect(decode, "new-decoded-pad", G_CALLBACK(NewPadCallback), state.get());
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(state->pipeline_.get())), BusCallbackSync, state.get());
  state->bus_callback_id_ = gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(state->pipeline_.get())), BusCallback, state.get());

  // Start the pipeline
  gst_element_set_state(state->pipeline_.get(), GST_STATE_PLAYING);

  // GStreamer now transcodes in another thread, so we can return now and do
  // something else.  Keep the JobState object around.  It'll post an event
  // to our event loop when it finishes.
  current_jobs_ << state;

  return true;
}

bool Transcoder::event(QEvent* e) {
  if (e->type() == JobFinishedEvent::sEventType) {
    JobFinishedEvent* finished_event = static_cast<JobFinishedEvent*>(e);

    // Find this job in the list
    JobStateList::iterator it = current_jobs_.begin();
    while (it != current_jobs_.end()) {
      if (it->get() == finished_event->state_)
        break;
      ++it;
    }
    if (it == current_jobs_.end()) {
      // Couldn't find it, maybe GStreamer gave us an event after we'd destroyed
      // the pipeline?
      return true;
    }

    // Emit the finished signal
    emit JobComplete((*it)->job_.input, finished_event->success_);

    // Remove event handlers from the gstreamer pipeline so they don't get
    // called after the pipeline is shutting down
    gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(
        finished_event->state_->pipeline_.get())), NULL, NULL);
    g_source_remove(finished_event->state_->bus_callback_id_);

    // Remove it from the list - this will also destroy the GStreamer pipeline
    current_jobs_.erase(it);

    // Start some more jobs
    MaybeStartNextJob();

    return true;
  }

  return QObject::event(e);
}

void Transcoder::Cancel() {
  // Remove all pending jobs
  queued_jobs_.clear();

  // Stop the running ones
  JobStateList::iterator it = current_jobs_.begin();
  while (it != current_jobs_.end()) {
    shared_ptr<JobState> state(*it);

    // Remove event handlers from the gstreamer pipeline so they don't get
    // called after the pipeline is shutting down
    gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(
        state->pipeline_.get())), NULL, NULL);
    g_source_remove(state->bus_callback_id_);

    // Stop the pipeline
    if (gst_element_set_state(state->pipeline_.get(), GST_STATE_NULL) == GST_STATE_CHANGE_ASYNC) {
      // Wait for it to finish stopping...
      gst_element_get_state(state->pipeline_.get(), NULL, NULL, GST_CLOCK_TIME_NONE);
    }

    // Remove the job, this destroys the GStreamer pipeline too
    it = current_jobs_.erase(it);
  }
}
