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

#include "spotifyconfig.h"

#include "core/network.h"
#include "spotifyservice.h"
#include "radiomodel.h"
#include "ui_spotifyconfig.h"

#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QtDebug>

SpotifyConfig::SpotifyConfig(QWidget *parent)
  : QWidget(parent),
    network_(new NetworkAccessManager(this)),
    ui_(new Ui_SpotifyConfig),
    service_(RadioModel::Service<SpotifyService>()),
    needs_validation_(true)
{
  ui_->setupUi(this);
  ui_->busy->hide();
  connect(service_, SIGNAL(LoginFinished(bool)), SLOT(LoginFinished(bool)));
}

SpotifyConfig::~SpotifyConfig() {
  delete ui_;
}

bool SpotifyConfig::NeedsValidation() const {
  // FIXME
  return needs_validation_;
}

void SpotifyConfig::Validate() {
  ui_->busy->show();
  service_->Login(ui_->username->text(), ui_->password->text());
}

void SpotifyConfig::Load() {
  QSettings s;
  s.beginGroup(SpotifyService::kSettingsGroup);

  ui_->username->setText(s.value("username").toString());
  ui_->password->setText(s.value("password").toString());
}

void SpotifyConfig::Save() {
  QSettings s;
  s.beginGroup(SpotifyService::kSettingsGroup);

  s.setValue("username", ui_->username->text());
  s.setValue("password", ui_->password->text());

  RadioModel::Service<SpotifyService>()->ReloadSettings();
}

void SpotifyConfig::LoginFinished(bool success) {
  qDebug() << Q_FUNC_INFO << success;
  needs_validation_ = !success;
  ui_->busy->hide();
  emit ValidationComplete(success);
}
