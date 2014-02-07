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

#include "songinfofetcher.h"
#include "songinfosettingspage.h"
#include "songinfotextview.h"
#include "songinfoview.h"
#include "ultimatelyricsprovider.h"
#include "ui_songinfosettingspage.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"

#include <QFile>
#include <QSettings>

SongInfoSettingsPage::SongInfoSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui_SongInfoSettingsPage) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("view-media-lyrics"));

  connect(ui_->up, SIGNAL(clicked()), SLOT(MoveUp()));
  connect(ui_->down, SIGNAL(clicked()), SLOT(MoveDown()));
  connect(ui_->providers,
          SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
          SLOT(CurrentItemChanged(QListWidgetItem*)));
  connect(ui_->providers, SIGNAL(itemChanged(QListWidgetItem*)),
          SLOT(ItemChanged(QListWidgetItem*)));

  QFile song_info_preview(":/lumberjacksong.txt");
  song_info_preview.open(QIODevice::ReadOnly);
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

  QList<const UltimateLyricsProvider*> providers =
      dialog()->song_info_view()->lyric_providers();

  ui_->providers->clear();
  foreach(const UltimateLyricsProvider * provider, providers) {
    QListWidgetItem* item = new QListWidgetItem(ui_->providers);
    item->setText(provider->name());
    item->setCheckState(provider->is_enabled() ? Qt::Checked : Qt::Unchecked);
    item->setForeground(
        provider->is_enabled()
            ? palette().color(QPalette::Active, QPalette::Text)
            : palette().color(QPalette::Disabled, QPalette::Text));
  }
}

void SongInfoSettingsPage::Save() {
  QSettings s;

  s.beginGroup(SongInfoTextView::kSettingsGroup);
  s.setValue("font_size", ui_->song_info_font_preview->font().pointSizeF());
  s.endGroup();

  s.beginGroup(SongInfoView::kSettingsGroup);
  QVariantList search_order;
  for (int i = 0; i < ui_->providers->count(); ++i) {
    const QListWidgetItem* item = ui_->providers->item(i);
    if (item->checkState() == Qt::Checked) search_order << item->text();
  }
  s.setValue("search_order", search_order);
  s.endGroup();
}

void SongInfoSettingsPage::CurrentItemChanged(QListWidgetItem* item) {
  if (!item) {
    ui_->up->setEnabled(false);
    ui_->down->setEnabled(false);
  } else {
    const int row = ui_->providers->row(item);
    ui_->up->setEnabled(row != 0);
    ui_->down->setEnabled(row != ui_->providers->count() - 1);
  }
}

void SongInfoSettingsPage::MoveUp() { Move(-1); }

void SongInfoSettingsPage::MoveDown() { Move(+1); }

void SongInfoSettingsPage::Move(int d) {
  const int row = ui_->providers->currentRow();
  QListWidgetItem* item = ui_->providers->takeItem(row);
  ui_->providers->insertItem(row + d, item);
  ui_->providers->setCurrentRow(row + d);
}

void SongInfoSettingsPage::ItemChanged(QListWidgetItem* item) {
  const bool checked = item->checkState() == Qt::Checked;
  item->setForeground(
      checked ? palette().color(QPalette::Active, QPalette::Text)
              : palette().color(QPalette::Disabled, QPalette::Text));
}

void SongInfoSettingsPage::FontSizeChanged(double value) {
  QFont font;
  font.setPointSizeF(value);

  ui_->song_info_font_preview->setFont(font);
}
