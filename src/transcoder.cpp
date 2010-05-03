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

#include <boost/bind.hpp>


GstElement* TranscoderFormat::CreateElement(const QString &factory_name,
                                            GstElement *bin,
                                            const QString &name) const {
  GstElement* ret = gst_element_factory_make(
      factory_name.toAscii().constData(),
      name.isNull() ? factory_name.toAscii().constData() : name.toAscii().constData());

  if (ret) {
    if (bin) gst_bin_add(GST_BIN(bin), ret);
  } else {
    gst_object_unref(GST_OBJECT(bin));
  }

  return ret;
}

GstElement* TranscoderFormat::CreateBin(const QStringList& elements) const {
  GstElement* bin = gst_bin_new("outputbin");

  GstElement* last_element = NULL;
  for (int i=0 ; i<elements.count() ; ++i) {
    GstElement* element = CreateElement(elements[i], bin);
    if (!element) return NULL;

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
  formats_ << new AacTranscoder;

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

  if (!output.isEmpty())
    job.output = output;
  else
    job.output = input.section('.', 0, -2) + '.' + output_format->file_extension();

  jobs_ << job;
}

void Transcoder::Start() {
  Q_ASSERT(!future_watcher_->isRunning());

  QFuture<void> future = QtConcurrent::map(
      jobs_, boost::bind(&Transcoder::RunJob, this, _1));
  future_watcher_->setFuture(future);
}

void Transcoder::RunJob(const Job& job) {
  // TODO

  emit JobComplete(job.input, true);
}

void Transcoder::JobsFinished() {
  jobs_.clear();
  emit AllJobsComplete();
}

