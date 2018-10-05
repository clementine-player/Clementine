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

#include "transcoder.h"

#include <algorithm>
#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QThread>
#include <QtDebug>

#include "core/logging.h"
#include "core/signalchecker.h"
#include "core/utilities.h"

using std::shared_ptr;

int Transcoder::JobFinishedEvent::sEventType = -1;

TranscoderPreset::TranscoderPreset(Song::FileType type, const QString& name,
                                   const QString& extension,
                                   const QString& codec_mimetype,
                                   const QString& muxer_mimetype)
    : type_(type),
      name_(name),
      extension_(extension),
      codec_mimetype_(codec_mimetype),
      muxer_mimetype_(muxer_mimetype) {}

GstElement* Transcoder::CreateElement(const QString& factory_name,
                                      GstElement* bin, const QString& name) {
  GstElement* ret = gst_element_factory_make(
      factory_name.toAscii().constData(),
      name.isNull() ? factory_name.toAscii().constData()
                    : name.toAscii().constData());

  if (ret && bin) gst_bin_add(GST_BIN(bin), ret);

  if (!ret) {
    emit LogLine(
        tr("Could not create the GStreamer element \"%1\" -"
           " make sure you have all the required GStreamer plugins installed")
            .arg(factory_name));
  } else {
    SetElementProperties(factory_name, G_OBJECT(ret));
  }

  return ret;
}

struct SuitableElement {
  SuitableElement(const QString& name = QString(), int rank = 0)
      : name_(name), rank_(rank) {}

  bool operator<(const SuitableElement& other) const {
    return rank_ < other.rank_;
  }

  QString name_;
  int rank_;
};

GstElement* Transcoder::CreateElementForMimeType(const QString& element_type,
                                                 const QString& mime_type,
                                                 GstElement* bin) {
  if (mime_type.isEmpty()) return nullptr;

  // HACK: Force mp4mux because it doesn't set any useful src caps
  if (mime_type == "audio/mp4") {
    LogLine(QString("Using '%1' (rank %2)").arg("mp4mux").arg(-1));
    return CreateElement("mp4mux", bin);
  }

  // Keep track of all the suitable elements we find and figure out which
  // is the best at the end.
  QList<SuitableElement> suitable_elements_;

  // The caps we're trying to find
  GstCaps* target_caps = gst_caps_from_string(mime_type.toUtf8().constData());

  GstRegistry* registry = gst_registry_get();
  GList* const features =
      gst_registry_get_feature_list(registry, GST_TYPE_ELEMENT_FACTORY);

  for (GList* p = features; p; p = g_list_next(p)) {
    GstElementFactory* factory = GST_ELEMENT_FACTORY(p->data);

    // Is this the right type of plugin?
    if (QString(gst_element_factory_get_klass(factory)).contains(element_type)) {
      const GList* const templates =
          gst_element_factory_get_static_pad_templates(factory);
      for (const GList* p = templates; p; p = g_list_next(p)) {
        // Only interested in source pads
        GstStaticPadTemplate* pad_template =
            reinterpret_cast<GstStaticPadTemplate*>(p->data);
        if (pad_template->direction != GST_PAD_SRC) continue;

        // Does this pad support the mime type we want?
        GstCaps* caps = gst_static_pad_template_get_caps(pad_template);
        GstCaps* intersection = gst_caps_intersect(caps, target_caps);

        if (intersection) {
          if (!gst_caps_is_empty(intersection)) {
            int rank = gst_plugin_feature_get_rank(GST_PLUGIN_FEATURE(factory));
            QString name = GST_OBJECT_NAME(factory);

            if (name.startsWith("ffmux") || name.startsWith("ffenc"))
              rank = -1;  // ffmpeg usually sucks

            suitable_elements_ << SuitableElement(name, rank);
          }
          gst_caps_unref(intersection);
        }
      }
    }
  }

  gst_plugin_feature_list_free(features);
  gst_caps_unref(target_caps);

  if (suitable_elements_.isEmpty()) return nullptr;

  // Sort by rank
  std::sort(suitable_elements_.begin(), suitable_elements_.end());
  const SuitableElement& best = suitable_elements_.last();

  LogLine(QString("Using '%1' (rank %2)").arg(best.name_).arg(best.rank_));

  if (best.name_ == "lamemp3enc") {
    // Special case: we need to add xingmux and id3v2mux to the pipeline when
    // using lamemp3enc because it doesn't write the VBR or ID3v2 headers
    // itself.

    LogLine("Adding xingmux and id3v2mux to the pipeline");

    // Create the bin
    GstElement* mp3bin = gst_bin_new("mp3bin");
    gst_bin_add(GST_BIN(bin), mp3bin);

    // Create the elements
    GstElement* lame = CreateElement("lamemp3enc", mp3bin);
    GstElement* xing = CreateElement("xingmux", mp3bin);
    GstElement* id3v2 = CreateElement("id3v2mux", mp3bin);

    if (!lame || !xing || !id3v2) {
      return nullptr;
    }

    // Link the elements together
    gst_element_link_many(lame, xing, id3v2, nullptr);

    // Link the bin's ghost pads to the elements on each end
    GstPad* pad = gst_element_get_static_pad(lame, "sink");
    gst_element_add_pad(mp3bin, gst_ghost_pad_new("sink", pad));
    gst_object_unref(GST_OBJECT(pad));

    pad = gst_element_get_static_pad(id3v2, "src");
    gst_element_add_pad(mp3bin, gst_ghost_pad_new("src", pad));
    gst_object_unref(GST_OBJECT(pad));

    return mp3bin;
  } else {
    return CreateElement(best.name_, bin);
  }
}

