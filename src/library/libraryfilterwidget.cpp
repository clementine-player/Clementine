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

#include "libraryfilterwidget.h"
#include "librarymodel.h"
#include "libraryquery.h"
#include "groupbydialog.h"
#include "ui_libraryfilterwidget.h"
#include "core/song.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"

#include <QActionGroup>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QRegExp>
#include <QSettings>
#include <QSignalMapper>
#include <QTimer>

LibraryFilterWidget::LibraryFilterWidget(QWidget* parent)
    : QWidget(parent),
      ui_(new Ui_LibraryFilterWidget),
      model_(nullptr),
      group_by_dialog_(new GroupByDialog),
      filter_delay_(new QTimer(this)),
      filter_applies_to_model_(true),
      delay_behaviour_(DelayedOnLargeLibraries) {
  ui_->setupUi(this);

  // Add the available fields to the tooltip here instead of the ui
  // file to prevent that they get translated by mistake.
  QString available_fields = (Song::kFtsColumns + Song::kIntColumns +
                              Song::kFloatColumns + Song::kDateColumns)
                                 .join(", ")
                                 .replace(QRegExp("\\bfts"), "");
  ui_->filter->setToolTip(ui_->filter->toolTip().arg(available_fields));

  connect(ui_->filter, SIGNAL(returnPressed()), SIGNAL(ReturnPressed()));
  connect(filter_delay_, SIGNAL(timeout()), SLOT(FilterDelayTimeout()));

  filter_delay_->setInterval(kFilterDelay);
  filter_delay_->setSingleShot(true);

  // Icons
  ui_->options->setIcon(IconLoader::Load("configure", IconLoader::Base));

  // Filter by age
  QActionGroup* filter_age_group = new QActionGroup(this);
  filter_age_group->addAction(ui_->filter_age_all);
  filter_age_group->addAction(ui_->filter_age_today);
  filter_age_group->addAction(ui_->filter_age_week);
  filter_age_group->addAction(ui_->filter_age_month);
  filter_age_group->addAction(ui_->filter_age_three_months);
  filter_age_group->addAction(ui_->filter_age_year);

  filter_age_menu_ = new QMenu(tr("Show"), this);
  filter_age_menu_->addActions(filter_age_group->actions());

  filter_age_mapper_ = new QSignalMapper(this);
  filter_age_mapper_->setMapping(ui_->filter_age_all, -1);
  filter_age_mapper_->setMapping(ui_->filter_age_today, 60 * 60 * 24);
  filter_age_mapper_->setMapping(ui_->filter_age_week, 60 * 60 * 24 * 7);
  filter_age_mapper_->setMapping(ui_->filter_age_month, 60 * 60 * 24 * 30);
  filter_age_mapper_->setMapping(ui_->filter_age_three_months,
                                 60 * 60 * 24 * 30 * 3);
  filter_age_mapper_->setMapping(ui_->filter_age_year, 60 * 60 * 24 * 365);

  connect(ui_->filter_age_all, SIGNAL(triggered()), filter_age_mapper_,
          SLOT(map()));
  connect(ui_->filter_age_today, SIGNAL(triggered()), filter_age_mapper_,
          SLOT(map()));
  connect(ui_->filter_age_week, SIGNAL(triggered()), filter_age_mapper_,
          SLOT(map()));
  connect(ui_->filter_age_month, SIGNAL(triggered()), filter_age_mapper_,
          SLOT(map()));
  connect(ui_->filter_age_three_months, SIGNAL(triggered()), filter_age_mapper_,
          SLOT(map()));
  connect(ui_->filter_age_year, SIGNAL(triggered()), filter_age_mapper_,
          SLOT(map()));

  // "Group by ..."
  group_by_group_ = CreateGroupByActions(this);

  group_by_menu_ = new QMenu(tr("Group by"), this);
  group_by_menu_->addActions(group_by_group_->actions());

  connect(group_by_group_, SIGNAL(triggered(QAction*)),
          SLOT(GroupByClicked(QAction*)));

  connect(ui_->save_grouping, SIGNAL(triggered()), this, SLOT(SaveGroupBy()));
  connect(ui_->manage_groupings, SIGNAL(triggered()), this,
          SLOT(ShowGroupingManager()));

  // Library config menu
  library_menu_ = new QMenu(tr("Display options"), this);
  library_menu_->setIcon(ui_->options->icon());
  library_menu_->addMenu(filter_age_menu_);
  library_menu_->addMenu(group_by_menu_);
  library_menu_->addAction(ui_->save_grouping);
  library_menu_->addAction(ui_->manage_groupings);
  library_menu_->addSeparator();
  ui_->options->setMenu(library_menu_);

  connect(ui_->filter, SIGNAL(textChanged(QString)),
          SLOT(FilterTextChanged(QString)));
}

