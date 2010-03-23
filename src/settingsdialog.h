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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

#include "ui_settingsdialog.h"

class LibraryDirectoryModel;

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  SettingsDialog(QWidget* parent = 0);

  void SetLibraryDirectoryModel(LibraryDirectoryModel* model);

  // QDialog
  void accept();

  // QWidget
  void showEvent(QShowEvent* e);

 private slots:
  void CurrentTextChanged(const QString& text);
  void NotificationTypeChanged();
  void LastFMValidationComplete(bool success);

 private:
  Ui::SettingsDialog ui_;
};

#endif // SETTINGSDIALOG_H
