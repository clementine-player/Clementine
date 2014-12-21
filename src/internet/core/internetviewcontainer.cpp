/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "internetviewcontainer.h"
#include "ui_internetviewcontainer.h"

#include <QMetaMethod>
#include <QTimeLine>
#include <QtDebug>

#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"
#include "core/application.h"
#include "core/mergedproxymodel.h"
#include "globalsearch/globalsearch.h"

const int InternetViewContainer::kAnimationDuration = 500;

InternetViewContainer::InternetViewContainer(QWidget* parent)
    : QWidget(parent),
      ui_(new Ui_InternetViewContainer),
      app_(nullptr),
      current_service_(nullptr),
      current_header_(nullptr) {
  ui_->setupUi(this);

  connect(ui_->tree, SIGNAL(collapsed(QModelIndex)),
          SLOT(Collapsed(QModelIndex)));
  connect(ui_->tree, SIGNAL(expanded(QModelIndex)),
          SLOT(Expanded(QModelIndex)));
  connect(ui_->tree, SIGNAL(FocusOnFilterSignal(QKeyEvent*)),
          SLOT(FocusOnFilter(QKeyEvent*)));
}

InternetViewContainer::~InternetViewContainer() { delete ui_; }

InternetView* InternetViewContainer::tree() const { return ui_->tree; }

void InternetViewContainer::SetApplication(Application* app) {
  app_ = app;

  ui_->tree->setModel(app_->internet_model()->merged_model());

  connect(ui_->tree->selectionModel(),
          SIGNAL(currentChanged(QModelIndex, QModelIndex)),
          SLOT(CurrentIndexChanged(QModelIndex)));
}

void InternetViewContainer::ServiceChanged(const QModelIndex& index) {
  InternetService* service =
      index.data(InternetModel::Role_Service).value<InternetService*>();
  if (!service || service == current_service_) return;
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
    connect(d.animation_, SIGNAL(frameChanged(int)),
            SLOT(SetHeaderHeight(int)));

    headers_.insert(header, d);
  }

  SetHeaderVisible(current_header_, false);
  current_header_ = header;
  SetHeaderVisible(current_header_, true);
}

void InternetViewContainer::CurrentIndexChanged(const QModelIndex& index) {
  ServiceChanged(index);
}

void InternetViewContainer::Collapsed(const QModelIndex& index) {
  if (app_->internet_model()->merged_model()->mapToSource(index).model() ==
          app_->internet_model() &&
      index.data(InternetModel::Role_Type) == InternetModel::Type_Service) {
    SetHeaderVisible(current_header_, false);
    current_service_ = nullptr;
    current_header_ = nullptr;
  }
}

void InternetViewContainer::Expanded(const QModelIndex& index) {
  ServiceChanged(index);
}

void InternetViewContainer::SetHeaderVisible(QWidget* header, bool visible) {
  if (!header) return;

  HeaderData& d = headers_[header];
  if (d.visible_ == visible) return;
  d.visible_ = visible;

  d.animation_->setDirection(visible ? QTimeLine::Forward
                                     : QTimeLine::Backward);
  d.animation_->start();
}

void InternetViewContainer::FocusOnFilter(QKeyEvent* event) {
  // Beware: magic

  if (current_header_) {
    int slot = current_header_->metaObject()->indexOfSlot(
        QMetaObject::normalizedSignature("FocusOnFilter(QKeyEvent*)"));
    if (slot != -1) {
      current_header_->metaObject()->method(slot).invoke(
          current_header_, Q_ARG(QKeyEvent*, event));
    }
  }
}

void InternetViewContainer::SetHeaderHeight(int height) {
  QTimeLine* animation = qobject_cast<QTimeLine*>(sender());
  QWidget* header = nullptr;
  for (QWidget* h : headers_.keys()) {
    if (headers_[h].animation_ == animation) {
      header = h;
      break;
    }
  }

  if (header) header->setMaximumHeight(height);
}

void InternetViewContainer::ScrollToIndex(const QModelIndex& index) {
  tree()->scrollTo(index, QTreeView::PositionAtCenter);
  tree()->setCurrentIndex(index);
  tree()->expand(index);
}