Transcoder::JobFinishedEvent::JobFinishedEvent(JobState* state, bool success)
    : QEvent(QEvent::Type(sEventType)), state_(state), success_(success) {}

void Transcoder::JobState::PostFinished(bool success) {
  if (success) {
    emit parent_->LogLine(tr("Successfully written %1")
                              .arg(QDir::toNativeSeparators(job_.output)));
  }

  QCoreApplication::postEvent(parent_,
                              new Transcoder::JobFinishedEvent(this, success));
}

Transcoder::Transcoder(QObject* parent, const QString& settings_postfix)
    : QObject(parent),
      max_threads_(QThread::idealThreadCount()),
      settings_postfix_(settings_postfix) {
  if (JobFinishedEvent::sEventType == -1)
    JobFinishedEvent::sEventType = QEvent::registerEventType();

  // Initialise some settings for the lamemp3enc element.
  QSettings s;
  s.beginGroup("Transcoder/lamemp3enc" + settings_postfix_);

  if (s.value("target").isNull()) {
    s.setValue("target", 1);  // 1 == bitrate
  }
  if (s.value("cbr").isNull()) {
    s.setValue("cbr", true);
  }
}

QList<TranscoderPreset> Transcoder::GetAllPresets() {
  QList<TranscoderPreset> ret;
  ret << PresetForFileType(Song::Type_Flac);
  ret << PresetForFileType(Song::Type_Mp4);
  ret << PresetForFileType(Song::Type_Mpeg);
  ret << PresetForFileType(Song::Type_OggVorbis);
  ret << PresetForFileType(Song::Type_OggFlac);
  ret << PresetForFileType(Song::Type_OggSpeex);
  ret << PresetForFileType(Song::Type_Asf);
  ret << PresetForFileType(Song::Type_Wav);
  ret << PresetForFileType(Song::Type_OggOpus);
  return ret;
}

TranscoderPreset Transcoder::PresetForFileType(Song::FileType type) {
  switch (type) {
    case Song::Type_Flac:
      return TranscoderPreset(type, tr("FLAC"), "flac", "audio/x-flac");
    case Song::Type_Mp4:
      return TranscoderPreset(type, tr("M4A AAC"), "mp4",
                              "audio/mpeg, mpegversion=(int)4", "audio/mp4");
    case Song::Type_Mpeg:
      return TranscoderPreset(type, tr("MP3"), "mp3",
                              "audio/mpeg, mpegversion=(int)1, layer=(int)3");
    case Song::Type_OggVorbis:
      return TranscoderPreset(type, tr("Ogg Vorbis"), "ogg", "audio/x-vorbis",
                              "application/ogg");
    case Song::Type_OggFlac:
      return TranscoderPreset(type, tr("Ogg Flac"), "ogg", "audio/x-flac",
                              "application/ogg");
    case Song::Type_OggSpeex:
      return TranscoderPreset(type, tr("Ogg Speex"), "spx", "audio/x-speex",
                              "application/ogg");
    case Song::Type_OggOpus:
      return TranscoderPreset(type, tr("Ogg Opus"), "opus", "audio/x-opus",
                              "application/ogg");
    case Song::Type_Asf:
      return TranscoderPreset(type, tr("Windows Media audio"), "wma",
                              "audio/x-wma", "video/x-ms-asf");
    case Song::Type_Wav:
      return TranscoderPreset(type, tr("Wav"), "wav", QString(), "audio/x-wav");
    default:
      qLog(Warning) << "Unsupported format in PresetForFileType:" << type;
      return TranscoderPreset();
  }
}

