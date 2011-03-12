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

#include "fingerprinter.h"

#include <QDir>
#include <QEventLoop>
#include <QtDebug>

Fingerprinter::Fingerprinter(const QString& filename)
  : filename_(filename),
    event_loop_(new QEventLoop),
    convert_element_(NULL)
{
}

Fingerprinter::~Fingerprinter() {
  delete event_loop_;
}

GstElement* Fingerprinter::CreateElement(const QString &factory_name,
                                       GstElement *bin) {
  GstElement* ret = gst_element_factory_make(
      factory_name.toAscii().constData(),
      factory_name.toAscii().constData());

  if (ret && bin)
    gst_bin_add(GST_BIN(bin), ret);

  if (!ret) {
    qDebug() << "Couldn't create the gstreamer element" << factory_name;
  }

  return ret;
}

QString Fingerprinter::CreateFingerprint() {
  GstElement* pipeline = gst_pipeline_new("pipeline");
  GstElement* src      = CreateElement("filesrc", pipeline);
  GstElement* decode   = CreateElement("decodebin2", pipeline);
  GstElement* convert  = CreateElement("audioconvert", pipeline);
  GstElement* ofa      = CreateElement("ofa", pipeline);
  GstElement* sink     = CreateElement("fakesink", pipeline);

  if (!src || !decode || !convert || !ofa || !sink) {
    return QString();
  }

  convert_element_ = convert;

  // Connect the elements
  gst_element_link_many(src, decode, NULL);
  gst_element_link_many(convert, ofa, sink, NULL);

  // Set the filename
  g_object_set(src, "location", filename_.toLocal8Bit().constData(), NULL);

  // Connect signals
  g_signal_connect(decode, "new-decoded-pad", G_CALLBACK(NewPadCallback), this);
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline)), BusCallbackSync, this);
  guint bus_callback_id = gst_bus_add_watch(gst_pipeline_get_bus(GST_PIPELINE(pipeline)), BusCallback, this);

  // Start playing
  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  event_loop_->exec();

  // Cleanup
  gst_bus_set_sync_handler(gst_pipeline_get_bus(GST_PIPELINE(pipeline)), NULL, NULL);
  g_source_remove(bus_callback_id);
  gst_object_unref(pipeline);

  return fingerprint_;
}

void Fingerprinter::NewPadCallback(GstElement*, GstPad* pad, gboolean, gpointer data) {
  Fingerprinter* instance = reinterpret_cast<Fingerprinter*>(data);
  GstPad* const audiopad = gst_element_get_pad(instance->convert_element_, "sink");

  if (GST_PAD_IS_LINKED(audiopad)) {
    qDebug() << "audiopad is already linked. Unlinking old pad.";
    gst_pad_unlink(audiopad, GST_PAD_PEER(audiopad));
  }

  gst_pad_link(pad, audiopad);
  gst_object_unref(audiopad);
}

void Fingerprinter::ReportError(GstMessage* msg) {
  GError* error;
  gchar* debugs;

  gst_message_parse_error(msg, &error, &debugs);
  QString message = QString::fromLocal8Bit(error->message);

  g_error_free(error);
  free(debugs);

  qDebug() << "Fingerprinter: Error processing" << filename_ << ":" << message;
}

gboolean Fingerprinter::BusCallback(GstBus*, GstMessage* msg, gpointer data) {
  Fingerprinter* instance = reinterpret_cast<Fingerprinter*>(data);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR:
      instance->ReportError(msg);
      instance->event_loop_->exit();
      break;

    case GST_MESSAGE_TAG:
      instance->TagMessageReceived(msg);
      break;

    default:
      break;
  }
  return GST_BUS_DROP;
}

GstBusSyncReply Fingerprinter::BusCallbackSync(GstBus*, GstMessage* msg, gpointer data) {
  Fingerprinter* instance = reinterpret_cast<Fingerprinter*>(data);

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      instance->event_loop_->exit();
      break;

    case GST_MESSAGE_ERROR:
      instance->ReportError(msg);
      instance->event_loop_->exit();
      break;

    case GST_MESSAGE_TAG:
      instance->TagMessageReceived(msg);
      break;

    default:
      break;
  }
  return GST_BUS_PASS;
}

void Fingerprinter::TagMessageReceived(GstMessage* message) {
  GstTagList* taglist = NULL;
  gst_message_parse_tag(message, &taglist);

  gchar* data = NULL;
  bool success = gst_tag_list_get_string(taglist, "ofa-fingerprint", &data);

  if (success && data) {
    fingerprint_ = QString::fromUtf8(data);
    g_free(data);
  }

  gst_tag_list_free(taglist);
}
