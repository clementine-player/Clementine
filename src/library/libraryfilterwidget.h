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

#ifndef LIBRARYFILTERWIDGET_H
#define LIBRARYFILTERWIDGET_H

#include <QWidget>

#include <boost/scoped_ptr.hpp>

#include "librarymodel.h"

class GroupByDialog;
class SettingsDialog;
class Ui_LibraryFilterWidget;

class QMenu;
class QActionGroup;
class QSignalMapper;

class LibraryFilterWidget : public QWidget {
  Q_OBJECT

 public:
  LibraryFilterWidget(QWidget* parent = 0);
  ~LibraryFilterWidget();

  void SetFilterHint(const QString& hint);
  void SetAgeFilterEnabled(bool enabled);
  void SetGroupByEnabled(bool enabled);
  void SetConfigDialogEnabled(bool enabled);

  void SetSettingsGroup(const QString& group) { settings_group_ = group; }
  void SetLibraryModel(LibraryModel* model);
  void SetSettingsDialog(SettingsDialog* dialog) { settings_dialog_ = dialog; }

 signals:
  void LibraryConfigChanged();

 public slots:
  void ShowConfigDialog();

 private slots:
  void GroupingChanged(const LibraryModel::Grouping& g);
  void GroupByClicked(QAction* action);
  void ClearFilter();

 private:
  Ui_LibraryFilterWidget* ui_;
  LibraryModel* model_;

  boost::scoped_ptr<GroupByDialog> group_by_dialog_;
  SettingsDialog* settings_dialog_;

  QMenu* filter_age_menu_;
  QMenu* group_by_menu_;
  QAction* config_action_;
  QActionGroup* group_by_group_;
  QSignalMapper* filter_age_mapper_;

  QString settings_group_;
};

#endif // LIBRARYFILTERWIDGET_H
