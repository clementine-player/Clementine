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

#include "deviceproperties.h"

#include <functional>
#include <memory>

#include <QScrollBar>
#include <QtConcurrentRun>

#include "connecteddevice.h"
#include "devicelister.h"
#include "devicemanager.h"
#include "ui_deviceproperties.h"
#include "core/utilities.h"
#include "transcoder/transcoder.h"
#include "ui/iconloader.h"

DeviceProperties::DeviceProperties(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_DeviceProperties),
      manager_(nullptr),
      updating_formats_(false) {
  ui_->setupUi(this);

  connect(ui_->open_device, SIGNAL(clicked()), SLOT(OpenDevice()));

  // Maximum height of the icon widget
  ui_->icon->setMaximumHeight(
      ui_->icon->iconSize().height() +
      ui_->icon->horizontalScrollBar()->sizeHint().height() +
      ui_->icon->spacing() * 2 + 5);
}

DeviceProperties::~DeviceProperties() { delete ui_; }

void DeviceProperties::SetDeviceManager(DeviceManager* manager) {
  manager_ = manager;
  connect(manager_, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
          SLOT(ModelChanged()));
  connect(manager_, SIGNAL(rowsInserted(QModelIndex, int, int)),
          SLOT(ModelChanged()));
  connect(manager_, SIGNAL(rowsRemoved(QModelIndex, int, int)),
          SLOT(ModelChanged()));
}

void DeviceProperties::ShowDevice(QModelIndex idx) {
  if (ui_->icon->count() == 0) {
    // Only load the icons the first time the dialog is shown
    QStringList icon_names = QStringList()
                             << "drive-removable-media-usb-pendrive"
                             << "multimedia-player-ipod-mini-blue"
                             << "multimedia-player-ipod-mini-gold"
                             << "multimedia-player-ipod-mini-green"
                             << "multimedia-player-ipod-mini-pink"
                             << "multimedia-player-ipod-mini-silver"
                             << "multimedia-player-ipod-nano-black"
                             << "multimedia-player-ipod-nano-white"
                             << "multimedia-player-ipod-nano-green"
                             << "multimedia-player-ipod-shuffle"
                             << "multimedia-player-ipod-standard-color"
                             << "multimedia-player-ipod-standard-monochrome"
                             << "multimedia-player-ipod-U2-color"
                             << "multimedia-player-ipod-U2-monochrome"
                             << "ipodtouchicon"
                             << "phone"
                             << "phone-google-nexus-one"
                             << "phone-htc-g1-white"
                             << "phone-nokia-n900"
                             << "phone-palm-pre";

    for (const QString& icon_name : icon_names) {
      QListWidgetItem* item = new QListWidgetItem(
          IconLoader::Load(icon_name, IconLoader::Base), QString(), ui_->icon);
      item->setData(Qt::UserRole, icon_name);
    }

    // Load the transcode formats the first time the dialog is shown
    for (const TranscoderPreset& preset : Transcoder::GetAllPresets()) {
      ui_->transcode_format->addItem(preset.name_, preset.type_);
    }
    ui_->transcode_format->model()->sort(0);
  }

  index_ = idx;

  // Basic information
  ui_->name->setText(index_.data(DeviceManager::Role_FriendlyName).toString());

  // Find the right icon
  QString icon_name = index_.data(DeviceManager::Role_IconName).toString();
  for (int i = 0; i < ui_->icon->count(); ++i) {
    if (ui_->icon->item(i)->data(Qt::UserRole).toString() == icon_name) {
      ui_->icon->setCurrentRow(i);
      break;
    }
  }

  UpdateHardwareInfo();
  UpdateFormats();

  show();
}

void DeviceProperties::AddHardwareInfo(int row, const QString& key,
                                       const QString& value) {
  ui_->hardware_info->setItem(row, 0, new QTableWidgetItem(key));
  ui_->hardware_info->setItem(row, 1, new QTableWidgetItem(value));
}

void DeviceProperties::ModelChanged() {
  if (!isVisible()) return;

  if (!index_.isValid())
    reject();  // Device went away
  else {
    UpdateHardwareInfo();
    UpdateFormats();
  }
}

void DeviceProperties::UpdateHardwareInfo() {
  // Hardware information
  QString id = index_.data(DeviceManager::Role_UniqueId).toString();
  if (DeviceLister* lister = manager_->GetLister(index_)) {
    QVariantMap info = lister->DeviceHardwareInfo(id);

    // Remove empty items
    for (const QString& key : info.keys()) {
      if (info[key].isNull() || info[key].toString().isEmpty())
        info.remove(key);
    }

    ui_->hardware_info_stack->setCurrentWidget(ui_->hardware_info_page);
    ui_->hardware_info->clear();
    ui_->hardware_info->setRowCount(2 + info.count());

    int row = 0;
    AddHardwareInfo(row++, tr("Model"), lister->DeviceModel(id));
    AddHardwareInfo(row++, tr("Manufacturer"), lister->DeviceManufacturer(id));
    for (const QString& key : info.keys()) {
      AddHardwareInfo(row++, tr(key.toAscii()), info[key].toString());
    }

    ui_->hardware_info->sortItems(0);
  } else {
    ui_->hardware_info_stack->setCurrentWidget(
        ui_->hardware_info_not_connected_page);
  }

  // Size
  quint64 total = index_.data(DeviceManager::Role_Capacity).toLongLong();

  QVariant free_var = index_.data(DeviceManager::Role_FreeSpace);
  if (free_var.isValid()) {
    quint64 free = free_var.toLongLong();

    ui_->free_space_bar->set_total_bytes(total);
    ui_->free_space_bar->set_free_bytes(free);
    ui_->free_space_bar->show();
  } else {
    ui_->free_space_bar->hide();
  }
}

