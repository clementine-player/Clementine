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

#include "gstenginepipeline.h"
#include "gstequalizer.h"
#include "gstengine.h"

#include <QDebug>

GstEnginePipeline::GstEnginePipeline()
  : QObject(NULL),
    valid_(false),
    sink_(GstEngine::kAutoSink),
    pipeline_(NULL),
    src_(NULL),
    decodebin_(NULL),
    audiobin_(NULL),
    audioconvert_(NULL),
    equalizer_(NULL),
    identity_(NULL),
    volume_(NULL),
    audioscale_(NULL),
    audiosink_(NULL),
    event_cb_id_(0)
{
}

void GstEnginePipeline::set_output_device(const QString &sink, const QString &device) {
  sink_ = sink;
  device_ = device;
}

bool GstEnginePipeline::Init(const QUrl &url) {
  pipeline_ = gst_pipeline_new("pipeline");

  // Source
  src_ = GstEngine::CreateElement("giosrc");
  if (!src_)
    return false;

  g_object_set(G_OBJECT(src_), "location", url.toEncoded().constData(), NULL);
  gst_bin_add(GST_BIN(pipeline_), src_);

  // Decode bin
  if (!(decodebin_ = GstEngine::CreateElement("decodebin", pipeline_))) { return false; }
  g_signal_connect(G_OBJECT(decodebin_), "new-decoded-pad", G_CALLBACK(NewPadCallback), this);

  GstPad* pad = gst_element_get_pad(decodebin_, "sink");
  if (pad) {
    event_cb_id_ = gst_pad_add_event_probe (pad, G_CALLBACK(EventCallback), this);
    gst_object_unref(pad);
  }

  // The link from decodebin to audioconvert will be made in the newPad-callback
  gst_element_link(src_, decodebin_);

  // Audio bin
  audiobin_ = gst_bin_new("audiobin");
  gst_bin_add(GST_BIN(pipeline_), audiobin_);

  if (!(audiosink_ = GstEngine::CreateElement(sink_, audiobin_)))
    return false;

  if (GstEngine::DoesThisSinkSupportChangingTheOutputDeviceToAUserEditableString(sink_) && !device_.isEmpty())
    g_object_set(G_OBJECT(audiosink_), "device", device_.toUtf8().constData(), NULL);

  equalizer_ = GST_ELEMENT(gst_equalizer_new());
  gst_bin_add(GST_BIN(audiobin_), equalizer_);

  if (!(audioconvert_ = GstEngine::CreateElement("audioconvert", audiobin_))) { return false; }
  if (!(identity_ = GstEngine::CreateElement("identity", audiobin_))) { return false; }
  if (!(volume_ = GstEngine::CreateElement("volume", audiobin_))) { return false; }
  if (!(audioscale_ = GstEngine::CreateElement("audioresample", audiobin_))) { return false; }

  pad = gst_element_get_pad(audioconvert_, "sink");
  gst_element_add_pad(audiobin_, gst_ghost_pad_new("sink", pad));
  gst_object_unref(pad);

  // add a data probe on the src pad if the audioconvert element for our scope
  // we do it here because we want pre-equalized and pre-volume samples
  // so that our visualization are not affected by them
  pad = gst_element_get_pad(audioconvert_, "src");
  gst_pad_add_buffer_probe(pad, G_CALLBACK(HandoffCallback), this);
  gst_object_unref (pad);

  // Ensure we get the right type out of audioconvert for our scope
  GstCaps* caps = gst_caps_new_simple ("audio/x-raw-int",
      "width", G_TYPE_INT, 16,
      "signed", G_TYPE_BOOLEAN, true,
      NULL);
  gst_element_link_filtered(audioconvert_, equalizer_, caps);
  gst_caps_unref(caps);

  // Add an extra audioconvert at the end as osxaudiosink supports only one format.
  GstElement* convert = GstEngine::CreateElement("audioconvert", audiobin_, "FFFUUUU");
  if (!convert) { return false; }
  gst_element_link_many(equalizer_, identity_, volume_,
                        audioscale_, convert, audiosink_, NULL);

  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), BusCallbackSync, this);
  gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(pipeline_)), BusCallback, this);

  return true;
}

GstEnginePipeline::~GstEnginePipeline() {
  // We don't want an EOS signal from the decodebin
  if (decodebin_) {
    GstPad *p = gst_element_get_pad(decodebin_, "sink");
    if (p)
      gst_pad_remove_event_probe(p, event_cb_id_);
  }

  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline_));
  }
}



