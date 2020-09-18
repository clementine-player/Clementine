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

#include "coverfromurldialog.h"

#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "core/network.h"
#include "covers/albumcoverloader.h"
#include "ui_coverfromurldialog.h"

CoverFromURLDialog::CoverFromURLDialog(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_CoverFromURLDialog),
      network_(new NetworkAccessManager(this)) {
  ui_->setupUi(this);
  ui_->busy->hide();
}

CoverFromURLDialog::~CoverFromURLDialog() { delete ui_; }

QImage CoverFromURLDialog::Exec() {
  // reset state
  ui_->url->setText("");
  ;
  last_image_ = QImage();

  QClipboard* clipboard = QApplication::clipboard();
  ui_->url->setText(clipboard->text());

  exec();
  return last_image_;
}

void CoverFromURLDialog::accept() {
  ui_->busy->show();

  QNetworkRequest network_request =
      QNetworkRequest(QUrl::fromUserInput(ui_->url->text()));

  QNetworkReply* reply = network_->get(network_request);
  connect(reply, SIGNAL(finished()), SLOT(LoadCoverFromURLFinished()));
}

void CoverFromURLDialog::LoadCoverFromURLFinished() {
  ui_->busy->hide();

  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    QMessageBox::information(this, tr("Fetching cover error"),
                             tr("The site you requested does not exist!"));
    return;
  }

  QImage image;
  image.loadFromData(reply->readAll());

  if (!image.isNull()) {
    last_image_ = image;
    QDialog::accept();
  } else {
    QMessageBox::information(this, tr("Fetching cover error"),
                             tr("The site you requested is not an image!"));
  }
}