Song::FileType Transcoder::PickBestFormat(QList<Song::FileType> supported) {
  if (supported.isEmpty()) return Song::Type_Unknown;

  QList<Song::FileType> best_formats;
  best_formats << Song::Type_Mpeg;
  best_formats << Song::Type_OggVorbis;
  best_formats << Song::Type_Asf;

  for (Song::FileType type : best_formats) {
    if (supported.isEmpty() || supported.contains(type)) return type;
  }

  return supported[0];
}

void Transcoder::AddJob(const QString& input, const TranscoderPreset& preset,
                        const QString& output) {
  Job job;
  job.input = input;
  job.preset = preset;

  // Use the supplied filename if there was one, otherwise take the file
  // extension off the input filename and append the correct one.
  if (!output.isEmpty())
    job.output = output;
  else
    job.output = input.section('.', 0, -2) + '.' + preset.extension_;

  // Never overwrite existing files
  if (QFile::exists(job.output)) {
    for (int i = 0;; ++i) {
      QString new_filename =
          QString("%1.%2.%3").arg(job.output.section('.', 0, -2)).arg(i).arg(
              preset.extension_);
      if (!QFile::exists(new_filename)) {
        job.output = new_filename;
        break;
      }
    }
  }

  queued_jobs_ << job;
}

void Transcoder::AddTemporaryJob(const QString &input, const TranscoderPreset &preset) {
  Job job;
  job.input = input;
  job.output = Utilities::GetTemporaryFileName();
  job.preset = preset;

  queued_jobs_ << job;
}

void Transcoder::Start() {
  emit LogLine(tr("Transcoding %1 files using %2 threads")
                   .arg(queued_jobs_.count())
                   .arg(max_threads()));

  forever {
    StartJobStatus status = MaybeStartNextJob();
    if (status == AllThreadsBusy || status == NoMoreJobs) break;
  }
}

Transcoder::StartJobStatus Transcoder::MaybeStartNextJob() {
  if (current_jobs_.count() >= max_threads()) return AllThreadsBusy;
  if (queued_jobs_.isEmpty()) {
    if (current_jobs_.isEmpty()) {
      emit AllJobsComplete();
    }

    return NoMoreJobs;
  }

  Job job = queued_jobs_.takeFirst();
  if (StartJob(job)) {
    return StartedSuccessfully;
  }

  emit JobComplete(job.input, job.output, false);
  return FailedToStart;
}

void Transcoder::NewPadCallback(GstElement*, GstPad* pad,
                                gpointer data) {
  JobState* state = reinterpret_cast<JobState*>(data);
  GstPad* const audiopad =
      gst_element_get_static_pad(state->convert_element_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qLog(Debug) << "audiopad is already linked, unlinking old pad";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);
}

GstBusSyncReply Transcoder::BusCallbackSync(GstBus*, GstMessage* msg,
                                            gpointer data) {
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

  emit parent_->LogLine(tr("Error processing %1: %2").arg(
      QDir::toNativeSeparators(job_.input), message));
}

bool Transcoder::StartJob(const Job& job) {
  shared_ptr<JobState> state(new JobState(job, this));

  emit LogLine(tr("Starting %1").arg(QDir::toNativeSeparators(job.input)));

  // Create the pipeline.
  // This should be a scoped_ptr, but scoped_ptr doesn't support custom
  // destructors.
  state->pipeline_ = gst_pipeline_new("pipeline");
  if (!state->pipeline_) return false;

  // Create all the elements
  GstElement* src = CreateElement("filesrc", state->pipeline_);
  GstElement* decode = CreateElement("decodebin", state->pipeline_);
  GstElement* convert = CreateElement("audioconvert", state->pipeline_);
  GstElement* resample = CreateElement("audioresample", state->pipeline_);
  GstElement* codec = CreateElementForMimeType(
      "Codec/Encoder/Audio", job.preset.codec_mimetype_, state->pipeline_);
  GstElement* muxer = CreateElementForMimeType(
      "Codec/Muxer", job.preset.muxer_mimetype_, state->pipeline_);
  GstElement* sink = CreateElement("filesink", state->pipeline_);

  if (!src || !decode || !convert || !sink) return false;

  if (!codec && !job.preset.codec_mimetype_.isEmpty()) {
    LogLine(tr("Couldn't find an encoder for %1, check you have the correct "
               "GStreamer plugins installed").arg(job.preset.codec_mimetype_));
    return false;
  }

  if (!muxer && !job.preset.muxer_mimetype_.isEmpty()) {
    LogLine(tr("Couldn't find a muxer for %1, check you have the correct "
               "GStreamer plugins installed").arg(job.preset.muxer_mimetype_));
    return false;
  }

  // Join them together
  gst_element_link(src, decode);
  if (codec && muxer)
    gst_element_link_many(convert, resample, codec, muxer, sink, nullptr);
  else if (codec)
    gst_element_link_many(convert, resample, codec, sink, nullptr);
  else if (muxer)
    gst_element_link_many(convert, resample, muxer, sink, nullptr);

  // Set properties
  g_object_set(src, "location", job.input.toUtf8().constData(), nullptr);
  g_object_set(sink, "location", job.output.toUtf8().constData(), nullptr);

  // Set callbacks
  state->convert_element_ = convert;

  CHECKED_GCONNECT(decode, "pad-added", &NewPadCallback, state.get());
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(state->pipeline_)),
                           BusCallbackSync, state.get(), nullptr);

  // Start the pipeline
  gst_element_set_state(state->pipeline_, GST_STATE_PLAYING);

  // GStreamer now transcodes in another thread, so we can return now and do
  // something else.  Keep the JobState object around.  It'll post an event
  // to our event loop when it finishes.
  current_jobs_ << state;

  return true;
}

