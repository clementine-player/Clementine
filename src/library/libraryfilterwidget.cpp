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

#include "libraryfilterwidget.h"
#include "librarymodel.h"
#include "groupbydialog.h"
#include "libraryconfigdialog.h"
#include "ui_libraryfilterwidget.h"
#include "ui/iconloader.h"

#include <QMenu>
#include <QActionGroup>
#include <QSignalMapper>
#include <QSettings>

LibraryFilterWidget::LibraryFilterWidget(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_LibraryFilterWidget),
    model_(NULL),
    group_by_dialog_(new GroupByDialog),
    library_config_dialog_(new LibraryConfigDialog)
{
  ui_->setupUi(this);

  // Icons
  ui_->clear->setIcon(IconLoader::Load("edit-clear-locationbar-ltr"));
  ui_->options->setIcon(IconLoader::Load("configure"));

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
  filter_age_mapper_->setMapping(ui_->filter_age_today, 60*60*24);
  filter_age_mapper_->setMapping(ui_->filter_age_week, 60*60*24*7);
  filter_age_mapper_->setMapping(ui_->filter_age_month, 60*60*24*30);
  filter_age_mapper_->setMapping(ui_->filter_age_three_months, 60*60*24*30*3);
  filter_age_mapper_->setMapping(ui_->filter_age_year, 60*60*24*365);

  connect(ui_->filter_age_all, SIGNAL(triggered()), filter_age_mapper_, SLOT(map()));
  connect(ui_->filter_age_today, SIGNAL(triggered()), filter_age_mapper_, SLOT(map()));
  connect(ui_->filter_age_week, SIGNAL(triggered()), filter_age_mapper_, SLOT(map()));
  connect(ui_->filter_age_month, SIGNAL(triggered()), filter_age_mapper_, SLOT(map()));
  connect(ui_->filter_age_three_months, SIGNAL(triggered()), filter_age_mapper_, SLOT(map()));
  connect(ui_->filter_age_year, SIGNAL(triggered()), filter_age_mapper_, SLOT(map()));
  connect(ui_->clear, SIGNAL(clicked()), SLOT(ClearFilter()));

  // "Group by ..."
  ui_->group_by_artist->setProperty("group_by", QVariant::fromValue(
      LibraryModel::Grouping(LibraryModel::GroupBy_Artist)));
  ui_->group_by_artist_album->setProperty("group_by", QVariant::fromValue(
      LibraryModel::Grouping(LibraryModel::GroupBy_Artist, LibraryModel::GroupBy_Album)));
  ui_->group_by_artist_yearalbum->setProperty("group_by", QVariant::fromValue(
      LibraryModel::Grouping(LibraryModel::GroupBy_Artist, LibraryModel::GroupBy_YearAlbum)));
  ui_->group_by_album->setProperty("group_by", QVariant::fromValue(
      LibraryModel::Grouping(LibraryModel::GroupBy_Album)));
  ui_->group_by_genre_album->setProperty("group_by", QVariant::fromValue(
      LibraryModel::Grouping(LibraryModel::GroupBy_Genre, LibraryModel::GroupBy_Album)));
  ui_->group_by_genre_artist_album->setProperty("group_by", QVariant::fromValue(
      LibraryModel::Grouping(LibraryModel::GroupBy_Genre, LibraryModel::GroupBy_Artist, LibraryModel::GroupBy_Album)));

  group_by_group_ = new QActionGroup(this);
  group_by_group_->addAction(ui_->group_by_artist);
  group_by_group_->addAction(ui_->group_by_artist_album);
  group_by_group_->addAction(ui_->group_by_artist_yearalbum);
  group_by_group_->addAction(ui_->group_by_album);
  group_by_group_->addAction(ui_->group_by_genre_album);
  group_by_group_->addAction(ui_->group_by_genre_artist_album);
  group_by_group_->addAction(ui_->group_by_advanced);

  group_by_menu_ = new QMenu(tr("Group by"), this);
  group_by_menu_->addActions(group_by_group_->actions());

  connect(group_by_group_, SIGNAL(triggered(QAction*)), SLOT(GroupByClicked(QAction*)));

  // Library config menu
  QMenu* library_menu = new QMenu(this);
  library_menu->addMenu(filter_age_menu_);
  library_menu->addMenu(group_by_menu_);
  library_menu->addSeparator();
  config_action_ = library_menu->addAction(
      tr("Configure library..."), library_config_dialog_.get(), SLOT(show()));
  ui_->options->setMenu(library_menu);
  connect(library_config_dialog_.get(), SIGNAL(accepted()), SIGNAL(LibraryConfigChanged()));
}

