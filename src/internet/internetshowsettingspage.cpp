/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "internetshowsettingspage.h"

#include "core/application.h"
#include "ui/settingsdialog.h"
#include "internetservice.h"
#include "internetmodel.h"

#include <QSettings>

InternetShowSettingsPage::InternetShowSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent), ui_(new Ui::InternetShowSettingsPage) {
  ui_->setupUi(this);

  ui_->sources->header()->setResizeMode(0, QHeaderView::Stretch);
  ui_->sources->header()->setResizeMode(1, QHeaderView::ResizeToContents);
}

void InternetShowSettingsPage::Load() {
  QMap<InternetService*, InternetModel::ServiceItem> shown_services =
      dialog()->app()->internet_model()->shown_services();

  ui_->sources->clear();

  for (QMap<InternetService*, InternetModel::ServiceItem>::iterator service =
           shown_services.begin();
       service != shown_services.end(); ++service) {
    QTreeWidgetItem* item = new QTreeWidgetItem;

    // Get the same text and the same icon as the service tree
    item->setText(0, service.value().item->text());
    item->setIcon(0, service.value().item->icon());

    Qt::CheckState check_state =
        service.value().shown == true ? Qt::Checked : Qt::Unchecked;
    item->setData(0, Qt::CheckStateRole, check_state);
    /* We have to store the constant name of the service */
    item->setData(1, Qt::UserRole, service.key()->name());

    ui_->sources->invisibleRootItem()->addChild(item);
  }

  ui_->sources->invisibleRootItem()->sortChildren(0, Qt::AscendingOrder);
}

void InternetShowSettingsPage::Save() {
  QSettings s;
  s.beginGroup(InternetModel::kSettingsGroup);

  for (int i = 0; i < ui_->sources->invisibleRootItem()->childCount(); ++i) {
    QTreeWidgetItem* item = ui_->sources->invisibleRootItem()->child(i);
    s.setValue(item->data(1, Qt::UserRole).toString(),
               (item->data(0, Qt::CheckStateRole).toBool()));
  }

  s.endGroup();

  dialog()->app()->internet_model()->UpdateServices();
}
