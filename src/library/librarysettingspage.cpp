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

#include "librarysettingspage.h"

#include "librarybackend.h"
#include "librarydirectorymodel.h"
#include "librarymodel.h"
#include "libraryview.h"
#include "librarywatcher.h"
#include "ui_librarysettingspage.h"
#include "core/application.h"
#include "core/utilities.h"
#include "playlist/playlistdelegates.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QtConcurrentRun>

const char* LibrarySettingsPage::kSettingsGroup = "LibraryConfig";

LibrarySettingsPage::LibrarySettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_LibrarySettingsPage),
      initialised_model_(false) {
  ui_->setupUi(this);
  ui_->list->setItemDelegate(new NativeSeparatorsDelegate(this));

  // Icons
  setWindowIcon(IconLoader::Load("folder-sound"));
  ui_->add->setIcon(IconLoader::Load("document-open-folder"));

  connect(ui_->add, SIGNAL(clicked()), SLOT(Add()));
  connect(ui_->remove, SIGNAL(clicked()), SLOT(Remove()));
  connect(ui_->sync_stats_button, SIGNAL(clicked()),
          SLOT(WriteAllSongsStatisticsToFiles()));
}

LibrarySettingsPage::~LibrarySettingsPage() { delete ui_; }

void LibrarySettingsPage::Add() {
  QSettings settings;
  settings.beginGroup(kSettingsGroup);

  QString path(
      settings.value("last_path", Utilities::GetConfigPath(
                                      Utilities::Path_DefaultMusicLibrary))
          .toString());
  path = QFileDialog::getExistingDirectory(this, tr("Add directory..."), path);

  if (!path.isNull()) {
    dialog()->library_directory_model()->AddDirectory(path);
  }

  settings.setValue("last_path", path);
}

void LibrarySettingsPage::Remove() {
  dialog()->library_directory_model()->RemoveDirectory(
      ui_->list->currentIndex());
}

void LibrarySettingsPage::CurrentRowChanged(const QModelIndex& index) {
  ui_->remove->setEnabled(index.isValid());
}

void LibrarySettingsPage::Save() {
  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);
  s.setValue("auto_open", ui_->auto_open->isChecked());
  s.setValue("pretty_covers", ui_->pretty_covers->isChecked());
  s.setValue("show_dividers", ui_->show_dividers->isChecked());
  s.endGroup();

  s.beginGroup(LibraryWatcher::kSettingsGroup);
  s.setValue("startup_scan", ui_->startup_scan->isChecked());
  s.setValue("monitor", ui_->monitor->isChecked());

  QString filter_text = ui_->cover_art_patterns->text();
  QStringList filters = filter_text.split(',', QString::SkipEmptyParts);
  s.setValue("cover_art_patterns", filters);

  s.endGroup();

  s.beginGroup(LibraryBackend::kSettingsGroup);
  s.setValue("save_ratings_in_file", ui_->save_ratings_in_file->isChecked());
  s.setValue("save_statistics_in_file",
             ui_->save_statistics_in_file->isChecked());
  s.endGroup();
}

void LibrarySettingsPage::Load() {
  if (!initialised_model_) {
    if (ui_->list->selectionModel()) {
      disconnect(ui_->list->selectionModel(),
                 SIGNAL(currentRowChanged(QModelIndex, QModelIndex)), this,
                 SLOT(CurrentRowChanged(QModelIndex)));
    }

    ui_->list->setModel(dialog()->library_directory_model());
    initialised_model_ = true;

    connect(ui_->list->selectionModel(),
            SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            SLOT(CurrentRowChanged(QModelIndex)));
  }

  QSettings s;
  s.beginGroup(LibraryView::kSettingsGroup);
  ui_->auto_open->setChecked(s.value("auto_open", true).toBool());
  ui_->pretty_covers->setChecked(s.value("pretty_covers", true).toBool());
  ui_->show_dividers->setChecked(s.value("show_dividers", true).toBool());
  s.endGroup();

  s.beginGroup(LibraryWatcher::kSettingsGroup);
  ui_->startup_scan->setChecked(s.value("startup_scan", true).toBool());
  ui_->monitor->setChecked(s.value("monitor", true).toBool());

  QStringList filters =
      s.value("cover_art_patterns", QStringList() << "front"
                                                  << "cover").toStringList();
  ui_->cover_art_patterns->setText(filters.join(","));

  s.endGroup();

  s.beginGroup(LibraryBackend::kSettingsGroup);
  ui_->save_ratings_in_file->setChecked(
      s.value("save_ratings_in_file", false).toBool());
  ui_->save_statistics_in_file->setChecked(
      s.value("save_statistics_in_file", false).toBool());
  s.endGroup();
}

void LibrarySettingsPage::WriteAllSongsStatisticsToFiles() {
  QMessageBox confirmation_dialog(
      QMessageBox::Question, tr("Write all songs statistics into songs' files"),
      tr("Are you sure you want to write song's statistics into song's file "
         "for all the songs of your library?"),
      QMessageBox::Yes | QMessageBox::Cancel);
  if (confirmation_dialog.exec() != QMessageBox::Yes) {
    return;
  }
  QtConcurrent::run(dialog()->app()->library(),
                    &Library::WriteAllSongsStatisticsToFiles);
}
