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

#include "radioviewcontainer.h"
#include "radiomodel.h"
#include "radioservice.h"
#include "ui_radioviewcontainer.h"
#include "core/mergedproxymodel.h"

#include <QtDebug>
#include <QTimeLine>

const int RadioViewContainer::kAnimationDuration = 500;

RadioViewContainer::RadioViewContainer(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_RadioViewContainer),
    model_(NULL),
    current_service_(NULL),
    current_header_(NULL)
{
  ui_->setupUi(this);

  connect(ui_->tree, SIGNAL(collapsed(QModelIndex)), SLOT(Collapsed(QModelIndex)));
  connect(ui_->tree, SIGNAL(expanded(QModelIndex)), SLOT(Expanded(QModelIndex)));
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

void RadioViewContainer::ServiceChanged(const QModelIndex& index) {
  RadioItem* item = model_->IndexToItem(
      model_->merged_model()->FindSourceParent(index));
  if (!item)
    return;

  RadioService* service = item->service;
  if (!service || service == current_service_)
    return;
  current_service_ = service;

  QWidget* header = service->HeaderWidget();
  if (header && !headers_.contains(header)) {
    header->setParent(ui_->header_container);
    header->setMaximumHeight(0);
    ui_->header_container->layout()->addWidget(header);
    header->show();

    HeaderData d;
    d.visible_ = false;
    d.animation_ = new QTimeLine(kAnimationDuration, this);
    d.animation_->setFrameRange(0, header->sizeHint().height());
    connect(d.animation_, SIGNAL(frameChanged(int)), SLOT(SetHeaderHeight(int)));

    headers_.insert(header, d);
  }

  SetHeaderVisible(current_header_, false);
  current_header_ = header;
  SetHeaderVisible(current_header_, true);
}

void RadioViewContainer::CurrentIndexChanged(const QModelIndex& index) {
  ServiceChanged(index);
}

void RadioViewContainer::Collapsed(const QModelIndex& index) {
  if (model_->merged_model()->mapToSource(index).model() == model_) {
    SetHeaderVisible(current_header_, false);
    current_service_ = NULL;
    current_header_ = NULL;
  }
}

void RadioViewContainer::Expanded(const QModelIndex& index) {
  ServiceChanged(index);
}

void RadioViewContainer::SetHeaderVisible(QWidget* header, bool visible) {
  if (!header)
    return;

  HeaderData& d = headers_[header];
  if (d.visible_ == visible)
    return;
  d.visible_ = visible;

  d.animation_->setDirection(visible ? QTimeLine::Forward : QTimeLine::Backward);
  d.animation_->start();
}

void RadioViewContainer::SetHeaderHeight(int height) {
  QTimeLine* animation = qobject_cast<QTimeLine*>(sender());
  QWidget* header = NULL;
  foreach (QWidget* h, headers_.keys()) {
    if (headers_[h].animation_ == animation) {
      header = h;
      break;
    }
  }

  if (header)
    header->setMaximumHeight(height);
}
