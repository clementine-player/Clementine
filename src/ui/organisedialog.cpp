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

#include "iconloader.h"
#include "organisedialog.h"
#include "organiseerrordialog.h"
#include "ui_organisedialog.h"
#include "core/musicstorage.h"
#include "core/organise.h"
#include "core/tagreaderclient.h"

#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QSignalMapper>
#include <QtDebug>

const char* OrganiseDialog::kDefaultFormat =
    "%artist/%album{ (Disc %disc)}/{%track - }%title.%extension";
const char* OrganiseDialog::kSettingsGroup = "OrganiseDialog";

OrganiseDialog::OrganiseDialog(TaskManager* task_manager, QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_OrganiseDialog),
    task_manager_(task_manager),
    total_size_(0),
    resized_by_user_(false)
{
  ui_->setupUi(this);
  connect(ui_->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(Reset()));

  ui_->aftercopying->setItemIcon(1, IconLoader::Load("edit-delete"));

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
  tags[tr("Track")] = "track";
  tags[tr("Disc")] = "disc";
  tags[tr("BPM")] = "bpm";
  tags[tr("Year")] = "year";
  tags[tr("Genre")] = "genre";
  tags[tr("Comment")] = "comment";
  tags[tr("Length")] = "length";
  tags[tr("Bitrate")] = "bitrate";
  tags[tr("Samplerate")] = "samplerate";
  tags[tr("File extension")] = "extension";

  // Naming scheme input field
  new OrganiseFormat::SyntaxHighlighter(ui_->naming);

  connect(ui_->destination, SIGNAL(currentIndexChanged(int)), SLOT(UpdatePreviews()));
  connect(ui_->naming, SIGNAL(textChanged()), SLOT(UpdatePreviews()));
  connect(ui_->replace_ascii, SIGNAL(toggled(bool)), SLOT(UpdatePreviews()));
  connect(ui_->replace_the, SIGNAL(toggled(bool)), SLOT(UpdatePreviews()));
  connect(ui_->replace_spaces, SIGNAL(toggled(bool)), SLOT(UpdatePreviews()));

  // Get the titles of the tags to put in the insert menu
  QStringList tag_titles = tags.keys();
  qStableSort(tag_titles);

  // Build the insert menu
  QMenu* tag_menu = new QMenu(this);
  QSignalMapper* tag_mapper = new QSignalMapper(this);
  foreach (const QString& title, tag_titles) {
    QAction* action = tag_menu->addAction(title, tag_mapper, SLOT(map()));
    tag_mapper->setMapping(action, tags[title]);
  }

  connect(tag_mapper, SIGNAL(mapped(QString)), SLOT(InsertTag(QString)));
  ui_->insert->setMenu(tag_menu);
}

OrganiseDialog::~OrganiseDialog() {
  delete ui_;
}

void OrganiseDialog::SetDestinationModel(QAbstractItemModel *model, bool devices) {
  ui_->destination->setModel(model);

  ui_->eject_after->setVisible(devices);
}

int OrganiseDialog::SetSongs(const SongList& songs) {
  total_size_ = 0;
  filenames_.clear();
  preview_songs_.clear();

  foreach (const Song& song, songs) {
    if (song.url().scheme() != "file") {
      continue;
    };

    if (song.filesize() > 0)
      total_size_ += song.filesize();
    filenames_ << song.url().toLocalFile();

    preview_songs_ << song;
  }

  ui_->free_space->set_additional_bytes(total_size_);
  UpdatePreviews();

  return filenames_.count();
}

int OrganiseDialog::SetUrls(const QList<QUrl> &urls, quint64 total_size) {
  QStringList filenames;

  // Only add file:// URLs
  foreach (const QUrl& url, urls) {
    if (url.scheme() != "file")
      continue;
    filenames << url.toLocalFile();
  }

  return SetFilenames(filenames, total_size);
}

