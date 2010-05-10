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

#include "lastfmconfigdialog.h"
#include "ui_lastfmconfigdialog.h"

LastFMConfigDialog::LastFMConfigDialog(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_LastFMConfigDialog)
{
  ui_->setupUi(this);

  connect(ui_->lastfm, SIGNAL(ValidationComplete(bool)), SLOT(ValidationComplete(bool)));
}

LastFMConfigDialog::~LastFMConfigDialog() {
  delete ui_;
}

void LastFMConfigDialog::showEvent(QShowEvent *) {
  ui_->lastfm->Load();
}

void LastFMConfigDialog::accept() {
  if (ui_->lastfm->NeedsValidation()) {
    ui_->lastfm->Validate();
    ui_->buttonBox->setEnabled(false);
  } else {
    ui_->lastfm->Save();
    QDialog::accept();
  }
}

void LastFMConfigDialog::ValidationComplete(bool success) {
  ui_->buttonBox->setEnabled(true);

  if (success)
    QDialog::accept();
}
