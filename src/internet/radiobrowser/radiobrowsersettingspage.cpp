/* This file is part of Clementine.
   Copyright 2021, Fabio Bas <ctrlaltca@gmail.com>

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

#include "radiobrowsersettingspage.h"

#include <QSettings>

#include "core/logging.h"
#include "internet/core/internetmodel.h"
#include "ui/iconloader.h"
#include "ui_radiobrowsersettingspage.h"

const QString RadioBrowserSettingsPage::defaultServer_ =
    QStringLiteral("http://all.api.radio-browser.info");

RadioBrowserSettingsPage::RadioBrowserSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog),
      ui_(new Ui_RadioBrowserSettingsPage),
      service_(InternetModel::Service<RadioBrowserService>()) {
  ui_->setupUi(this);
  setWindowIcon(IconLoader::Load("radiobrowser", IconLoader::Provider));

  connect(ui_->server, SIGNAL(editingFinished()),
          SLOT(ServerEditingFinished()));
  connect(ui_->restoreButton, SIGNAL(clicked()), this,
          SLOT(RestoreDefaultServer()));
}

RadioBrowserSettingsPage::~RadioBrowserSettingsPage() { delete ui_; }

void RadioBrowserSettingsPage::Load() {
  QSettings s;
  s.beginGroup(RadioBrowserService::kSettingsGroup);

  ui_->server->setText(
      s.value("server", RadioBrowserSettingsPage::defaultServer_).toString());
}

void RadioBrowserSettingsPage::Save() {
  QSettings s;
  s.beginGroup(RadioBrowserService::kSettingsGroup);

  s.setValue("server", ui_->server->text());
}

void RadioBrowserSettingsPage::ServerEditingFinished() {
  QString input = ui_->server->text();
  QUrl url = QUrl::fromUserInput(input);

  // Veto things that don't get guessed as an HTTP URL, the result will be
  // unhelpful
  if (!url.scheme().startsWith("http")) {
    return;
  }

  ui_->server->setText(url.toString());
  qLog(Debug) << "URL fixed:" << input << "to" << url;
  service_->ReloadSettings();
}

void RadioBrowserSettingsPage::RestoreDefaultServer() {
  ui_->server->setText(RadioBrowserSettingsPage::defaultServer_);
  service_->ReloadSettings();
}