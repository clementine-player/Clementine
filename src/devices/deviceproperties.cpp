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

#include "devicelister.h"
#include "devicemanager.h"
#include "deviceproperties.h"
#include "ui_deviceproperties.h"
#include "core/utilities.h"
#include "ui/iconloader.h"

#include <QScrollBar>

DeviceProperties::DeviceProperties(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_DeviceProperties),
    manager_(NULL)
{
  ui_->setupUi(this);

  // Maximum height of the icon widget
  ui_->icon->setMaximumHeight(ui_->icon->iconSize().height() +
                              ui_->icon->horizontalScrollBar()->sizeHint().height() +
                              ui_->icon->spacing() * 2 + 5);
}

DeviceProperties::~DeviceProperties() {
  delete ui_;
}

void DeviceProperties::SetDeviceManager(DeviceManager *manager) {
  manager_ = manager;
  connect(manager_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(ModelChanged()));
  connect(manager_, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(ModelChanged()));
  connect(manager_, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(ModelChanged()));
}

void DeviceProperties::ShowDevice(int row) {
  // Only load the icons the first time the dialog is shown
  if (ui_->icon->count() == 0) {
    QStringList icon_names = QStringList()
        << "drive-removable-media-usb-pendrive"
        << "multimedia-player-ipod-mini-blue"
        << "multimedia-player-ipod-mini-gold"
        << "multimedia-player-ipod-mini-green"
        << "multimedia-player-ipod-mini-pink"
        << "multimedia-player-ipod-mini-silver"
        << "multimedia-player-ipod-nano-black"
        << "multimedia-player-ipod-nano-white"
        << "multimedia-player-ipod-shuffle"
        << "multimedia-player-ipod-standard-color"
        << "multimedia-player-ipod-standard-monochrome"
        << "multimedia-player-ipod-U2-color"
        << "multimedia-player-ipod-U2-monochrome"
        << "phone"
        << "phone-google-nexus-one"
        << "phone-htc-g1-white"
        << "phone-nokia-n900"
        << "phone-palm-pre";

    foreach (const QString& icon_name, icon_names) {
      QListWidgetItem* item = new QListWidgetItem(
          IconLoader::Load(icon_name), QString(), ui_->icon);
      item->setData(Qt::UserRole, icon_name);
    }
  }

  index_ = manager_->index(row);

  // Basic information
  ui_->name->setText(index_.data(DeviceManager::Role_FriendlyName).toString());

  // Find the right icon
  QString icon_name = index_.data(DeviceManager::Role_IconName).toString();
  for (int i=0 ; i<ui_->icon->count() ; ++i) {
    if (ui_->icon->item(i)->data(Qt::UserRole).toString() == icon_name) {
      ui_->icon->setCurrentRow(i);
      break;
    }
  }

  UpdateHardwareInfo();

  show();
}

void DeviceProperties::AddHardwareInfo(int row, const QString &key, const QString &value) {
  ui_->hardware_info->setItem(row, 0, new QTableWidgetItem(key));
  ui_->hardware_info->setItem(row, 1, new QTableWidgetItem(value));
}

void DeviceProperties::ModelChanged() {
  if (!isVisible())
    return;

  if (!index_.isValid())
    reject(); // Device went away
  else
    UpdateHardwareInfo();
}

void DeviceProperties::UpdateHardwareInfo() {
  // Hardware information
  QString id = index_.data(DeviceManager::Role_UniqueId).toString();
  if (DeviceLister* lister = manager_->GetLister(index_.row())) {
    QVariantMap info = lister->DeviceHardwareInfo(id);

    ui_->hardware_info_stack->setCurrentWidget(ui_->hardware_info_page);
    ui_->hardware_info->clear();
    ui_->hardware_info->setRowCount(2 + info.count());

    int row = 0;
    AddHardwareInfo(row++, tr("Model"), lister->DeviceModel(id));
    AddHardwareInfo(row++, tr("Manufacturer"), lister->DeviceManufacturer(id));
    foreach (const QString& key, info.keys()) {
      AddHardwareInfo(row++, tr(key.toAscii()), info[key].toString());
    }

    ui_->hardware_info->sortItems(0);
  } else {
    ui_->hardware_info_stack->setCurrentWidget(ui_->hardware_info_not_connected_page);
  }

  // Size
  quint64 total = index_.data(DeviceManager::Role_Capacity).toLongLong();
  QString total_text = Utilities::PrettySize(total);

  QVariant free_var = index_.data(DeviceManager::Role_FreeSpace);
  if (free_var.isValid()) {
    quint64 free = free_var.toLongLong();
    QString free_text = Utilities::PrettySize(free);

    ui_->free_space_label->setText(tr("Available space"));
    ui_->free_space_value->setText(tr("%1 of %2").arg(free_text, total_text));
    ui_->free_space_bar->set_total_bytes(total);
    ui_->free_space_bar->set_free_bytes(free);
    ui_->free_space_bar->show();
  } else {
    ui_->free_space_label->setText(tr("Capacity"));
    ui_->free_space_value->setText(total_text);
    ui_->free_space_bar->hide();
  }
}

void DeviceProperties::accept() {
  QDialog::accept();

  manager_->SetDeviceIdentity(index_.row(), ui_->name->text(),
                              ui_->icon->currentItem()->data(Qt::UserRole).toString());
}

