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
#include "mergedproxymodel.h"

#include <QtDebug>
#include <QTimeLine>

RadioViewContainer::RadioViewContainer(QWidget *parent)
  : QWidget(parent),
    model_(NULL),
    current_service_(NULL),
    filter_visible_(false),
    filter_animation_(new QTimeLine(500, this))
{
  ui_.setupUi(this);

  filter_animation_->setFrameRange(0, ui_.filter->sizeHint().height());
  connect(filter_animation_, SIGNAL(frameChanged(int)), SLOT(SetFilterHeight(int)));

  ui_.filter->setMaximumHeight(0);
}

void RadioViewContainer::SetModel(RadioModel* model) {
  model_ = model;

  ui_.tree->setModel(model->merged_model());

  connect(ui_.tree->selectionModel(),
          SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          SLOT(CurrentIndexChanged(QModelIndex)));
}

void RadioViewContainer::CurrentIndexChanged(const QModelIndex& index) {
  RadioItem* item = model_->IndexToItem(
      model_->merged_model()->FindSourceParent(index));
  if (!item)
    return;

  RadioService* service = item->service;
  if (!service || service == current_service_)
    return;

  qDebug() << service->name();

  SetFilterVisible(service->SetupLibraryFilter(ui_.filter));
}

void RadioViewContainer::SetFilterVisible(bool visible) {
  if (filter_visible_ == visible)
    return;
  filter_visible_ = visible;

  filter_animation_->setDirection(visible ? QTimeLine::Forward : QTimeLine::Backward);
  filter_animation_->start();
}

void RadioViewContainer::SetFilterHeight(int height) {
  ui_.filter->setMaximumHeight(height);
}
