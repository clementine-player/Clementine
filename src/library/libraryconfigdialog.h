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

#ifndef LIBRARYCONFIGDIALOG_H
#define LIBRARYCONFIGDIALOG_H

#include <QDialog>

class Ui_LibraryConfigDialog;
class LibraryDirectoryModel;

class LibraryConfigDialog : public QDialog {
  Q_OBJECT
 public:
  LibraryConfigDialog(QWidget* parent = 0);
  ~LibraryConfigDialog();

  void SetModel(LibraryDirectoryModel* model);

 public slots:
  void accept();

 private:
  Ui_LibraryConfigDialog* ui_;
};

#endif // LIBRARYCONFIGDIALOG_H
