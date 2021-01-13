/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

#include "gstpipelinebase.h"

#include "core/logging.h"

std::atomic<int> GstPipelineBase::sId(1);

GstPipelineBase::GstPipelineBase(const QString& type)
    : QObject(nullptr), pipeline_(nullptr), type_(type), id_(sId++) {}

GstPipelineBase::~GstPipelineBase() {
  if (pipeline_) {
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline_));
  }
}

bool GstPipelineBase::Init() {
  QString name = QString("%1-pipeline-%2").arg(type_).arg(id_);
  pipeline_ = gst_pipeline_new(name.toUtf8().constData());
  return pipeline_ != nullptr;
}

void GstPipelineBase::DumpGraph() {
#ifdef GST_DISABLE_GST_DEBUG
  qLog(Debug) << "Cannot dump graph. gstreamer debug is not enabled.";
#else
  if (pipeline_) {
    qLog(Debug) << "Dumping pipeline graph";
    GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(pipeline_),
                                      GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
  }
#endif
}
