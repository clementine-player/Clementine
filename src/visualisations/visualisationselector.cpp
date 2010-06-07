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

#include "projectmpresetmodel.h"
#include "projectmvisualisation.h"
#include "visualisationselector.h"
#include "ui_visualisationselector.h"

#include <QPushButton>

VisualisationSelector::VisualisationSelector(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_VisualisationSelector),
    vis_(NULL),
    preset_model_(NULL)
{
  ui_->setupUi(this);

  QPushButton* select_all =
      ui_->buttonBox->addButton(tr("Select All"), QDialogButtonBox::ActionRole);
  QPushButton* select_none =
      ui_->buttonBox->addButton(tr("Select None"), QDialogButtonBox::ActionRole);
  connect(select_all, SIGNAL(clicked()), SLOT(SelectAll()));
  connect(select_none, SIGNAL(clicked()), SLOT(SelectNone()));

  connect(ui_->mode, SIGNAL(currentIndexChanged(int)), SLOT(ModeChanged(int)));
}

VisualisationSelector::~VisualisationSelector() {
  delete ui_;
}

void VisualisationSelector::showEvent(QShowEvent *) {
  if (!ui_->list->model()) {
    preset_model_ = new ProjectMPresetModel(vis_, this);
    ui_->list->setModel(preset_model_);
    connect(ui_->list->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            preset_model_, SLOT(SetImmediatePreset(QModelIndex)));

    ui_->mode->setCurrentIndex(vis_->mode());
  }
}

void VisualisationSelector::ModeChanged(int mode) {
  ui_->list->setEnabled(mode == 1);
  vis_->set_mode(mode);
}

void VisualisationSelector::SelectAll() {
  preset_model_->SelectAll();
}

void VisualisationSelector::SelectNone() {
  preset_model_->SelectNone();
}
