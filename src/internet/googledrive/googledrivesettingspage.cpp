/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2013-2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "googledrivesettingspage.h"

#include <QListWidgetItem>
#include <QSortFilterProxyModel>

#include "core/application.h"
#include "googledriveclient.h"
#include "googledriveservice.h"
#include "internet/core/internetmodel.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"
#include "ui_googledrivesettingspage.h"

namespace {
// Matches PickedItem::id, stashed on each QListWidgetItem so RemoveItem()
// knows what to forget.
const int kPickedItemIdRole = Qt::UserRole;
}  // namespace

GoogleDriveSettingsPage::GoogleDriveSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::GoogleDriveSettingsPage),
      service_(
          dialog()->app()->internet_model()->Service<GoogleDriveService>()) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("googledrive", IconLoader::Provider));

  ui_->login_state->AddCredentialGroup(ui_->login_container);
  ui_->drive_items_container->setVisible(false);

  connect(ui_->login_button, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(ui_->add_files_button, SIGNAL(clicked()), SLOT(AddFilesClicked()));
  connect(ui_->remove_item_button, SIGNAL(clicked()),
          SLOT(RemoveItemClicked()));
  connect(service_, SIGNAL(Connected()), SLOT(Connected()));
  connect(service_, SIGNAL(PickedItemsChanged()), SLOT(UpdatePickedItems()));

  dialog()->installEventFilter(this);
}

GoogleDriveSettingsPage::~GoogleDriveSettingsPage() { delete ui_; }

void GoogleDriveSettingsPage::Load() {
  QSettings s;
  s.beginGroup(GoogleDriveService::kSettingsGroup);

  const QString user_email = s.value("user_email").toString();
  const QString refresh_token = s.value("refresh_token").toString();

  // refresh_token is what actually determines whether we're connected (see
  // GoogleDriveService::has_credentials()) - user_email is just display
  // info and shouldn't gate this, or a hiccup fetching it (eg. a failed
  // tokeninfo request) would leave the page permanently showing "logged
  // out" despite functioning normally otherwise.
  if (!refresh_token.isEmpty()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, user_email);
    ui_->drive_items_container->setVisible(true);
  }

  UpdatePickedItems();
}

void GoogleDriveSettingsPage::Save() {
  QSettings s;
  s.beginGroup(GoogleDriveService::kSettingsGroup);
}

void GoogleDriveSettingsPage::LoginClicked() {
  // There's no separate "just log in" step under drive.file scope - a login
  // without picking anything grants access to nothing, so this performs the
  // same combined OAuth+Picker flow as "Add files" below.
  service_->AddFiles();
  ui_->login_button->setEnabled(false);
}

bool GoogleDriveSettingsPage::eventFilter(QObject* object, QEvent* event) {
  if (object == dialog() && event->type() == QEvent::Enter) {
    ui_->login_button->setEnabled(true);
    return false;
  }

  return SettingsPage::eventFilter(object, event);
}

void GoogleDriveSettingsPage::LogoutClicked() {
  service_->ForgetCredentials();
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
  ui_->drive_items_container->setVisible(false);
}

void GoogleDriveSettingsPage::Connected() {
  QSettings s;
  s.beginGroup(GoogleDriveService::kSettingsGroup);

  const QString user_email = s.value("user_email").toString();

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, user_email);
  ui_->drive_items_container->setVisible(true);
}

void GoogleDriveSettingsPage::AddFilesClicked() { service_->AddFiles(); }

void GoogleDriveSettingsPage::RemoveItemClicked() {
  QListWidgetItem* item = ui_->picked_items_list->currentItem();
  if (!item) {
    return;
  }
  service_->RemoveItem(item->data(kPickedItemIdRole).toString());
}

void GoogleDriveSettingsPage::UpdatePickedItems() {
  ui_->picked_items_list->clear();

  for (const GoogleDriveService::PickedItem& picked : service_->picked_items()) {
    QListWidgetItem* item =
        new QListWidgetItem(picked.title, ui_->picked_items_list);
    item->setData(kPickedItemIdRole, picked.id);
  }
}