int OrganiseDialog::SetFilenames(const QStringList& filenames, quint64 total_size) {
  filenames_ = filenames;
  preview_songs_.clear();

  // Load some of the songs to show in the preview
  const int n = filenames_.count();
  for (int i=0 ; i<n ; ++i) {
    LoadPreviewSongs(filenames_[i]);
  }

  ui_->free_space->set_additional_bytes(total_size);
  total_size_ = total_size;

  UpdatePreviews();

  return filenames_.count();
}

void OrganiseDialog::LoadPreviewSongs(const QString& filename) {

  if (QFileInfo(filename).isDir()) {
    QDir dir(filename);
    QStringList entries = dir.entryList(
        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
    foreach (const QString& entry, entries) {
      LoadPreviewSongs(filename + "/" + entry);
    }
    return;
  }

  Song song;
  TagReaderClient::Instance()->ReadFileBlocking(filename, &song);

  if (song.is_valid())
    preview_songs_ << song;
}

void OrganiseDialog::SetCopy(bool copy) {
  ui_->aftercopying->setCurrentIndex(copy ? 0 : 1);
}

void OrganiseDialog::InsertTag(const QString &tag) {
  ui_->naming->insertPlainText("%" + tag);
}

void OrganiseDialog::UpdatePreviews() {
  const QModelIndex destination = ui_->destination->model()->index(
      ui_->destination->currentIndex(), 0);
  boost::shared_ptr<MusicStorage> storage;
  bool has_local_destination = false;

  if (destination.isValid()) {
    storage = destination.data(MusicStorage::Role_Storage)
              .value<boost::shared_ptr<MusicStorage> >();
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
  bool ok = format_valid && !filenames_.isEmpty();
  if (capacity != 0 && total_size_ > free)
    ok = false;

  ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
  if (!format_valid)
    return;

  // Update the previews
  ui_->preview->clear();
  ui_->preview_group->setVisible(has_local_destination);
  ui_->naming_group->setVisible(has_local_destination);
  if (has_local_destination) {
    foreach (const Song& song, preview_songs_) {
      QString filename = storage->LocalPath() + "/" +
                         format_.GetFilenameForSong(song);
      ui_->preview->addItem(QDir::toNativeSeparators(filename));
    }
  }

  if (!resized_by_user_) {
    adjustSize();
  }
}

QSize OrganiseDialog::sizeHint() const {
  return QSize(650, 0);
}

void OrganiseDialog::Reset() {
  ui_->naming->setPlainText(kDefaultFormat);
  ui_->replace_ascii->setChecked(false);
  ui_->replace_spaces->setChecked(false);
  ui_->replace_the->setChecked(false);
  ui_->overwrite->setChecked(true);
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
  ui_->overwrite->setChecked(s.value("overwrite", true).toBool());
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
  s.setValue("destination", ui_->destination->currentText());
  s.setValue("eject_after", ui_->eject_after->isChecked());

  const QModelIndex destination = ui_->destination->model()->index(
      ui_->destination->currentIndex(), 0);
  boost::shared_ptr<MusicStorage> storage =
      destination.data(MusicStorage::Role_StorageForceConnect)
      .value<boost::shared_ptr<MusicStorage> >();

  if (!storage)
    return;

  // It deletes itself when it's finished.
  const bool copy = ui_->aftercopying->currentIndex() == 0;
  Organise* organise = new Organise(
      task_manager_, storage, format_, copy, ui_->overwrite->isChecked(),
      filenames_, ui_->eject_after->isChecked());
  connect(organise, SIGNAL(Finished(QStringList)), SLOT(OrganiseFinished(QStringList)));
  organise->Start();

  QDialog::accept();
}

void OrganiseDialog::OrganiseFinished(const QStringList& files_with_errors) {
  if (files_with_errors.isEmpty())
    return;

  error_dialog_.reset(new OrganiseErrorDialog);
  error_dialog_->Show(OrganiseErrorDialog::Type_Copy, files_with_errors);
}

void OrganiseDialog::resizeEvent(QResizeEvent* e) {
  if (e->spontaneous()) {
    resized_by_user_ = true;
  }

  QDialog::resizeEvent(e);
}