void DeviceProperties::UpdateFormats() {
  QString id = index_.data(DeviceManager::Role_UniqueId).toString();
  DeviceLister* lister = manager_->GetLister(index_);
  std::shared_ptr<ConnectedDevice> device =
      manager_->GetConnectedDevice(index_);

  // Transcode mode
  MusicStorage::TranscodeMode mode = MusicStorage::TranscodeMode(
      index_.data(DeviceManager::Role_TranscodeMode).toInt());
  switch (mode) {
    case MusicStorage::Transcode_Always:
      ui_->transcode_all->setChecked(true);
      break;

    case MusicStorage::Transcode_Never:
      ui_->transcode_off->setChecked(true);
      break;

    case MusicStorage::Transcode_Unsupported:
    default:
      ui_->transcode_unsupported->setChecked(true);
      break;
  }

  // If there's no lister then the device is physically disconnected
  if (!lister) {
    ui_->formats_stack->setCurrentWidget(ui_->formats_page_not_connected);
    ui_->open_device->setEnabled(false);
    return;
  }

  // If there's a lister but no device then the user just needs to open the
  // device.  This will cause a rescan so we don't do it automatically.
  if (!device) {
    ui_->formats_stack->setCurrentWidget(ui_->formats_page_not_connected);
    ui_->open_device->setEnabled(true);
    return;
  }

  if (!updating_formats_) {
    // Get the device's supported formats list.  This takes a long time and it
    // blocks, so do it in the background.
    supported_formats_.clear();

    QFuture<bool> future = QtConcurrent::run(std::bind(
        &ConnectedDevice::GetSupportedFiletypes, device, &supported_formats_));
    NewClosure(future, this, SLOT(UpdateFormatsFinished(QFuture<bool>)),
               future);

    ui_->formats_stack->setCurrentWidget(ui_->formats_page_loading);
    updating_formats_ = true;
  }
}

void DeviceProperties::accept() {
  QDialog::accept();

  // Transcode mode
  MusicStorage::TranscodeMode mode = MusicStorage::Transcode_Unsupported;
  if (ui_->transcode_all->isChecked())
    mode = MusicStorage::Transcode_Always;
  else if (ui_->transcode_off->isChecked())
    mode = MusicStorage::Transcode_Never;
  else if (ui_->transcode_unsupported->isChecked())
    mode = MusicStorage::Transcode_Unsupported;

  // Transcode format
  Song::FileType format = Song::FileType(
      ui_->transcode_format->itemData(ui_->transcode_format->currentIndex())
          .toInt());

  // By default no icon is selected and thus no current item is selected
  QString icon_name;
  if (ui_->icon->currentItem() != nullptr) {
    icon_name = ui_->icon->currentItem()->data(Qt::UserRole).toString();
  }

  manager_->SetDeviceOptions(index_, ui_->name->text(), icon_name, mode,
                             format);
}

void DeviceProperties::OpenDevice() { manager_->Connect(index_); }

void DeviceProperties::UpdateFormatsFinished(QFuture<bool> future) {
  updating_formats_ = false;

  if (!future.result()) {
    supported_formats_.clear();
  }

  // Hide widgets if there are no supported types
  ui_->supported_formats_container->setVisible(!supported_formats_.isEmpty());
  ui_->transcode_unsupported->setEnabled(!supported_formats_.isEmpty());

  if (ui_->transcode_unsupported->isChecked() && supported_formats_.isEmpty()) {
    ui_->transcode_off->setChecked(true);
  }

  // Populate supported types list
  ui_->supported_formats->clear();
  for (Song::FileType type : supported_formats_) {
    QListWidgetItem* item = new QListWidgetItem(Song::TextForFiletype(type));
    ui_->supported_formats->addItem(item);
  }
  ui_->supported_formats->sortItems();

  // Set the format combobox item
  TranscoderPreset preset = Transcoder::PresetForFileType(
      Song::FileType(index_.data(DeviceManager::Role_TranscodeFormat).toInt()));
  if (preset.type_ == Song::Type_Unknown) {
    // The user hasn't chosen a format for this device yet, so work our way down
    // a list of some preferred formats, picking the first one that is supported
    preset = Transcoder::PresetForFileType(
        Transcoder::PickBestFormat(supported_formats_));
  }
  ui_->transcode_format->setCurrentIndex(
      ui_->transcode_format->findText(preset.name_));

  ui_->formats_stack->setCurrentWidget(ui_->formats_page);
}