LibraryFilterWidget::~LibraryFilterWidget() {
  delete ui_;
}

void LibraryFilterWidget::SetLibraryModel(LibraryModel *model) {
  if (model_) {
    disconnect(model_, 0, this, 0);
    disconnect(model_, 0, group_by_dialog_.get(), 0);
    disconnect(group_by_dialog_.get(), 0, model_, 0);
    disconnect(ui_->filter, 0, model_, 0);
    disconnect(filter_age_mapper_, 0, model_, 0);
  }

  model_ = model;

  // Connect signals
  connect(model_, SIGNAL(GroupingChanged(LibraryModel::Grouping)),
          group_by_dialog_.get(), SLOT(LibraryGroupingChanged(LibraryModel::Grouping)));
  connect(model_, SIGNAL(GroupingChanged(LibraryModel::Grouping)),
          SLOT(GroupingChanged(LibraryModel::Grouping)));
  connect(group_by_dialog_.get(), SIGNAL(Accepted(LibraryModel::Grouping)),
          model_, SLOT(SetGroupBy(LibraryModel::Grouping)));
  connect(ui_->filter, SIGNAL(textChanged(QString)), model_, SLOT(SetFilterText(QString)));
  connect(filter_age_mapper_, SIGNAL(mapped(int)), model_, SLOT(SetFilterAge(int)));

  // Set up the dialogs
  library_config_dialog_->SetModel(model_->directory_model());

  // Load settings
  if (!settings_group_.isEmpty()) {
    QSettings s;
    s.beginGroup(settings_group_);
    model_->SetGroupBy(LibraryModel::Grouping(
        LibraryModel::GroupBy(s.value("group_by1", int(LibraryModel::GroupBy_Artist)).toInt()),
        LibraryModel::GroupBy(s.value("group_by2", int(LibraryModel::GroupBy_Album)).toInt()),
        LibraryModel::GroupBy(s.value("group_by3", int(LibraryModel::GroupBy_None)).toInt())));
  }
}

void LibraryFilterWidget::GroupByClicked(QAction* action) {
  if (action->property("group_by").isNull()) {
    group_by_dialog_->show();
    return;
  }

  LibraryModel::Grouping g = action->property("group_by").value<LibraryModel::Grouping>();
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
  foreach (QAction* action, group_by_group_->actions()) {
    if (action->property("group_by").isNull())
      continue;

    if (g == action->property("group_by").value<LibraryModel::Grouping>()) {
      action->setChecked(true);
      return;
    }
  }
  ui_->group_by_advanced->setChecked(true);
}

void LibraryFilterWidget::ClearFilter() {
  ui_->filter->clear();
  ui_->filter->setFocus();
}

void LibraryFilterWidget::ShowConfigDialog() {
  library_config_dialog_->show();
}

void LibraryFilterWidget::SetFilterHint(const QString& hint) {
  ui_->filter->SetHint(hint);
}

void LibraryFilterWidget::SetAgeFilterEnabled(bool enabled) {
  filter_age_menu_->setEnabled(enabled);
}

void LibraryFilterWidget::SetGroupByEnabled(bool enabled) {
  group_by_menu_->setEnabled(enabled);
}

void LibraryFilterWidget::SetConfigDialogEnabled(bool enabled) {
  config_action_->setEnabled(enabled);
}
