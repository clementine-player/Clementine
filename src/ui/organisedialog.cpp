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

#include "organisedialog.h"
#include "ui_organisedialog.h"

#include <QDir>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QSignalMapper>

const int OrganiseDialog::kNumberOfPreviews = 5;
const char* OrganiseDialog::kDefaultFormat =
    "%artist/%album{ (Disc %disc)}/{%track - }%title.%extension";
const char* OrganiseDialog::kSettingsGroup = "OrganiseDialog";

OrganiseDialog::OrganiseDialog(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_OrganiseDialog)
{
  ui_->setupUi(this);
  connect(ui_->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(Reset()));

  // Valid tags
  QMap<QString, QString> tags;
  tags[tr("Title")] = "title";
  tags[tr("Album")] = "album";
  tags[tr("Artist")] = "artist";
  tags[tr("Artist's initial")] = "artistinitial";
  tags[tr("Album artist")] = "albumartist";
  tags[tr("Composer")] = "composer";
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

void OrganiseDialog::AddDirectoryModel(QAbstractItemModel *model) {
  // TODO: Add this model to a proxy model that merges different models
  // together, eg. from the local library and also removable devices.
  ui_->destination->setModel(model);
}

void OrganiseDialog::SetUrls(const QList<QUrl> &urls) {
  QStringList filenames;
  foreach (const QUrl& url, urls) {
    if (url.scheme() != "file")
      continue;
    filenames << url.toLocalFile();
  }
  SetFilenames(filenames);
}

void OrganiseDialog::SetFilenames(const QStringList &filenames) {
  filenames_ = filenames;
  preview_songs_.clear();

  // Load some of the songs to show in the preview
  const int n = qMin(filenames_.count(), kNumberOfPreviews);
  for (int i=0 ; i<n ; ++i) {
    Song song;
    song.InitFromFile(filenames_[i], -1);
    preview_songs_ << song;
  }

  UpdatePreviews();
}

void OrganiseDialog::InsertTag(const QString &tag) {
  ui_->naming->insertPlainText("%" + tag);
}

void OrganiseDialog::UpdatePreviews() {
  // Update the format object
  format_.set_format(ui_->naming->toPlainText());
  format_.set_replace_non_ascii(ui_->replace_ascii->isChecked());
  format_.set_replace_spaces(ui_->replace_spaces->isChecked());
  format_.set_replace_the(ui_->replace_the->isChecked());

  const bool format_valid = format_.IsValid();
  ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
      format_valid && !ui_->destination->currentText().isEmpty());
  if (!format_valid)
    return;

  ui_->preview->clear();
  foreach (const Song& song, preview_songs_) {
    QString filename = format_.GetFilenameForSong(song);
    ui_->preview->addItem(QDir::toNativeSeparators(filename));
  }
}

void OrganiseDialog::Reset() {
  ui_->naming->setPlainText(kDefaultFormat);
  ui_->replace_ascii->setChecked(false);
  ui_->replace_spaces->setChecked(false);
  ui_->replace_the->setChecked(false);
  ui_->overwrite->setChecked(true);
}

void OrganiseDialog::showEvent(QShowEvent *) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  ui_->naming->setPlainText(s.value("format", kDefaultFormat).toString());
  ui_->replace_ascii->setChecked(s.value("replace_ascii", false).toBool());
  ui_->replace_spaces->setChecked(s.value("replace_spaces", false).toBool());
  ui_->replace_the->setChecked(s.value("replace_the", false).toBool());
  ui_->overwrite->setChecked(s.value("overwrite", true).toBool());
}

void OrganiseDialog::accept() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("format", ui_->naming->toPlainText());
  s.setValue("replace_ascii", ui_->replace_ascii->isChecked());
  s.setValue("replace_spaces", ui_->replace_spaces->isChecked());
  s.setValue("replace_the", ui_->replace_the->isChecked());
  s.setValue("overwrite", ui_->overwrite->isChecked());

  QDialog::accept();
}