Transcoder::JobState::~JobState() {
  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(pipeline_);
  }
}

bool Transcoder::event(QEvent* e) {
  if (e->type() == JobFinishedEvent::sEventType) {
    JobFinishedEvent* finished_event = static_cast<JobFinishedEvent*>(e);

    // Find this job in the list
    JobStateList::iterator it = current_jobs_.begin();
    while (it != current_jobs_.end()) {
      if (it->get() == finished_event->state_) break;
      ++it;
    }
    if (it == current_jobs_.end()) {
      // Couldn't find it, maybe GStreamer gave us an event after we'd destroyed
      // the pipeline?
      return true;
    }

    QString input = (*it)->job_.input;
    QString output = (*it)->job_.output;

    // Remove event handlers from the gstreamer pipeline so they don't get
    // called after the pipeline is shutting down
    gst_bus_set_sync_handler(
        gst_pipeline_get_bus(GST_PIPELINE(finished_event->state_->pipeline_)),
        nullptr, nullptr, nullptr);

    // Remove it from the list - this will also destroy the GStreamer pipeline
    current_jobs_.erase(it);

    // Emit the finished signal
    emit JobComplete(input, output, finished_event->success_);

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
    gst_bus_set_sync_handler(gst_pipeline_get_bus(
        GST_PIPELINE(state->pipeline_)), nullptr, nullptr, nullptr);

    // Stop the pipeline
    if (gst_element_set_state(state->pipeline_, GST_STATE_NULL) ==
        GST_STATE_CHANGE_ASYNC) {
      // Wait for it to finish stopping...
      gst_element_get_state(state->pipeline_, nullptr, nullptr,
                            GST_CLOCK_TIME_NONE);
    }

    // Remove the job, this destroys the GStreamer pipeline too
    it = current_jobs_.erase(it);
  }
}

QMap<QString, float> Transcoder::GetProgress() const {
  QMap<QString, float> ret;

  for (const auto& state : current_jobs_) {
    if (!state->pipeline_) continue;

    gint64 position = 0;
    gint64 duration = 0;

    gst_element_query_position(state->pipeline_, GST_FORMAT_TIME, &position);
    gst_element_query_duration(state->pipeline_, GST_FORMAT_TIME, &duration);

    ret[state->job_.input] = float(position) / duration;
  }

  return ret;
}

void Transcoder::SetElementProperties(const QString& name, GObject* object) {
  QSettings s;
  s.beginGroup("Transcoder/" + name + settings_postfix_);

  guint properties_count = 0;
  GParamSpec** properties = g_object_class_list_properties(
      G_OBJECT_GET_CLASS(object), &properties_count);

  for (int i = 0; i < properties_count; ++i) {
    GParamSpec* property = properties[i];

    const QVariant value = s.value(property->name);
    if (value.isNull()) continue;

    LogLine(QString("Setting %1 property: %2 = %3")
                .arg(name, property->name, value.toString()));

    switch (property->value_type) {
      case G_TYPE_DOUBLE:
        g_object_set(object, property->name, value.toDouble(), nullptr);
        break;
      case G_TYPE_FLOAT:
        g_object_set(object, property->name, value.toFloat(), nullptr);
        break;
      case G_TYPE_BOOLEAN:
        g_object_set(object, property->name, value.toInt(), nullptr);
        break;
      case G_TYPE_INT:
      default:
        g_object_set(object, property->name, value.toInt(), nullptr);
        break;
    }
  }

  g_free(properties);
}
