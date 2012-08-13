/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   
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

#include "googledriveclient.h"
#include "googledrivefoldermodel.h"
#include "googledriveservice.h"
#include "googledrivesettingspage.h"
#include "ui_googledrivesettingspage.h"
#include "core/application.h"
#include "internet/internetmodel.h"
#include "ui/settingsdialog.h"

#include <QSortFilterProxyModel>

GoogleDriveSettingsPage::GoogleDriveSettingsPage(SettingsDialog* parent)
  : SettingsPage(parent),
    ui_(new Ui::GoogleDriveSettingsPage),
    model_(NULL),
    proxy_model_(NULL),
    item_needs_selecting_(false)
{
  ui_->setupUi(this);
}

GoogleDriveSettingsPage::~GoogleDriveSettingsPage() {
  delete ui_;
}

void GoogleDriveSettingsPage::Load() {
  QSettings s;
  s.beginGroup(GoogleDriveService::kSettingsGroup);

  destination_folder_id_ = s.value("destination_folder_id").toString();
  item_needs_selecting_ = !destination_folder_id_.isEmpty();

  if (!model_) {
    GoogleDriveService* service =
        dialog()->app()->internet_model()->Service<GoogleDriveService>();
    google_drive::Client* client = service->client();

    model_ = new google_drive::FolderModel(client, this);
    proxy_model_ = new QSortFilterProxyModel(this);
    proxy_model_->setSourceModel(model_);
    proxy_model_->setDynamicSortFilter(true);
    proxy_model_->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy_model_->sort(0);

    ui_->upload_destination->setModel(proxy_model_);

    connect(model_, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(DirectoryRowsInserted(QModelIndex)));
  }
}

void GoogleDriveSettingsPage::Save() {
  QSettings s;
  s.beginGroup(GoogleDriveService::kSettingsGroup);

  s.setValue("destination_folder_id",
      ui_->upload_destination->currentIndex().data(
          google_drive::FolderModel::Role_Id).toString());
}

void GoogleDriveSettingsPage::DirectoryRowsInserted(const QModelIndex& parent) {
  ui_->upload_destination->expand(proxy_model_->mapFromSource(parent));

  if (item_needs_selecting_) {
    QStandardItem* item = model_->ItemById(destination_folder_id_);
    if (item) {
      ui_->upload_destination->selectionModel()->select(
            proxy_model_->mapFromSource(item->index()),
            QItemSelectionModel::ClearAndSelect);
      item_needs_selecting_ = false;
    }
  }
}
