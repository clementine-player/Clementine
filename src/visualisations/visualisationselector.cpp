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

#include "config.h"
#include "projectmpresetmodel.h"
#include "projectmvisualisation.h"
#include "visualisationselector.h"
#include "ui_visualisationselector.h"

#include <QPushButton>

#ifdef USE_SYSTEM_PROJECTM
#  include <libprojectM/projectM.hpp>
#else
#  include "projectM.hpp"
#endif

VisualisationSelector::VisualisationSelector(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_VisualisationSelector),
    vis_(NULL)
{
  ui_->setupUi(this);

  select_all_ =
      ui_->buttonBox->addButton(tr("Select All"), QDialogButtonBox::ActionRole);
  select_none_ =
      ui_->buttonBox->addButton(tr("Select None"), QDialogButtonBox::ActionRole);
  connect(select_all_, SIGNAL(clicked()), SLOT(SelectAll()));
  connect(select_none_, SIGNAL(clicked()), SLOT(SelectNone()));
  select_all_->setEnabled(false);
  select_none_->setEnabled(false);

  connect(ui_->mode, SIGNAL(currentIndexChanged(int)), SLOT(ModeChanged(int)));
}

VisualisationSelector::~VisualisationSelector() {
  delete ui_;
}

void VisualisationSelector::showEvent(QShowEvent *) {
  if (!ui_->list->model()) {
    ui_->delay->setValue(vis_->duration());
    ui_->list->setModel(vis_->preset_model());
    connect(ui_->list->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            vis_->preset_model(), SLOT(SetImmediatePreset(QModelIndex)));
    connect(ui_->delay, SIGNAL(valueChanged(int)), vis_, SLOT(SetDuration(int)));

    ui_->mode->setCurrentIndex(vis_->mode());
  }

  vis_->Lock(true);
}

void VisualisationSelector::hideEvent(QHideEvent *) {
  vis_->Lock(false);
}

void VisualisationSelector::ModeChanged(int mode) {
  bool enabled = mode == 1;
  ui_->list->setEnabled(enabled);
  select_all_->setEnabled(enabled);
  select_none_->setEnabled(enabled);

  vis_->SetMode(ProjectMVisualisation::Mode(mode));
}

void VisualisationSelector::SelectAll() {
  vis_->preset_model()->SelectAll();
}

void VisualisationSelector::SelectNone() {
  vis_->preset_model()->SelectNone();
}
