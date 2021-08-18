/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2021, Lukas Prediger <lumip@lumip.de>

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

#include "filenameformatwidget.h"

#include <QDebug>
#include <QMap>
#include <QMenu>
#include <QSettings>
#include <QString>
#include <QStringList>

#include "core/organise.h"
#include "ui/organisedialog.h"
#include "ui_filenameformatwidget.h"

const char* FileNameFormatWidget::kDefaultFormat =
    "%artist/%album{ (Disc %disc)}/{%track - }%title.%extension";
const char* FileNameFormatWidget::kSettingsGroup = "FileNameFormatWidget";

FileNameFormatWidget::FileNameFormatWidget(QWidget* parent)
    : QWidget(parent), ui_(new Ui_FileNameFormatWidget) {
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  ui_->setupUi(this);

  // Syntax highlighting for naming scheme input field.
  // attaches as child to ui_->naming, which transfers ownership
  new OrganiseFormat::SyntaxHighlighter(ui_->naming);

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

  // Get the titles of the tags to put in the insert menu
  QStringList tag_titles = tags.keys();
  std::stable_sort(tag_titles.begin(), tag_titles.end());

  // Build the insert menu
  QMenu* tag_menu = new QMenu(this);
  for (const QString& title : tag_titles) {
    QAction* action = tag_menu->addAction(title);
    QString tag = tags[title];
    connect(action, &QAction::triggered, [this, tag]() { InsertTag(tag); });
  }

  ui_->insert->setMenu(tag_menu);

  connect(ui_->naming, SIGNAL(textChanged()), SIGNAL(FormatStringChanged()));
  connect(ui_->replace_ascii, SIGNAL(toggled(bool)), SIGNAL(OptionChanged()));
  connect(ui_->replace_spaces, SIGNAL(toggled(bool)), SIGNAL(OptionChanged()));
  connect(ui_->replace_the, SIGNAL(toggled(bool)), SIGNAL(OptionChanged()));
  connect(ui_->overwrite, SIGNAL(toggled(bool)), SIGNAL(OptionChanged()));
  connect(ui_->mark_as_listened, SIGNAL(toggled(bool)),
          SIGNAL(OptionChanged()));

  LoadSettings();
}

void FileNameFormatWidget::Reset() {
  ui_->naming->setPlainText(kDefaultFormat);
  ui_->replace_ascii->setChecked(false);
  ui_->replace_spaces->setChecked(false);
  ui_->replace_the->setChecked(false);
  ui_->overwrite->setChecked(false);
  ui_->mark_as_listened->setChecked(false);
}

bool FileNameFormatWidget::ignore_the() const {
  return ui_->replace_the->isChecked();
}

bool FileNameFormatWidget::replace_spaces() const {
  return ui_->replace_spaces->isChecked();
}

bool FileNameFormatWidget::restrict_to_ascii() const {
  return ui_->replace_ascii->isChecked();
}

bool FileNameFormatWidget::overwrite_existing() const {
  return ui_->overwrite->isChecked();
}

bool FileNameFormatWidget::mark_as_listened() const {
  return ui_->mark_as_listened->isChecked();
}

OrganiseFormat FileNameFormatWidget::format() const {
  OrganiseFormat format;
  format.set_format(ui_->naming->toPlainText());
  format.set_replace_non_ascii(ui_->replace_ascii->isChecked());
  format.set_replace_spaces(ui_->replace_spaces->isChecked());
  format.set_replace_the(ui_->replace_the->isChecked());
  return format;
}

void FileNameFormatWidget::InsertTag(const QString& tag) {
  ui_->naming->insertPlainText("%" + tag);
}

void FileNameFormatWidget::LoadSettings() {
  QSettings s;

  // transitional fallback: if the new kSettingsGroup for FileNameFormatWidget
  // is not present, try loading from OrganiseDialog::kSettingsGroup, where
  // these settings where previously held.
  if (!s.childGroups().contains(kSettingsGroup) &&
      s.childGroups().contains(OrganiseDialog::kSettingsGroup)) {
    s.beginGroup(OrganiseDialog::kSettingsGroup);
  } else {
    s.beginGroup(kSettingsGroup);
  }

  ui_->naming->setPlainText(s.value("format", kDefaultFormat).toString());
  ui_->replace_ascii->setChecked(s.value("replace_ascii", false).toBool());
  ui_->replace_spaces->setChecked(s.value("replace_spaces", false).toBool());
  ui_->replace_the->setChecked(s.value("replace_the", false).toBool());
  ui_->overwrite->setChecked(s.value("overwrite", false).toBool());
  ui_->mark_as_listened->setChecked(
      s.value("mark_as_listened", false).toBool());
}

void FileNameFormatWidget::StoreSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("format", ui_->naming->toPlainText());
  s.setValue("replace_ascii", ui_->replace_ascii->isChecked());
  s.setValue("replace_spaces", ui_->replace_spaces->isChecked());
  s.setValue("replace_the", ui_->replace_the->isChecked());
  s.setValue("overwrite", ui_->overwrite->isChecked());
  s.setValue("mark_as_listened", ui_->overwrite->isChecked());
}
