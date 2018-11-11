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
#include "ui_organisedialog.h"

#include <algorithm>
#include <memory>

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QSignalMapper>
#include <QtConcurrentRun>
#include <QtDebug>

#include "iconloader.h"
#include "organiseerrordialog.h"
#include "core/musicstorage.h"
#include "core/organise.h"
#include "core/tagreaderclient.h"
#include "core/utilities.h"
#include "library/librarybackend.h"

const char* OrganiseDialog::kDefaultFormat =
    "%artist/%album{ (Disc %disc)}/{%track - }%title.%extension";
const char* OrganiseDialog::kSettingsGroup = "OrganiseDialog";

OrganiseDialog::OrganiseDialog(
    TaskManager* task_manager, LibraryBackend* backend, QWidget* parent)
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

  // Valid tags
  QMap<QString, QString> tags;
  tags[tr("Title")] = "title";
  tags[tr("Album")] = "album";
  tags[tr("Artist")] = "artist";
  tags[tr("Artist's initial")] = "artistinitial";
  tags[tr("Album artist")] = "albumartist";
  tags[tr("Composer")] = "composer";
  tags[tr("Performer")] = "performer";
  tags[tr("Grouping")] = "grouping";
  tags[tr("Lyrics")] = "lyrics";
  tags[tr("Track")] = "track";
  tags[tr("Disc")] = "disc";
  tags[tr("BPM")] = "bpm";
  tags[tr("Year")] = "year";
  tags[tr("Original year")] = "originalyear";
  tags[tr("Genre")] = "genre";
  tags[tr("Comment")] = "comment";
  tags[tr("Length")] = "length";
  tags[tr("Bitrate", "Refers to bitrate in file organise dialog.")] = "bitrate";
  tags[tr("Samplerate")] = "samplerate";
  tags[tr("File extension")] = "extension";

  // Naming scheme input field
  new OrganiseFormat::SyntaxHighlighter(ui_->naming);

  connect(ui_->destination, SIGNAL(currentIndexChanged(int)),
          SLOT(UpdatePreviews()));
  connect(ui_->naming, SIGNAL(textChanged()), SLOT(UpdatePreviews()));
  connect(ui_->replace_ascii, SIGNAL(toggled(bool)), SLOT(UpdatePreviews()));
  connect(ui_->replace_the, SIGNAL(toggled(bool)), SLOT(UpdatePreviews()));
  connect(ui_->replace_spaces, SIGNAL(toggled(bool)), SLOT(UpdatePreviews()));

  // Get the titles of the tags to put in the insert menu
  QStringList tag_titles = tags.keys();
  std::stable_sort(tag_titles.begin(), tag_titles.end());

  // Build the insert menu
  QMenu* tag_menu = new QMenu(this);
  QSignalMapper* tag_mapper = new QSignalMapper(this);
  for (const QString& title : tag_titles) {
    QAction* action = tag_menu->addAction(title, tag_mapper, SLOT(map()));
    tag_mapper->setMapping(action, tags[title]);
  }

  connect(tag_mapper, SIGNAL(mapped(QString)), SLOT(InsertTag(QString)));
  ui_->insert->setMenu(tag_menu);
}

OrganiseDialog::~OrganiseDialog() { delete ui_; }

void OrganiseDialog::SetDestinationModel(QAbstractItemModel* model,
                                         bool devices) {
  ui_->destination->setModel(model);

  ui_->eject_after->setVisible(devices);
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

void OrganiseDialog::InsertTag(const QString& tag) {
  ui_->naming->insertPlainText("%" + tag);
}

Organise::NewSongInfoList OrganiseDialog::ComputeNewSongsFilenames(
    const SongList& songs, const OrganiseFormat& format) {
  // Check if we will have multiple files with the same name.
  // If so, they will erase each other if the overwrite flag is set.
  // Better to rename them: e.g. foo.bar -> foo(2).bar
  QHash<QString, int> filenames;
  Organise::NewSongInfoList new_songs_info;

  for (const Song& song : songs) {
    QString new_filename = format.GetFilenameForSong(song);
    if (filenames.contains(new_filename)) {
      QString song_number = QString::number(++filenames[new_filename]);
      new_filename = Utilities::PathWithoutFilenameExtension(new_filename) +
                     "(" + song_number + ")." +
                     QFileInfo(new_filename).suffix();
    }
    filenames.insert(new_filename, 1);
    new_songs_info << Organise::NewSongInfo(song, new_filename);
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

  // Update the format object
  format_.set_format(ui_->naming->toPlainText());
  format_.set_replace_non_ascii(ui_->replace_ascii->isChecked());
  format_.set_replace_spaces(ui_->replace_spaces->isChecked());
  format_.set_replace_the(ui_->replace_the->isChecked());

  const bool format_valid = !has_local_destination || format_.IsValid();

  // Are we gonna enable the ok button?
  bool ok = format_valid && !songs_.isEmpty();
  if (capacity != 0 && total_size_ > free) ok = false;

  ui_->button_box->button(QDialogButtonBox::Ok)->setEnabled(ok);
  if (!format_valid) return;

  new_songs_info_ = ComputeNewSongsFilenames(songs_, format_);

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

QSize OrganiseDialog::sizeHint() const { return QSize(650, 0); }

void OrganiseDialog::Reset() {
  ui_->naming->setPlainText(kDefaultFormat);
  ui_->replace_ascii->setChecked(false);
  ui_->replace_spaces->setChecked(false);
  ui_->replace_the->setChecked(false);
  ui_->overwrite->setChecked(false);
  ui_->mark_as_listened->setChecked(false);
  ui_->eject_after->setChecked(false);
}

void OrganiseDialog::showEvent(QShowEvent*) {
  resized_by_user_ = false;

  QSettings s;
  s.beginGroup(kSettingsGroup);
  ui_->naming->setPlainText(s.value("format", kDefaultFormat).toString());
  ui_->replace_ascii->setChecked(s.value("replace_ascii", false).toBool());
  ui_->replace_spaces->setChecked(s.value("replace_spaces", false).toBool());
  ui_->replace_the->setChecked(s.value("replace_the", false).toBool());
  ui_->overwrite->setChecked(s.value("overwrite", false).toBool());
  ui_->mark_as_listened->setChecked(
      s.value("mark_as_listened", false).toBool());
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
  s.setValue("format", ui_->naming->toPlainText());
  s.setValue("replace_ascii", ui_->replace_ascii->isChecked());
  s.setValue("replace_spaces", ui_->replace_spaces->isChecked());
  s.setValue("replace_the", ui_->replace_the->isChecked());
  s.setValue("overwrite", ui_->overwrite->isChecked());
  s.setValue("mark_as_listened", ui_->overwrite->isChecked());
  s.setValue("destination", ui_->destination->currentText());
  s.setValue("eject_after", ui_->eject_after->isChecked());

  const QModelIndex destination =
      ui_->destination->model()->index(ui_->destination->currentIndex(), 0);
  std::shared_ptr<MusicStorage> storage =
      destination.data(MusicStorage::Role_StorageForceConnect)
          .value<std::shared_ptr<MusicStorage>>();

  if (!storage) return;

  // It deletes itself when it's finished.
  const bool copy = ui_->aftercopying->currentIndex() == 0;
  Organise* organise = new Organise(
      task_manager_, storage, format_, copy, ui_->overwrite->isChecked(),
      ui_->mark_as_listened->isChecked(), new_songs_info_,
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
