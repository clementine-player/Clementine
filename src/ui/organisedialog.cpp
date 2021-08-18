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

#include "organisedialog.h"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QtConcurrentRun>
#include <QtDebug>
#include <algorithm>
#include <memory>

#include "core/musicstorage.h"
#include "core/organise.h"
#include "core/tagreaderclient.h"
#include "core/utilities.h"
#include "iconloader.h"
#include "library/librarybackend.h"
#include "organiseerrordialog.h"
#include "ui_organisedialog.h"

const char* OrganiseDialog::kDefaultFormat =
    "%artist/%album{ (Disc %disc)}/{%track - }%title.%extension";
const char* OrganiseDialog::kSettingsGroup = "OrganiseDialog";

OrganiseDialog::OrganiseDialog(TaskManager* task_manager,
                               LibraryBackend* backend, QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_OrganiseDialog),
      task_manager_(task_manager),
      backend_(backend),
      total_size_(0),
      resized_by_user_(false) {
  ui_->setupUi(this);
  connect(ui_->button_box->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
          SLOT(Reset()));

  ui_->aftercopying->setItemIcon(
      1, IconLoader::Load("edit-delete", IconLoader::Base));

  connect(ui_->destination, SIGNAL(currentIndexChanged(int)),
          SLOT(UpdatePreviews()));
  connect(ui_->naming_group, SIGNAL(OptionChanged()), SLOT(UpdatePreviews()));
  connect(ui_->naming_group, SIGNAL(FormatStringChanged()),
          SLOT(UpdatePreviews()));
}

OrganiseDialog::~OrganiseDialog() { delete ui_; }

void OrganiseDialog::SetDestinationModel(QAbstractItemModel* model,
                                         bool devices) {
  ui_->destination->setModel(model);

  ui_->eject_after->setVisible(devices);

  // In case this is called more than once, disconnect old model.
  if (model_connection_) disconnect(model_connection_);
  // If a device changes, transcoding options may have changed.
  model_connection_ =
      connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
              SLOT(DestDataChanged(QModelIndex, QModelIndex)));
}

bool OrganiseDialog::SetSongs(const SongList& songs) {
  total_size_ = 0;
  songs_.clear();

  for (const Song& song : songs) {
    if (song.url().scheme() != "file") {
      continue;
    }

    if (song.filesize() > 0) total_size_ += song.filesize();

    songs_ << song;
  }

  ui_->free_space->set_additional_bytes(total_size_);
  UpdatePreviews();
  SetLoadingSongs(false);

  if (songs_future_.isRunning()) {
    songs_future_.cancel();
  }
  songs_future_ = QFuture<SongList>();

  return songs_.count();
}

bool OrganiseDialog::SetUrls(const QList<QUrl>& urls) {
  QStringList filenames;

  // Only add file:// URLs
  for (const QUrl& url : urls) {
    if (url.scheme() == "file") {
      filenames << url.toLocalFile();
    }
  }

  return SetFilenames(filenames);
}

bool OrganiseDialog::SetFilenames(const QStringList& filenames) {
  songs_future_ =
      QtConcurrent::run(this, &OrganiseDialog::LoadSongsBlocking, filenames);
  NewClosure(songs_future_, [=]() { SetSongs(songs_future_.result()); });

  SetLoadingSongs(true);
  return true;
}

void OrganiseDialog::SetLoadingSongs(bool loading) {
  if (loading) {
    ui_->preview_stack->setCurrentWidget(ui_->loading_page);
    ui_->button_box->button(QDialogButtonBox::Ok)->setEnabled(false);
  } else {
    ui_->preview_stack->setCurrentWidget(ui_->preview_page);
    // The Ok button is enabled by UpdatePreviews
  }
}

SongList OrganiseDialog::LoadSongsBlocking(const QStringList& filenames) {
  SongList songs;
  Song song;

  QStringList filenames_copy = filenames;
  while (!filenames_copy.isEmpty()) {
    const QString filename = filenames_copy.takeFirst();

    // If it's a directory, add all the files inside.
    if (QFileInfo(filename).isDir()) {
      const QDir dir(filename);
      for (const QString& entry :
           dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot |
                         QDir::Readable)) {
        filenames_copy << dir.filePath(entry);
      }
      continue;
    }

    TagReaderClient::Instance()->ReadFileBlocking(filename, &song);
    if (song.is_valid()) songs << song;
  }

  return songs;
}

void OrganiseDialog::SetCopy(bool copy) {
  ui_->aftercopying->setCurrentIndex(copy ? 0 : 1);
}

Organise::NewSongInfoList OrganiseDialog::ComputeNewSongsFilenames(
    const SongList& songs, const OrganiseFormat& format) {
  QStringList new_filenames = format.GetFilenamesForSongs(songs);
  Q_ASSERT(new_filenames.length() == songs.length());

  Organise::NewSongInfoList new_songs_info;
  for (int i = 0; i < new_filenames.length(); ++i) {
    new_songs_info << Organise::NewSongInfo(songs[i], new_filenames[i]);
  }
  return new_songs_info;
}

