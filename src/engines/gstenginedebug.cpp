/* This file is part of Clementine.
   Copyright 2020, Jim Broadus <jbroadus@gmail.com>

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

#include "gstenginedebug.h"

#include "core/logging.h"
#include "gstengine.h"
#include "gstenginepipeline.h"

GstEngineDebug::GstEngineDebug(GstEngine* engine, QWidget* parent)
    : QWidget(parent), engine_(engine) {
  ui_.setupUi(this);
  connect(ui_.dump_graph_button, SIGNAL(clicked()), SLOT(DumpGraph()));
}

void GstEngineDebug::DumpGraph() {
  std::shared_ptr<GstEnginePipeline> pipeline = engine_->GetCurrentPipeline();
  if (pipeline) {
    pipeline->DumpGraph();
  }
}
