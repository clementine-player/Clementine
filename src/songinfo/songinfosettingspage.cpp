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

#include "songinfosettingspage.h"

#include <QFile>
#include <QSettings>

#include "songinfo/lyricsfetcher.h"
#include "songinfotextview.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"
#include "ui_songinfosettingspage.h"

SongInfoSettingsPage::SongInfoSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_SongInfoSettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("view-media-lyrics", IconLoader::Base));

  QFile song_info_preview(":/lumberjacksong.txt");
  (void)song_info_preview.open(QIODevice::ReadOnly);
  ui_->song_info_font_preview->setText(
      QString::fromUtf8(song_info_preview.readAll()));

  connect(ui_->song_info_font_size, SIGNAL(valueChanged(double)),
          SLOT(FontSizeChanged(double)));
}

SongInfoSettingsPage::~SongInfoSettingsPage() { delete ui_; }

void SongInfoSettingsPage::Load() {
  QSettings s;

  s.beginGroup(SongInfoTextView::kSettingsGroup);
  ui_->song_info_font_size->setValue(
      s.value("font_size", SongInfoTextView::kDefaultFontSize).toReal());
  s.endGroup();

  s.beginGroup(LyricsFetcher::kSettingsGroup);
  ui_->online_lyrics->setChecked(
      s.value(LyricsFetcher::kOnlineLyricsKey, true).toBool());
}

void SongInfoSettingsPage::Save() {
  QSettings s;

  s.beginGroup(SongInfoTextView::kSettingsGroup);
  s.setValue("font_size", ui_->song_info_font_preview->font().pointSizeF());
  s.endGroup();

  s.beginGroup(LyricsFetcher::kSettingsGroup);
  s.setValue(LyricsFetcher::kOnlineLyricsKey, ui_->online_lyrics->isChecked());
  // The lyrics websites Clementine used to search, in order.
  s.remove("search_order");
  s.endGroup();
}

void SongInfoSettingsPage::FontSizeChanged(double value) {
  QFont font;
  font.setPointSizeF(value);

  ui_->song_info_font_preview->setFont(font);
}
