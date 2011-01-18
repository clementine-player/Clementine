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

#include "installscriptdialog.h"
#include "scriptarchive.h"
#include "ui_installscriptdialog.h"

#include <QListWidgetItem>
#include <QMessageBox>

InstallScriptDialog::InstallScriptDialog(ScriptArchive* archive, QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_InstallScriptDialog),
    archive_(archive)
{
  setAttribute(Qt::WA_DeleteOnClose);
  ui_->setupUi(this);

  // Add the script info to the list
  foreach (const ScriptInfo& info, archive->info()) {
    ui_->list->addItem(new QListWidgetItem(info.icon(), info.name(), ui_->list));
  }
}

InstallScriptDialog::~InstallScriptDialog() {
  delete ui_;
  delete archive_;
}

void InstallScriptDialog::accept() {
  if (!archive_->Install()) {
    QMessageBox::warning(this, tr("Error installing script"),
      tr("Some files failed to install.  The script may not work correctly."));
  }
  QDialog::accept();
}
