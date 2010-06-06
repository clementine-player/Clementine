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

#include "visualisationcontainer.h"
#include "projectmvisualisation.h"
#include "engines/gstengine.h"

#include <QHBoxLayout>
#include <QSettings>

const char* VisualisationContainer::kSettingsGroup = "Visualisations";
const int VisualisationContainer::kDefaultWidth = 828;
const int VisualisationContainer::kDefaultHeight = 512;

VisualisationContainer::VisualisationContainer(QWidget *parent)
  : QWidget(parent),
    engine_(NULL),
    vis_(new ProjectMVisualisation(this))
{
  setWindowTitle(tr("Clementine Visualisation"));

  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  layout->addWidget(vis_);
  setLayout(layout);

  QSettings s;
  s.beginGroup(kSettingsGroup);
  if (!restoreGeometry(s.value("geometry").toByteArray())) {
    resize(kDefaultWidth, kDefaultHeight);
  }
}

void VisualisationContainer::SetEngine(GstEngine* engine) {
  engine_ = engine;
  if (isVisible())
    engine_->AddBufferConsumer(vis_);
}

void VisualisationContainer::showEvent(QShowEvent*) {
  if (engine_)
    engine_->AddBufferConsumer(vis_);
}

void VisualisationContainer::hideEvent(QHideEvent*) {
  if (engine_)
    engine_->RemoveBufferConsumer(vis_);
}

void VisualisationContainer::resizeEvent(QResizeEvent *) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("geometry", saveGeometry());
}