void OrganiseDialog::UpdatePreviews() {
  if (songs_future_.isRunning()) {
    return;
  }

  const QModelIndex destination =
      ui_->destination->model()->index(ui_->destination->currentIndex(), 0);
  std::shared_ptr<MusicStorage> storage;
  bool has_local_destination = false;

  if (destination.isValid()) {
    storage = destination.data(MusicStorage::Role_Storage)
                  .value<std::shared_ptr<MusicStorage>>();
    if (storage) {
      has_local_destination = !storage->LocalPath().isEmpty();
    }
  }

  // Update the free space bar
  quint64 capacity = destination.data(MusicStorage::Role_Capacity).toLongLong();
  quint64 free = destination.data(MusicStorage::Role_FreeSpace).toLongLong();

  if (!capacity) {
    ui_->free_space->hide();
  } else {
    ui_->free_space->show();
    ui_->free_space->set_free_bytes(free);
    ui_->free_space->set_total_bytes(capacity);
  }

  // Get updated format object
  OrganiseFormat format = ui_->naming_group->format();

  // If this is set to Transcode_Always, then the user has selected transcode,
  // so we can be fairly certain that the device supports the selected format.
  // However, if the option has not been set, and the device does not support
  // the file's format, then Organise will try to transcode to a new format and
  // the preview will be incorrect.
  if (storage &&
      storage->GetTranscodeMode() == MusicStorage::Transcode_Always) {
    const Song::FileType file_format = storage->GetTranscodeFormat();
    TranscoderPreset preset = Transcoder::PresetForFileType(file_format);
    format.add_tag_override("extension", preset.extension_);
  } else {
    format.reset_tag_overrides();
  }

  const bool format_valid = !has_local_destination || format.IsValid();

  // Are we gonna enable the ok button?
  bool ok = format_valid && !songs_.isEmpty();
  if (capacity != 0 && total_size_ > free) ok = false;

  ui_->button_box->button(QDialogButtonBox::Ok)->setEnabled(ok);
  if (!format_valid) return;

  new_songs_info_ = ComputeNewSongsFilenames(songs_, format);

  // Update the previews
  ui_->preview->clear();
  ui_->preview_group->setVisible(has_local_destination);
  ui_->naming_group->setVisible(has_local_destination);
  if (has_local_destination) {
    for (const Organise::NewSongInfo& song_info : new_songs_info_) {
      QString filename = storage->LocalPath() + "/" + song_info.new_filename_;
      ui_->preview->addItem(QDir::toNativeSeparators(filename));
    }
  }

  if (!resized_by_user_) {
    adjustSize();
  }
}

void OrganiseDialog::DestDataChanged(const QModelIndex& begin,
                                     const QModelIndex& end) {
  const QModelIndex destination =
      ui_->destination->model()->index(ui_->destination->currentIndex(), 0);
  if (QItemSelection(begin, end).contains(destination)) {
    qLog(Debug) << "Destination data changed";
    UpdatePreviews();
  }
}

QSize OrganiseDialog::sizeHint() const { return QSize(650, 0); }

void OrganiseDialog::Reset() {
  ui_->naming_group->Reset();
  ui_->eject_after->setChecked(false);
}

void OrganiseDialog::showEvent(QShowEvent*) {
  resized_by_user_ = false;

  QSettings s;
  s.beginGroup(kSettingsGroup);
  ui_->eject_after->setChecked(s.value("eject_after", false).toBool());

  QString destination = s.value("destination").toString();
  int index = ui_->destination->findText(destination);
  if (index != -1 && !destination.isEmpty()) {
    ui_->destination->setCurrentIndex(index);
  }
}

void OrganiseDialog::accept() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("destination", ui_->destination->currentText());
  s.setValue("eject_after", ui_->eject_after->isChecked());
  ui_->naming_group->StoreSettings();

  const QModelIndex destination =
      ui_->destination->model()->index(ui_->destination->currentIndex(), 0);
  std::shared_ptr<MusicStorage> storage =
      destination.data(MusicStorage::Role_StorageForceConnect)
          .value<std::shared_ptr<MusicStorage>>();

  if (!storage) return;

  // Reset the extension override if we set it. Organise should correctly set
  // the Song object.
  OrganiseFormat format = ui_->naming_group->format();
  format.reset_tag_overrides();

  // It deletes itself when it's finished.
  const bool copy = ui_->aftercopying->currentIndex() == 0;
  Organise* organise =
      new Organise(task_manager_, storage, format, copy,
                   ui_->naming_group->overwrite_existing(),
                   ui_->naming_group->mark_as_listened(), new_songs_info_,
                   ui_->eject_after->isChecked());
  connect(organise, SIGNAL(Finished(QStringList)),
          SLOT(OrganiseFinished(QStringList)));
  connect(organise, SIGNAL(FileCopied(int)), this, SIGNAL(FileCopied(int)));
  if (backend_ != nullptr) {
    connect(organise, SIGNAL(SongPathChanged(const Song&, const QFileInfo&)),
            backend_, SLOT(SongPathChanged(const Song&, const QFileInfo&)));
  }
  organise->Start();

  QDialog::accept();
}

void OrganiseDialog::OrganiseFinished(const QStringList& files_with_errors) {
  if (files_with_errors.isEmpty()) return;

  error_dialog_.reset(new OrganiseErrorDialog);
  error_dialog_->Show(OrganiseErrorDialog::Type_Copy, files_with_errors);
}

void OrganiseDialog::resizeEvent(QResizeEvent* e) {
  if (e->spontaneous()) {
    resized_by_user_ = true;
  }

  QDialog::resizeEvent(e);
}
