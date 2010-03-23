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

#ifndef ADDSTREAMDIALOG_H
#define ADDSTREAMDIALOG_H

#include <QDialog>

#include "ui_addstreamdialog.h"

class SavedRadio;

class AddStreamDialog : public QDialog {
  Q_OBJECT

 public:
  AddStreamDialog(QWidget *parent = 0);

  QUrl url() const;

  void accept();

 private slots:
  void TextChanged(const QString& text);

 private:
  static const char* kSettingsGroup;

  Ui::AddStreamDialog ui_;

  SavedRadio* saved_radio_;
};

#endif // ADDSTREAMDIALOG_H