gboolean GstEnginePipeline::BusCallback(GstBus*, GstMessage* msg, gpointer self) {
  GstEnginePipeline* instance = static_cast<GstEnginePipeline*>(self);

  switch ( GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
      GError* error;
      gchar* debugs;

      gst_message_parse_error(msg, &error, &debugs);
      qDebug() << "ERROR RECEIVED IN BUS_CB <" << error->message << ">" ;

      emit instance->Error(QString::fromAscii(error->message));
      break;
    }

    case GST_MESSAGE_TAG: {
      gchar* string=NULL;
      Engine::SimpleMetaBundle bundle;
      GstTagList* taglist;
      gst_message_parse_tag(msg,&taglist);
      bool success = false;

      if ( gst_tag_list_get_string( taglist, GST_TAG_TITLE, &string ) && string ) {
        qDebug() << "received tag 'Title': " << QString( string ) ;
        bundle.title = string;
        success = true;
      }
      if ( gst_tag_list_get_string( taglist, GST_TAG_ARTIST, &string ) && string ) {
        qDebug() << "received tag 'Artist': " << QString( string ) ;
        bundle.artist = string;
        success = true;
      }
      if ( gst_tag_list_get_string( taglist, GST_TAG_COMMENT, &string ) && string ) {
        qDebug() << "received tag 'Comment': " << QString( string ) ;
        bundle.comment = string;
        success = true;
      }
      if ( gst_tag_list_get_string( taglist, GST_TAG_ALBUM, &string ) && string ) {
        qDebug() << "received tag 'Album': " << QString( string ) ;
        bundle.album = string;
        success = true;
      }
      g_free(string);
      gst_tag_list_free(taglist);
      if (success)
        emit instance->MetadataFound(bundle);
      break;
    }

    default:
      break;
  }
  return GST_BUS_DROP;
}

GstBusSyncReply GstEnginePipeline::BusCallbackSync(GstBus*, GstMessage* msg, gpointer self) {
  GstEnginePipeline* instance = static_cast<GstEnginePipeline*>(self);
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      emit instance->EndOfStreamReached();
      break;

    default:
      break;
  }

  return GST_BUS_PASS;
}


void GstEnginePipeline::NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer self) {
  GstEnginePipeline* instance = static_cast<GstEnginePipeline*>(self);
  GstPad* const audiopad = gst_element_get_pad(instance->audiobin_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qDebug() << "audiopad is already linked. Unlinking old pad." ;
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);

  gst_object_unref(audiopad);
}


void GstEnginePipeline::HandoffCallback(GstPad*, GstBuffer* buf, gpointer self) {
  GstEnginePipeline* instance = static_cast<GstEnginePipeline*>(self);

  if (instance->forwards_buffers_) {
    gst_buffer_ref(buf);
    emit instance->BufferFound(buf);
  }
}

void GstEnginePipeline::EventCallback(GstPad*, GstEvent* event, gpointer self) {
  GstEnginePipeline* instance = static_cast<GstEnginePipeline*>(self);

  switch(static_cast<int>(event->type)) {
    case GST_EVENT_EOS:
      emit instance->EndOfStreamReached();
      break;

    default:
      break;
  }
}

qint64 GstEnginePipeline::position() const {
  GstFormat fmt = GST_FORMAT_TIME;
  gint64 value = 0;
  gst_element_query_position(pipeline_, &fmt, &value);

  return value;
}


qint64 GstEnginePipeline::length() const {
  GstFormat fmt = GST_FORMAT_TIME;
  gint64 value = 0;
  gst_element_query_duration(pipeline_,  &fmt, &value);

  return value;
}


GstState GstEnginePipeline::state() const {
  GstState s, sp;
  if (gst_element_get_state(pipeline_, &s, &sp, kGstStateTimeout) ==
      GST_STATE_CHANGE_FAILURE)
    return GST_STATE_NULL;

  return s;
}

bool GstEnginePipeline::SetState(GstState state) {
  return gst_element_set_state(pipeline_, state) != GST_STATE_CHANGE_FAILURE;
}

bool GstEnginePipeline::Seek(qint64 nanosec) {
  return gst_element_seek(pipeline_, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                          GST_SEEK_TYPE_SET, nanosec, GST_SEEK_TYPE_NONE,
                          GST_CLOCK_TIME_NONE);
}

void GstEnginePipeline::SetEqualizerEnabled(bool enabled) {
  g_object_set(G_OBJECT(equalizer_), "active", enabled, NULL);
}


void GstEnginePipeline::SetEqualizerParams(int preamp, const QList<int>& band_gains) {
  // Preamp
  g_object_set(G_OBJECT(equalizer_), "preamp", ( preamp + 100 ) / 2, NULL);

  // Gains
  std::vector<int> gains_temp;
  gains_temp.resize( band_gains.count() );
  for ( int i = 0; i < band_gains.count(); i++ )
    gains_temp[i] = band_gains.at( i ) + 100;

  g_object_set(G_OBJECT(equalizer_), "gain", &gains_temp, NULL);
}

void GstEnginePipeline::SetVolume(int percent) {
  g_object_set(G_OBJECT(volume_), "volume", double(percent) * 0.01, NULL);
}
