/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "airplaysink.h"

#include <gst/base/gstbasesink.h>
#include <gst/gst.h>

#include <QtGlobal>

#include "engines/gstobjectlocker.h"

namespace AirPlaySink {

const char kFactoryName[] = "clementineairplaysink";

}  // namespace AirPlaySink

namespace {

struct ClementineAirPlaySink {
  GstBaseSink parent;
  // Guarded by the object lock. A pointer, since GObject doesn't construct
  // the struct's members.
  std::shared_ptr<AirPlaySinkTarget>* target;
};

struct ClementineAirPlaySinkClass {
  GstBaseSinkClass parent_class;
};

GType clementine_airplay_sink_get_type();
G_DEFINE_TYPE(ClementineAirPlaySink, clementine_airplay_sink,
              GST_TYPE_BASE_SINK)

ClementineAirPlaySink* Self(gpointer object) {
  return reinterpret_cast<ClementineAirPlaySink*>(object);
}

std::shared_ptr<AirPlaySinkTarget> TargetOf(GstBaseSink* sink) {
  GstObjectLocker l(sink);
  return *Self(sink)->target;
}

void Finalize(GObject* object) {
  delete Self(object)->target;
  G_OBJECT_CLASS(clementine_airplay_sink_parent_class)->finalize(object);
}

GstFlowReturn Render(GstBaseSink* sink, GstBuffer* buffer) {
  std::shared_ptr<AirPlaySinkTarget> target = TargetOf(sink);
  if (!target) return GST_FLOW_OK;
  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) return GST_FLOW_ERROR;
  target->Write(reinterpret_cast<const char*>(map.data), int(map.size));
  gst_buffer_unmap(buffer, &map);
  return GST_FLOW_OK;
}

gboolean Event(GstBaseSink* sink, GstEvent* event) {
  if (GST_EVENT_TYPE(event) == GST_EVENT_FLUSH_STOP) {
    if (std::shared_ptr<AirPlaySinkTarget> target = TargetOf(sink)) {
      target->Flushed();
    }
  }
  return GST_BASE_SINK_CLASS(clementine_airplay_sink_parent_class)
      ->event(sink, event);
}

void clementine_airplay_sink_class_init(ClementineAirPlaySinkClass* klass) {
  GObjectClass* object_class = G_OBJECT_CLASS(klass);
  object_class->finalize = Finalize;

  GstElementClass* element_class = GST_ELEMENT_CLASS(klass);
  gst_element_class_set_static_metadata(
      element_class, "Clementine AirPlay sink", "Sink/Audio",
      "Plays to an AirPlay receiver through Clementine's session",
      "Clementine");
  static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE(
      "sink", GST_PAD_SINK, GST_PAD_ALWAYS,
      GST_STATIC_CAPS("audio/x-raw, format=(string)S16LE, "
                      "rate=(int)44100, channels=(int)2, "
                      "layout=(string)interleaved"));
  gst_element_class_add_static_pad_template(element_class, &sink_template);

  GstBaseSinkClass* sink_class = GST_BASE_SINK_CLASS(klass);
  sink_class->render = Render;
  sink_class->event = Event;
}

void clementine_airplay_sink_init(ClementineAirPlaySink* sink) {
  sink->target = new std::shared_ptr<AirPlaySinkTarget>;
  // Render in real time, which paces what's sent to the receiver.
  gst_base_sink_set_sync(GST_BASE_SINK(sink), TRUE);
}

}  // namespace

void AirPlaySink::SetTarget(GstElement* sink,
                            std::shared_ptr<AirPlaySinkTarget> target) {
  if (!G_TYPE_CHECK_INSTANCE_TYPE(sink, clementine_airplay_sink_get_type())) {
    return;
  }
  GstObjectLocker l(sink);
  *Self(sink)->target = target;
}

void AirPlaySink::RegisterElement() {
  static const bool registered = gst_element_register(
      nullptr, kFactoryName, GST_RANK_NONE, clementine_airplay_sink_get_type());
  Q_UNUSED(registered);
}
