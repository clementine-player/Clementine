/* This file is part of Clementine.
   Copyright 2014, Chocobozzz <djidane14ff@hotmail.fr>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#include "seafilesettingspage.h"

#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QtDebug>

#include "seafileservice.h"
#include "internet/core/internetmodel.h"
#include "core/logging.h"
#include "core/network.h"
#include "ui_seafilesettingspage.h"
#include "ui/iconloader.h"

SeafileSettingsPage::SeafileSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_SeafileSettingsPage),
      service_(InternetModel::Service<SeafileService>()) {
  ui_->setupUi(this);

  setWindowIcon(IconLoader::Load("seafile", IconLoader::Provider));

  connect(ui_->login_button, SIGNAL(clicked()), SLOT(Login()));
  connect(ui_->login_state, SIGNAL(LogoutClicked()), SLOT(Logout()));

  ui_->login_state->AddCredentialField(ui_->server);
  ui_->login_state->AddCredentialField(ui_->mail);
  ui_->login_state->AddCredentialField(ui_->password);
  ui_->login_state->AddCredentialGroup(ui_->account_group);

  ui_->library_box->addItem("None", "none");

  connect(service_, SIGNAL(GetLibrariesFinishedSignal(QMap<QString, QString>)),
          this, SLOT(GetLibrariesFinished(QMap<QString, QString>)));
}

SeafileSettingsPage::~SeafileSettingsPage() {}

void SeafileSettingsPage::Load() {
  QSettings s;
  s.beginGroup(SeafileService::kSettingsGroup);

  ui_->server->setText(s.value("server").toString());
  ui_->mail->setText(s.value("mail").toString());

  if (service_->has_credentials()) {
    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn,
                                  ui_->mail->text());

    // If there is more than "none" library, that means that we already got the
    // libraries
    if (ui_->library_box->count() <= 1) {
      service_->GetLibraries();
    }
  }
}

void SeafileSettingsPage::GetLibrariesFinished(
    const QMap<QString, QString>& libraries) {
  ui_->library_box->clear();
  ui_->library_box->addItem("None", "none");
  ui_->library_box->addItem("All (could be slow)", "all");

  // key : library's id, value : library's name
  QMapIterator<QString, QString> library(libraries);
  while (library.hasNext()) {
    library.next();
    ui_->library_box->addItem(library.value(), library.key());
  }

  QSettings s;
  s.beginGroup(SeafileService::kSettingsGroup);
  QString library_id = s.value("library").toString();

  int saved_index = ui_->library_box->findData(library_id);
  if (saved_index != -1) {
    ui_->library_box->setCurrentIndex(saved_index);
  }
}

void SeafileSettingsPage::Save() {
  QString id =
      ui_->library_box->itemData(ui_->library_box->currentIndex()).toString();

  QSettings s;
  s.beginGroup(SeafileService::kSettingsGroup);

  s.setValue("mail", ui_->mail->text());
  s.setValue("server", ui_->server->text());
  s.setValue("library", id);
  // Don't need to save the password

  if (service_->has_credentials()) {
    service_->ChangeLibrary(id);
  }
}

void SeafileSettingsPage::Login() {
  ui_->login_button->setEnabled(false);

  if (service_->GetToken(ui_->mail->text(), ui_->password->text(),
                         ui_->server->text())) {
    Save();

    ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedIn,
                                  ui_->mail->text());

    service_->GetLibraries();
  } else {
    ui_->login_button->setEnabled(true);
    QMessageBox::warning(this, tr("Unable to connect"),
                         tr("Unable to connect"));
  }
}

void SeafileSettingsPage::Logout() {
  // Forget the added songs by changing the library to "none"
  service_->ChangeLibrary("none");
  service_->ForgetCredentials();

  // We choose to keep the server
  ui_->mail->clear();
  ui_->password->clear();

  QSettings s;
  s.beginGroup(SeafileService::kSettingsGroup);

  s.remove("mail");
  s.remove("server");
  s.remove("library");

  ui_->library_box->clear();
  ui_->library_box->addItem("None", "none");

  ui_->login_state->SetLoggedIn(LoginStateWidget::LoggedOut);
  ui_->login_button->setEnabled(true);
}
