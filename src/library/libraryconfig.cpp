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
#include "librarywatcher.h"
#include "ui_libraryconfig.h"
#include "ui/iconloader.h"

#include <QFileDialog>
#include <QSettings>
#include <QDir>

const char* LibraryConfig::kSettingsGroup = "LibraryConfig";

LibraryConfig::LibraryConfig(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_LibraryConfig),
    model_(NULL)
{
  ui_->setupUi(this);

  // Icons
  ui_->add->setIcon(IconLoader::Load("document-open-folder"));

  connect(ui_->add, SIGNAL(clicked()), SLOT(Add()));
  connect(ui_->remove, SIGNAL(clicked()), SLOT(Remove()));
}

LibraryConfig::~LibraryConfig() {
  delete ui_;
}

void LibraryConfig::SetModel(LibraryDirectoryModel *model) {
  if (ui_->list->selectionModel()) {
    disconnect(ui_->list->selectionModel(),
               SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
               this, SLOT(CurrentRowChanged(QModelIndex)));
  }

  model_ = model;
  ui_->list->setModel(model_);

  connect(ui_->list->selectionModel(),
          SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
          SLOT(CurrentRowChanged(QModelIndex)));
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
  model_->RemoveDirectory(ui_->list->currentIndex());
}

void LibraryConfig::CurrentRowChanged(const QModelIndex& index) {
  ui_->remove->setEnabled(index.isValid());
}

void LibraryConfig::Save() {
  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);
  s.setValue("auto_open", ui_->auto_open->isChecked());
  s.setValue("autoclear_playlist", ui_->auto_load->isChecked());
  s.endGroup();

  s.beginGroup(LibraryWatcher::kSettingsGroup);
  s.setValue("startup_scan", ui_->startup_scan->isChecked());
  s.endGroup();
}

void LibraryConfig::showEvent(QShowEvent *) {
  Load();
}

void LibraryConfig::Load() {
  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);
  ui_->auto_open->setChecked(s.value("auto_open", true).toBool());
  ui_->auto_load->setChecked(s.value("autoclear_playlist", false).toBool());
  s.endGroup();

  s.beginGroup(LibraryWatcher::kSettingsGroup);
  ui_->startup_scan->setChecked(s.value("startup_scan", true).toBool());
  s.endGroup();
}