LibraryFilterWidget::~LibraryFilterWidget() { delete ui_; }

void LibraryFilterWidget::UpdateGroupByActions() {
  if (group_by_group_) {
    disconnect(group_by_group_, 0, 0, 0);
    delete group_by_group_;
  }

  group_by_group_ = CreateGroupByActions(this);
  group_by_menu_->clear();
  group_by_menu_->addActions(group_by_group_->actions());
  connect(group_by_group_, SIGNAL(triggered(QAction*)),
          SLOT(GroupByClicked(QAction*)));
  if (model_) {
    CheckCurrentGrouping(model_->GetGroupBy());
  }
}

QActionGroup* LibraryFilterWidget::CreateGroupByActions(QObject* parent) {
  QActionGroup* ret = new QActionGroup(parent);
  ret->addAction(CreateGroupByAction(
      tr("Group by Artist"), parent,
      LibraryModel::Grouping(LibraryModel::GroupBy_Artist)));
  ret->addAction(
      CreateGroupByAction(tr("Group by Artist/Album"), parent,
                          LibraryModel::Grouping(LibraryModel::GroupBy_Artist,
                                                 LibraryModel::GroupBy_Album)));
  ret->addAction(
      CreateGroupByAction(tr("Group by Album artist/Album"), parent,
                          LibraryModel::Grouping(LibraryModel::GroupBy_AlbumArtist,
                                                 LibraryModel::GroupBy_Album)));
  ret->addAction(CreateGroupByAction(
      tr("Group by Artist/Year - Album"), parent,
      LibraryModel::Grouping(LibraryModel::GroupBy_Artist,
                             LibraryModel::GroupBy_YearAlbum)));
  ret->addAction(
      CreateGroupByAction(tr("Group by Album"), parent,
                          LibraryModel::Grouping(LibraryModel::GroupBy_Album)));
  ret->addAction(
      CreateGroupByAction(tr("Group by Genre/Album"), parent,
                          LibraryModel::Grouping(LibraryModel::GroupBy_Genre,
                                                 LibraryModel::GroupBy_Album)));
  ret->addAction(
      CreateGroupByAction(tr("Group by Genre/Artist/Album"), parent,
                          LibraryModel::Grouping(LibraryModel::GroupBy_Genre,
                                                 LibraryModel::GroupBy_Artist,
                                                 LibraryModel::GroupBy_Album)));

  QAction* sep1 = new QAction(parent);
  sep1->setSeparator(true);
  ret->addAction(sep1);

  // read saved groupings
  QSettings s;
  s.beginGroup(LibraryModel::kSavedGroupingsSettingsGroup);
  QStringList saved = s.childKeys();
  for (int i = 0; i < saved.size(); ++i) {
    QByteArray bytes = s.value(saved.at(i)).toByteArray();
    QDataStream ds(&bytes, QIODevice::ReadOnly);
    LibraryModel::Grouping g;
    ds >> g;
    ret->addAction(CreateGroupByAction(saved.at(i), parent, g));
  }

  QAction* sep2 = new QAction(parent);
  sep2->setSeparator(true);
  ret->addAction(sep2);

  ret->addAction(CreateGroupByAction(tr("Advanced grouping..."), parent,
                                     LibraryModel::Grouping()));

  return ret;
}

QAction* LibraryFilterWidget::CreateGroupByAction(
    const QString& text, QObject* parent,
    const LibraryModel::Grouping& grouping) {
  QAction* ret = new QAction(text, parent);
  ret->setCheckable(true);

  if (grouping.first != LibraryModel::GroupBy_None) {
    ret->setProperty("group_by", QVariant::fromValue(grouping));
  }

  return ret;
}

void LibraryFilterWidget::SaveGroupBy() {
  QString text =
      QInputDialog::getText(this, tr("Grouping Name"), tr("Grouping name:"));
  if (!text.isEmpty() && model_) {
    model_->SaveGrouping(text);
    UpdateGroupByActions();
  }
}

void LibraryFilterWidget::ShowGroupingManager() {
  if (!groupings_manager_) {
    groupings_manager_.reset(new SavedGroupingManager);
  }
  groupings_manager_->SetFilter(this);
  groupings_manager_->UpdateModel();
  groupings_manager_->show();
}

void LibraryFilterWidget::FocusOnFilter(QKeyEvent* event) {
  ui_->filter->setFocus();
  QApplication::sendEvent(ui_->filter, event);
}

