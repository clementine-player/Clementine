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

#include "settingscategory.h"

#include "settingspage.h"

SettingsCategory::SettingsCategory(const QString& name, SettingsDialog* dialog)
    : dialog_(dialog) {
  setText(0, name);
  setData(0, SettingsDialog::Role_IsSeparator, true);
  setFlags(Qt::ItemIsEnabled);
}

SettingsCategory::SettingsCategory(SettingsDialog::Page id, SettingsPage* page,
                                   SettingsDialog* dialog)
    : dialog_(dialog) {
  setText(0, page->windowTitle());
  setData(0, SettingsDialog::Role_IsSeparator, true);
  setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  dialog->AddPageToStack(id, page, this);
}

void SettingsCategory::AddPage(SettingsDialog::Page id, SettingsPage* page) {
  // Create the list item
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText(0, page->windowTitle());
  item->setIcon(0, page->windowIcon());
  item->setData(0, SettingsDialog::Role_IsSeparator, false);

  if (!page->IsEnabled()) {
    item->setFlags(Qt::NoItemFlags);
  }

  addChild(item);

  dialog_->AddPageToStack(id, page, item);
}
