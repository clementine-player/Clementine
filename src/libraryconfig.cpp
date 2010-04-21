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

#include "libraryconfig.h"
#include "librarydirectorymodel.h"
#include "libraryview.h"

#include <QFileDialog>
#include <QSettings>
#include <QDir>

const char* LibraryConfig::kSettingsGroup = "LibraryConfig";

LibraryConfig::LibraryConfig(QWidget* parent)
  : QWidget(parent),
    model_(NULL)
{
  ui_.setupUi(this);

  connect(ui_.add, SIGNAL(clicked()), SLOT(Add()));
  connect(ui_.remove, SIGNAL(clicked()), SLOT(Remove()));
}

void LibraryConfig::SetModel(LibraryDirectoryModel *model) {
  model_ = model;
  ui_.list->setModel(model_);

  connect(ui_.list->selectionModel(),
          SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
          SLOT(CurrentRowChanged(QModelIndex)));


  if (model_->IsBackendReady())
    BackendReady();
  else
    connect(model_, SIGNAL(BackendReady()), SLOT(BackendReady()));
}

void LibraryConfig::Add() {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  QString path(settings.value("last_path", QDir::homePath()).toString());
  path = QFileDialog::getExistingDirectory(this, tr("Add directory..."), path);

  if (!path.isNull()) {
    model_->AddDirectory(path);
  }

  settings.setValue("last_path", path);
}

void LibraryConfig::Remove() {
  model_->RemoveDirectory(ui_.list->currentIndex());
}

void LibraryConfig::CurrentRowChanged(const QModelIndex& index) {
  ui_.remove->setEnabled(index.isValid());
}

void LibraryConfig::BackendReady() {
  ui_.list->setEnabled(true);
  ui_.add->setEnabled(true);
  ui_.remove->setEnabled(true);
}

void LibraryConfig::Save() {
  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);

  s.setValue("auto_open", ui_.auto_open->isChecked());
}

void LibraryConfig::showEvent(QShowEvent *) {
  Load();
}

void LibraryConfig::Load() {
  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);

  ui_.auto_open->setChecked(s.value("auto_open", true).toBool());
}
