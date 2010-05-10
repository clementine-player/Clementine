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

#include "libraryconfigdialog.h"
#include "ui_libraryconfigdialog.h"

LibraryConfigDialog::LibraryConfigDialog(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_LibraryConfigDialog)
{
  ui_->setupUi(this);
}

LibraryConfigDialog::~LibraryConfigDialog() {
  delete ui_;
}

void LibraryConfigDialog::SetModel(LibraryDirectoryModel* model) {
  ui_->config->SetModel(model);
}

void LibraryConfigDialog::accept() {
  ui_->config->Save();
  QDialog::accept();
}
