/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

#include "songmetadatasettingspage.h"

#include <QSettings>
#include <QThread>

#include "core/player.h"
#include "core/songpathparser.h"
#include "core/tagreaderclient.h"
#include "ui/iconloader.h"
#include "ui_songmetadatasettingspage.h"

SongMetadataSettingsPage::SongMetadataSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_SongMetadataSettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("view-media-lyrics", IconLoader::Base));

  connect(ui_->max_numprocs_tagclients, SIGNAL(valueChanged(int)),
          SLOT(MaxNumProcsTagClientsChanged(int)));
  ui_->max_numprocs_tagclients_value_label->setMinimumWidth(
      QFontMetrics(ui_->max_numprocs_tagclients_value_label->font())
          .width("WWW"));

  // Limit max tag clients to number of CPU cores.
  ui_->max_numprocs_tagclients->setMaximum(QThread::idealThreadCount());
}

SongMetadataSettingsPage::~SongMetadataSettingsPage() { delete ui_; }

void SongMetadataSettingsPage::Load() {
  QSettings s;
  s.beginGroup(Player::kSettingsGroup);

  int max_numprocs_tagclients =
      s.value("max_numprocs_tagclients", QThread::idealThreadCount()).toInt();
  ui_->max_numprocs_tagclients->setValue(max_numprocs_tagclients);
  ui_->max_numprocs_tagclients_value_label->setText(
      QString::number(max_numprocs_tagclients));

  s.endGroup();

  s.beginGroup(SongPathParser::kSongMetadataSettingsGroup);
  bool guess = s.value(SongPathParser::kGuessMetadataSetting,
                       SongPathParser::kGuessMetadataSettingDefault)
                   .toBool();
  ui_->guess_metadata->setCheckState(guess ? Qt::Checked : Qt::Unchecked);

  s.endGroup();
}

void SongMetadataSettingsPage::Save() {
  QSettings s;
  s.beginGroup(Player::kSettingsGroup);

  s.setValue("max_numprocs_tagclients", ui_->max_numprocs_tagclients->value());

  s.endGroup();

  s.beginGroup(SongPathParser::kSongMetadataSettingsGroup);

  s.setValue(SongPathParser::kGuessMetadataSetting,
             ui_->guess_metadata->checkState() != Qt::Unchecked);

  s.endGroup();

  TagReaderClient::Instance()->ReloadSettings();
}

void SongMetadataSettingsPage::MaxNumProcsTagClientsChanged(int value) {
  ui_->max_numprocs_tagclients_value_label->setText(QString::number(value));
}
