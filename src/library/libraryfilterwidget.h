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

#ifndef LIBRARYFILTERWIDGET_H
#define LIBRARYFILTERWIDGET_H

#include <QWidget>

#include <boost/scoped_ptr.hpp>

#include "librarymodel.h"

class GroupByDialog;
class LineEditInterface;
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

  static const int kFilterDelay = 500; // msec

  void SetFilterHint(const QString& hint);
  void SetAgeFilterEnabled(bool enabled);
  void SetGroupByEnabled(bool enabled);
  void AddMenuAction(QAction* action);

  void SetSettingsGroup(const QString& group) { settings_group_ = group; }
  void SetLibraryModel(LibraryModel* model);

 public slots:
  void AppendAndFocus(const QString& text);

 signals:
  void UpPressed();
  void DownPressed();
  void ReturnPressed();

 protected:
  void keyReleaseEvent(QKeyEvent* e);

 private slots:
  void GroupingChanged(const LibraryModel::Grouping& g);
  void GroupByClicked(QAction* action);

  void FilterTextChanged(const QString& text);
  void FilterDelayTimeout();

 private:
  Ui_LibraryFilterWidget* ui_;
  LibraryModel* model_;

  boost::scoped_ptr<GroupByDialog> group_by_dialog_;
  SettingsDialog* settings_dialog_;

  QMenu* filter_age_menu_;
  QMenu* group_by_menu_;
  QMenu* library_menu_;
  QActionGroup* group_by_group_;
  QSignalMapper* filter_age_mapper_;

  QTimer* filter_delay_;

  QString settings_group_;

  LineEditInterface* filter_;
};

#endif // LIBRARYFILTERWIDGET_H
