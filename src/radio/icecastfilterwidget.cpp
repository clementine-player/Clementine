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

#include "icecastfilterwidget.h"
#include "ui_icecastfilterwidget.h"
#include "ui/iconloader.h"
#include "widgets/maclineedit.h"

#include <QMenu>

IcecastFilterWidget::IcecastFilterWidget(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_IcecastFilterWidget)
{
  ui_->setupUi(this);
  connect(ui_->clear, SIGNAL(clicked()), SLOT(ClearFilter()));

  // Icons
  ui_->clear->setIcon(IconLoader::Load("edit-clear-locationbar-ltr"));
  ui_->options->setIcon(IconLoader::Load("configure"));

  QMenu* options_menu = new QMenu(this);
  options_menu->addAction(ui_->action_sort_genre_popularity);
  options_menu->addAction(ui_->action_sort_genre_alphabetically);
  options_menu->addAction(ui_->action_sort_station);

  ui_->options->setMenu(options_menu);

#ifdef Q_OS_DARWIN
  delete ui_->filter;
  MacLineEdit* lineedit = new MacLineEdit(this);
  ui_->horizontalLayout->insertWidget(1, lineedit);
  filter_ = lineedit;
  ui_->clear->setHidden(true);
#else
  filter_ = ui_->filter;
#endif
}

IcecastFilterWidget::~IcecastFilterWidget() {
  delete ui_;
}

void IcecastFilterWidget::SetIcecastModel(IcecastModel* model) {
  model_ = model;
}

void IcecastFilterWidget::ClearFilter() {
  filter_->clear();
  filter_->setFocus();
}
