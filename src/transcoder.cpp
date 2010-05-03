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
#include <QEventLoop>
#include <QFile>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

using boost::shared_ptr;


GstElement* TranscoderFormat::CreateElement(const QString &factory_name,
                                            GstElement *bin,
                                            const QString &name) const {
  GstElement* ret = gst_element_factory_make(
      factory_name.toAscii().constData(),
      name.isNull() ? factory_name.toAscii().constData() : name.toAscii().constData());

  if (ret && bin)
    gst_bin_add(GST_BIN(bin), ret);

  return ret;
}

GstElement* TranscoderFormat::CreateBin(const QStringList& elements) const {
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

Transcoder::Transcoder(QObject* parent)
  : QObject(parent),
    future_watcher_(new QFutureWatcher<void>(this))
{
  formats_ << new OggVorbisTranscoder;
  formats_ << new OggSpeexTranscoder;
  formats_ << new FlacTranscoder;
  formats_ << new Mp3Transcoder;
  formats_ << new M4aTranscoder;
  formats_ << new ThreeGPTranscoder;

  connect(future_watcher_, SIGNAL(finished()), SLOT(JobsFinished()));
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
  Q_ASSERT(!future_watcher_->isRunning());

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

  jobs_ << job;
}

void Transcoder::Start() {
  Q_ASSERT(!future_watcher_->isRunning());

  QFuture<void> future = QtConcurrent::map(
      jobs_, boost::bind(&Transcoder::RunJob, this, _1));
  future_watcher_->setFuture(future);
}

void Transcoder::RunJob(const Job& job) {
  bool success = Transcode(job);

  emit JobComplete(job.input, success);
}

void Transcoder::NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer data) {
  JobState* state = reinterpret_cast<JobState*>(data);
  GstPad* const audiopad = gst_element_get_pad(state->convert_element, "sink");

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
      state->success = false;
      state->event_loop->exit();
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
      state->event_loop->exit();
      break;

    case GST_MESSAGE_ERROR:
      state->success = false;
      state->event_loop->exit();
      break;

    default:
      break;
  }
  return GST_BUS_PASS;
}

bool Transcoder::Transcode(const Job &job) const {
  // Create the pipeline.
  // This should be a scoped_ptr, but scoped_ptr doesn't support custom
  // destructors.
  shared_ptr<GstElement> pipeline(gst_pipeline_new("pipeline"),
                                  boost::bind(gst_object_unref, _1));
  if (!pipeline) return false;

  // Create all the elements
  const TranscoderFormat* f = job.output_format;
  GstElement* src     = f->CreateElement("filesrc", pipeline.get());
  GstElement* decode  = f->CreateElement("decodebin", pipeline.get());
  GstElement* convert = f->CreateElement("audioconvert", pipeline.get());
  GstElement* encode  = f->CreateEncodeBin();
  GstElement* sink    = f->CreateElement("filesink", pipeline.get());

  if (!src || !decode || !convert || !encode || !sink)
    return false;

  // Join them together
  gst_bin_add(GST_BIN(pipeline.get()), encode);
  gst_element_link(src, decode);
  gst_element_link_many(convert, encode, sink, NULL);

  // Set properties
  g_object_set(src, "location", job.input.toLocal8Bit().constData(), NULL);
  g_object_set(sink, "location", job.output.toLocal8Bit().constData(), NULL);

  // Set callbacks
  JobState state;
  state.convert_element = convert;
  state.event_loop.reset(new QEventLoop);
  state.success = true;

  g_signal_connect(decode, "new-decoded-pad", G_CALLBACK(NewPadCallback), &state);
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline.get())), BusCallbackSync, &state);
  gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(pipeline.get())), BusCallback, &state);

  // Start the pipeline and wait until it finishes
  gst_element_set_state(pipeline.get(), GST_STATE_PLAYING);

  state.event_loop->exec();

  // Do this explicitly so that it's guaranteed to happen before the event
  // loop is destroyed.
  pipeline.reset();

  return state.success;
}

void Transcoder::JobsFinished() {
  jobs_.clear();
  emit AllJobsComplete();
}

