/* This file is part of Clementine.
   Copyright 2010-2011, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "icecastmodel.h"
#include "icecastfilterwidget.h"
#include "ui_icecastfilterwidget.h"
#include "ui/iconloader.h"

#include <QKeyEvent>
#include <QMenu>
#include <QSettings>
#include <QSignalMapper>

const char* IcecastFilterWidget::kSettingsGroup = "Icecast";

IcecastFilterWidget::IcecastFilterWidget(QWidget* parent)
    : QWidget(parent),
      ui_(new Ui_IcecastFilterWidget),
      menu_(new QMenu(tr("Display options"), this)),
      sort_mode_mapper_(new QSignalMapper(this)) {
  ui_->setupUi(this);

  // Icons
  ui_->options->setIcon(IconLoader::Load("configure"));

  // Options actions
  QActionGroup* group = new QActionGroup(this);
  AddAction(group, ui_->action_sort_genre_popularity,
            IcecastModel::SortMode_GenreByPopularity);
  AddAction(group, ui_->action_sort_genre_alphabetically,
            IcecastModel::SortMode_GenreAlphabetical);
  AddAction(group, ui_->action_sort_station,
            IcecastModel::SortMode_StationAlphabetical);

  // Options menu
  menu_->setIcon(ui_->options->icon());
  menu_->addActions(group->actions());
  ui_->options->setMenu(menu_);

  connect(sort_mode_mapper_, SIGNAL(mapped(int)), SLOT(SortModeChanged(int)));
}

void IcecastFilterWidget::AddAction(QActionGroup* group, QAction* action,
                                    IcecastModel::SortMode mode) {
  group->addAction(action);
  sort_mode_mapper_->setMapping(action, mode);
  connect(action, SIGNAL(triggered()), sort_mode_mapper_, SLOT(map()));
}

IcecastFilterWidget::~IcecastFilterWidget() { delete ui_; }

void IcecastFilterWidget::FocusOnFilter(QKeyEvent* event) {
  ui_->filter->setFocus(Qt::OtherFocusReason);
  QApplication::sendEvent(ui_->filter, event);
}

void IcecastFilterWidget::SetIcecastModel(IcecastModel* model) {
  model_ = model;
  connect(ui_->filter, SIGNAL(textChanged(QString)), model_,
          SLOT(SetFilterText(QString)));

  // Load settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  switch (
      s.value("sort_by", IcecastModel::SortMode_GenreByPopularity).toInt()) {
    case IcecastModel::SortMode_GenreByPopularity:
      ui_->action_sort_genre_popularity->trigger();
      break;

    case IcecastModel::SortMode_GenreAlphabetical:
      ui_->action_sort_genre_alphabetically->trigger();
      break;

    case IcecastModel::SortMode_StationAlphabetical:
      ui_->action_sort_station->trigger();
      break;
  }
}

void IcecastFilterWidget::SortModeChanged(int mode) {
  model_->SetSortMode(IcecastModel::SortMode(mode));

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("sort_by", mode);
}
