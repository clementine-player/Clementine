/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "internetservice.h"
#include "searchboxwidget.h"
#include "ui_searchboxwidget.h"
#include "ui/iconloader.h"
#include "widgets/didyoumean.h"

#include <QKeyEvent>
#include <QMenu>

SearchBoxWidget::SearchBoxWidget(InternetService* service)
    : service_(service),
      ui_(new Ui_SearchBoxWidget),
      menu_(new QMenu(tr("Display options"), this)) {
  ui_->setupUi(this);

  // Icons
  ui_->options->setIcon(IconLoader::Load("configure"));

  // Options menu
  menu_->setIcon(ui_->options->icon());
  ui_->options->setMenu(menu_);

  menu_->addAction(IconLoader::Load("configure"),
                   tr("Configure %1...").arg(service_->name()), service_,
                   SLOT(ShowConfig()));

  ui_->filter->setPlaceholderText(
      QString("Search on %1").arg(service_->name()));
  connect(ui_->filter, SIGNAL(textChanged(QString)),
          SIGNAL(TextChanged(QString)));

  did_you_mean_ = new DidYouMean(ui_->filter, this);
  connect(did_you_mean_, SIGNAL(Accepted(QString)), ui_->filter,
          SLOT(setText(QString)));
}

SearchBoxWidget::~SearchBoxWidget() { delete ui_; }

void SearchBoxWidget::FocusOnFilter(QKeyEvent* event) {
  ui_->filter->setFocus(Qt::OtherFocusReason);
  QApplication::sendEvent(ui_->filter, event);
}

void SearchBoxWidget::keyReleaseEvent(QKeyEvent* e) {
  switch (e->key()) {
    case Qt::Key_Escape:
      ui_->filter->clear();
      e->accept();
      break;
  }

  QWidget::keyReleaseEvent(e);
}
