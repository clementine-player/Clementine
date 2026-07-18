/* This file is part of Clementine.
   Copyright 2026

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

#include "megasettingspage.h"

#include <QTreeWidgetItem>

#include "core/application.h"
#include "internet/core/internetmodel.h"
#include "internet/mega/megaservice.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"
#include "ui_megasettingspage.h"

MegaSettingsPage::MegaSettingsPage(SettingsDialog* parent)
    : SettingsPage(parent),
      ui_(new Ui::MegaSettingsPage),
      service_(dialog()->app()->internet_model()->Service<MegaService>()) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("folder-cloud", IconLoader::Base));

  ui_->login_state->AddCredentialGroup(ui_->login_container);

  connect(ui_->login_button, SIGNAL(clicked()), SLOT(LoginClicked()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(LogoutClicked()));
  connect(service_, SIGNAL(Connected()), SLOT(Connected()));
  connect(ui_->refresh_folders_button, SIGNAL(clicked()),
          SLOT(RefreshFoldersClicked()));
  connect(ui_->scan_button, SIGNAL(clicked()), SLOT(ScanClicked()));
  connect(ui_->reset_button, SIGNAL(clicked()), SLOT(ResetClicked()));
  connect(ui_->autoselect_button, SIGNAL(clicked()),
          SLOT(AutoSelectClicked()));

  dialog()->installEventFilter(this);
}

MegaSettingsPage::~MegaSettingsPage() { delete ui_; }

void MegaSettingsPage::Load() {
  QSettings s;
  s.beginGroup(MegaService::kSettingsGroup);

  const QString email = s.value("email").toString();
  const QString password = s.value("password").toString();

  ui_->email->setText(email);
  ui_->password->setText(password);

  if (!email.isEmpty() && !password.isEmpty()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn, email);
  } else {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
  }

  if (service_->IsReady()) {
    // Service is connected: show live folder tree.
    PopulateFolderTree();
  } else {
    // Not connected yet: restore cached folder tree so the UI isn't empty.
    QList<MegaFolderInfo> cached = service_->GetCachedFolderTree();
    if (!cached.isEmpty()) {
      QStringList selected = service_->GetSelectedFolderHandles();
      ui_->folder_tree->clear();
      AddFolderItems(nullptr, cached, selected);
      ui_->folder_group->setEnabled(true);
      ui_->folder_tree->collapseAll();
    }
  }
}

void MegaSettingsPage::Save() {
  QSettings s;
  s.beginGroup(MegaService::kSettingsGroup);
  s.setValue("email", ui_->email->text().trimmed());
  s.setValue("password", ui_->password->text());

  // Save folder selections.
  SaveSelectedFolders();
}

void MegaSettingsPage::LoginClicked() {
  Save();
  service_->Connect();
  ui_->login_button->setEnabled(false);
}

bool MegaSettingsPage::eventFilter(QObject* object, QEvent* event) {
  if (object == dialog() && event->type() == QEvent::Enter) {
    ui_->login_button->setEnabled(true);
    return false;
  }

  return SettingsPage::eventFilter(object, event);
}

void MegaSettingsPage::LogoutClicked() {
  service_->ForgetCredentials();
  ui_->password->clear();
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
  ui_->folder_group->setEnabled(false);
  ui_->folder_tree->clear();
}

void MegaSettingsPage::Connected() {
  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn,
                                ui_->email->text().trimmed());
  PopulateFolderTree();
}

void MegaSettingsPage::RefreshFoldersClicked() {
  if (!service_->IsReady()) {
    // Need to connect first to get a live folder tree.
    service_->Connect();
    // PopulateFolderTree will be called via the Connected() signal.
    return;
  }
  PopulateFolderTree();
}

void MegaSettingsPage::ScanClicked() {
  SaveSelectedFolders();
  service_->ScanSelectedFolders();
}

void MegaSettingsPage::ResetClicked() {
  service_->ResetDatabase();
  ui_->folder_tree->clear();
  if (service_->IsReady()) {
    PopulateFolderTree();
  }
}

void MegaSettingsPage::AutoSelectClicked() {
  if (!service_->IsReady()) return;

  QSet<QString> audio_folders = service_->GetFoldersWithAudio();
  AutoSelectItems(nullptr, audio_folders);
}

void MegaSettingsPage::AutoSelectItems(QTreeWidgetItem* parent,
                                        const QSet<QString>& handles) {
  int count = parent ? parent->childCount()
                     : ui_->folder_tree->topLevelItemCount();
  for (int i = 0; i < count; ++i) {
    QTreeWidgetItem* item =
        parent ? parent->child(i) : ui_->folder_tree->topLevelItem(i);
    QString handle = item->data(0, Qt::UserRole).toString();
    if (handles.contains(handle)) {
      item->setCheckState(0, Qt::Checked);
      // Expand parent chain so the user can see what was selected.
      QTreeWidgetItem* p = item->parent();
      while (p) {
        p->setExpanded(true);
        p = p->parent();
      }
    }
    AutoSelectItems(item, handles);
  }
}

void MegaSettingsPage::PopulateFolderTree() {
  ui_->folder_tree->clear();

  QList<MegaFolderInfo> folders = service_->GetFolderTree();
  QStringList selected = service_->GetSelectedFolderHandles();

  // Cache the tree for future app restarts.
  service_->SaveCachedFolderTree(folders);

  AddFolderItems(nullptr, folders, selected);

  ui_->folder_group->setEnabled(true);
  ui_->folder_tree->collapseAll();
}

void MegaSettingsPage::AddFolderItems(
    QTreeWidgetItem* parent, const QList<MegaFolderInfo>& folders,
    const QStringList& selected) {
  for (const MegaFolderInfo& folder : folders) {
    QTreeWidgetItem* item;
    if (parent) {
      item = new QTreeWidgetItem(parent);
    } else {
      item = new QTreeWidgetItem(ui_->folder_tree);
    }
    item->setText(0, folder.name);
    item->setData(0, Qt::UserRole, folder.handle_b64);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(
        0, selected.contains(folder.handle_b64) ? Qt::Checked : Qt::Unchecked);

    AddFolderItems(item, folder.children, selected);
  }
}

void MegaSettingsPage::CollectSelectedFolders(QTreeWidgetItem* parent,
                                               QStringList& handles) {
  int count = parent ? parent->childCount()
                     : ui_->folder_tree->topLevelItemCount();
  for (int i = 0; i < count; ++i) {
    QTreeWidgetItem* item =
        parent ? parent->child(i) : ui_->folder_tree->topLevelItem(i);
    if (item->checkState(0) == Qt::Checked) {
      handles.append(item->data(0, Qt::UserRole).toString());
    }
    CollectSelectedFolders(item, handles);
  }
}

void MegaSettingsPage::SaveSelectedFolders() {
  QStringList handles;
  CollectSelectedFolders(nullptr, handles);
  service_->SetSelectedFolderHandles(handles);
}
