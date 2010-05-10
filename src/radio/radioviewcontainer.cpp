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

#include "radioviewcontainer.h"
#include "radiomodel.h"
#include "radioservice.h"
#include "ui_radioviewcontainer.h"
#include "core/mergedproxymodel.h"

#include <QtDebug>
#include <QTimeLine>

RadioViewContainer::RadioViewContainer(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_RadioViewContainer),
    model_(NULL),
    current_service_(NULL),
    filter_visible_(false),
    filter_animation_(new QTimeLine(500, this))
{
  ui_->setupUi(this);

  connect(ui_->tree, SIGNAL(collapsed(QModelIndex)), SLOT(Collapsed(QModelIndex)));
  connect(ui_->tree, SIGNAL(expanded(QModelIndex)), SLOT(Expanded(QModelIndex)));

  filter_animation_->setFrameRange(0, ui_->filter->sizeHint().height());
  connect(filter_animation_, SIGNAL(frameChanged(int)), SLOT(SetFilterHeight(int)));

  ui_->filter->setMaximumHeight(0);
}

RadioViewContainer::~RadioViewContainer() {
  delete ui_;
}

RadioView* RadioViewContainer::tree() const {
  return ui_->tree;
}

void RadioViewContainer::SetModel(RadioModel* model) {
  model_ = model;

  ui_->tree->setModel(model->merged_model());

  connect(ui_->tree->selectionModel(),
          SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          SLOT(CurrentIndexChanged(QModelIndex)));
}

void RadioViewContainer::ServiceChanged(const QModelIndex& index, bool changed_away) {
  RadioItem* item = model_->IndexToItem(
      model_->merged_model()->FindSourceParent(index));
  if (!item)
    return;

  if (changed_away) {
    SetFilterVisible(false);
  } else {
    RadioService* service = item->service;
    if (!service || service == current_service_)
      return;
    current_service_ = service;

    SetFilterVisible(service->SetupLibraryFilter(ui_->filter));
  }
}

void RadioViewContainer::CurrentIndexChanged(const QModelIndex& index) {
  ServiceChanged(index);
}

void RadioViewContainer::Collapsed(const QModelIndex& index) {
  if (model_->merged_model()->mapToSource(index).model() == model_) {
    SetFilterVisible(false);
    current_service_ = NULL;
  }
}

void RadioViewContainer::Expanded(const QModelIndex& index) {
  ServiceChanged(index);
}

void RadioViewContainer::SetFilterVisible(bool visible) {
  if (filter_visible_ == visible)
    return;
  filter_visible_ = visible;

  filter_animation_->setDirection(visible ? QTimeLine::Forward : QTimeLine::Backward);
  filter_animation_->start();
}

void RadioViewContainer::SetFilterHeight(int height) {
  ui_->filter->setMaximumHeight(height);
}
