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

#include "lyricsettings.h"
#include "songinfoview.h"
#include "ultimatelyricsprovider.h"
#include "ui_lyricsettings.h"

#include <QSettings>

LyricSettings::LyricSettings(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_LyricSettings),
    view_(NULL)
{
  ui_->setupUi(this);

  connect(ui_->up, SIGNAL(clicked()), SLOT(MoveUp()));
  connect(ui_->down, SIGNAL(clicked()), SLOT(MoveDown()));
  connect(ui_->providers, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
          SLOT(CurrentItemChanged(QListWidgetItem*)));
  connect(ui_->providers, SIGNAL(itemChanged(QListWidgetItem*)),
          SLOT(ItemChanged(QListWidgetItem*)));
}

LyricSettings::~LyricSettings() {
  delete ui_;
}

void LyricSettings::Load() {
  QList<const UltimateLyricsProvider*> providers = view_->lyric_providers();

  ui_->providers->clear();
  foreach (const UltimateLyricsProvider* provider, providers) {
    QListWidgetItem* item = new QListWidgetItem(ui_->providers);
    item->setText(provider->name());
    item->setCheckState(provider->is_enabled() ? Qt::Checked : Qt::Unchecked);
    item->setForeground(provider->is_enabled() ? palette().color(QPalette::Active, QPalette::Text)
                                               : palette().color(QPalette::Disabled, QPalette::Text));
  }
}

void LyricSettings::Save() {
  QSettings s;
  s.beginGroup(SongInfoView::kSettingsGroup);

  QVariantList search_order;
  for (int i=0 ; i<ui_->providers->count() ; ++i) {
    const QListWidgetItem* item = ui_->providers->item(i);
    if (item->checkState() == Qt::Checked)
      search_order << item->text();
  }
  s.setValue("search_order", search_order);
}

void LyricSettings::CurrentItemChanged(QListWidgetItem* item) {
  if (!item) {
    ui_->up->setEnabled(false);
    ui_->down->setEnabled(false);
  } else {
    const int row = ui_->providers->row(item);
    ui_->up->setEnabled(row != 0);
    ui_->down->setEnabled(row != ui_->providers->count() - 1);
  }
}

void LyricSettings::MoveUp() {
  Move(-1);
}

void LyricSettings::MoveDown() {
  Move(+1);
}

void LyricSettings::Move(int d) {
  const int row = ui_->providers->currentRow();
  QListWidgetItem* item = ui_->providers->takeItem(row);
  ui_->providers->insertItem(row + d, item);
  ui_->providers->setCurrentRow(row + d);
}

void LyricSettings::ItemChanged(QListWidgetItem* item) {
  const bool checked = item->checkState() == Qt::Checked;
  item->setForeground(checked ? palette().color(QPalette::Active, QPalette::Text)
                              : palette().color(QPalette::Disabled, QPalette::Text));
}