void LibraryFilterWidget::SetLibraryModel(LibraryModel* model) {
  if (model_) {
    disconnect(model_, 0, this, 0);
    disconnect(model_, 0, group_by_dialog_.get(), 0);
    disconnect(group_by_dialog_.get(), 0, model_, 0);
    disconnect(filter_age_mapper_, 0, model_, 0);
  }

  model_ = model;

  // Connect signals
  connect(model_, SIGNAL(GroupingChanged(LibraryModel::Grouping)),
          group_by_dialog_.get(),
          SLOT(LibraryGroupingChanged(LibraryModel::Grouping)));
  connect(model_, SIGNAL(GroupingChanged(LibraryModel::Grouping)),
          SLOT(GroupingChanged(LibraryModel::Grouping)));
  connect(group_by_dialog_.get(), SIGNAL(Accepted(LibraryModel::Grouping)),
          model_, SLOT(SetGroupBy(LibraryModel::Grouping)));
  connect(filter_age_mapper_, SIGNAL(mapped(int)), model_,
          SLOT(SetFilterAge(int)));

  // Load settings
  if (!settings_group_.isEmpty()) {
    QSettings s;
    s.beginGroup(settings_group_);
    model_->SetGroupBy(LibraryModel::Grouping(
        LibraryModel::GroupBy(
            s.value("group_by1", int(LibraryModel::GroupBy_AlbumArtist))
                .toInt()),
        LibraryModel::GroupBy(
            s.value("group_by2", int(LibraryModel::GroupBy_Album)).toInt()),
        LibraryModel::GroupBy(
            s.value("group_by3", int(LibraryModel::GroupBy_None)).toInt())));
  }
}

void LibraryFilterWidget::GroupByClicked(QAction* action) {
  if (action->property("group_by").isNull()) {
    group_by_dialog_->show();
    return;
  }

  LibraryModel::Grouping g =
      action->property("group_by").value<LibraryModel::Grouping>();
  model_->SetGroupBy(g);
}

void LibraryFilterWidget::GroupingChanged(const LibraryModel::Grouping& g) {
  if (!settings_group_.isEmpty()) {
    // Save the settings
    QSettings s;
    s.beginGroup(settings_group_);
    s.setValue("group_by1", int(g[0]));
    s.setValue("group_by2", int(g[1]));
    s.setValue("group_by3", int(g[2]));
  }

  // Now make sure the correct action is checked
  CheckCurrentGrouping(g);
}

void LibraryFilterWidget::CheckCurrentGrouping(
    const LibraryModel::Grouping& g) {
  for (QAction* action : group_by_group_->actions()) {
    if (action->property("group_by").isNull()) continue;

    if (g == action->property("group_by").value<LibraryModel::Grouping>()) {
      action->setChecked(true);
      return;
    }
  }

  // Check the advanced action
  group_by_group_->actions().last()->setChecked(true);
}

void LibraryFilterWidget::SetFilterHint(const QString& hint) {
  ui_->filter->setPlaceholderText(hint);
}

void LibraryFilterWidget::SetQueryMode(QueryOptions::QueryMode query_mode) {
  ui_->filter->clear();
  ui_->filter->setEnabled(query_mode == QueryOptions::QueryMode_All);

  model_->SetFilterQueryMode(query_mode);
}

void LibraryFilterWidget::ShowInLibrary(const QString& search) {
  ui_->filter->setText(search);
}

void LibraryFilterWidget::SetAgeFilterEnabled(bool enabled) {
  filter_age_menu_->setEnabled(enabled);
}

void LibraryFilterWidget::SetGroupByEnabled(bool enabled) {
  group_by_menu_->setEnabled(enabled);
}

void LibraryFilterWidget::AddMenuAction(QAction* action) {
  library_menu_->addAction(action);
}

void LibraryFilterWidget::keyReleaseEvent(QKeyEvent* e) {
  switch (e->key()) {
    case Qt::Key_Up:
      emit UpPressed();
      e->accept();
      break;

    case Qt::Key_Down:
      emit DownPressed();
      e->accept();
      break;

    case Qt::Key_Escape:
      ui_->filter->clear();
      e->accept();
      break;
  }

  QWidget::keyReleaseEvent(e);
}

void LibraryFilterWidget::FilterTextChanged(const QString& text) {
  // Searching with one or two characters can be very expensive on the database
  // even with FTS, so if there are a large number of songs in the database
  // introduce a small delay before actually filtering the model, so if the
  // user is typing the first few characters of something it will be quicker.
  const bool delay =
      (delay_behaviour_ == AlwaysDelayed) ||
      (delay_behaviour_ == DelayedOnLargeLibraries && !text.isEmpty() &&
       text.length() < 3 && model_->total_song_count() >= 100000);

  if (delay) {
    filter_delay_->start();
  } else {
    filter_delay_->stop();
    FilterDelayTimeout();
  }
}

void LibraryFilterWidget::FilterDelayTimeout() {
  emit Filter(ui_->filter->text());
  if (filter_applies_to_model_) {
    model_->SetFilterText(ui_->filter->text());
  }
}
