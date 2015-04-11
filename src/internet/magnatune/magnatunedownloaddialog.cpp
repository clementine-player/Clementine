/* This file is part of Clementine.
   Copyright 2010, 2012, David Sansome <me@davidsansome.com>
   Copyright 2011-2012, 2014, John Maguire <john.maguire@gmail.com>
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

#include "magnatunedownloaddialog.h"

#include <memory>

#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaEnum>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSet>
#include <QSettings>
#include <QXmlStreamReader>
#include <QUrlQuery>

#include "magnatuneservice.h"
#include "internet/core/internetmodel.h"
#include "ui_magnatunedownloaddialog.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"
#include "ui/iconloader.h"
#include "widgets/progressitemdelegate.h"

MagnatuneDownloadDialog::MagnatuneDownloadDialog(MagnatuneService* service,
                                                 QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_MagnatuneDownloadDialog),
      service_(service),
      network_(new NetworkAccessManager(this)),
      current_reply_(nullptr),
      next_row_(0) {
  ui_->setupUi(this);

  setWindowIcon(IconLoader::Load("magnatune", IconLoader::Provider));

  ui_->albums->header()->setResizeMode(QHeaderView::ResizeToContents);
  ui_->albums->header()->setResizeMode(1, QHeaderView::Fixed);
  ui_->albums->header()->resizeSection(1, 150);
  ui_->albums->setItemDelegateForColumn(1, new ProgressItemDelegate(this));

  ui_->button_box->button(QDialogButtonBox::Cancel)->hide();

  connect(ui_->browse, SIGNAL(clicked()), SLOT(Browse()));

  QSettings s;
  s.beginGroup(MagnatuneService::kSettingsGroup);
  ui_->directory->setText(s.value("download_dir", QDir::homePath()).toString());
}

MagnatuneDownloadDialog::~MagnatuneDownloadDialog() {
  delete ui_;
  delete current_reply_;
}

void MagnatuneDownloadDialog::Show(const SongList& songs) {
  ui_->albums->clear();

  QSet<QString> sku_codes;
  for (const Song& song : songs) {
    if (!sku_codes.contains(song.comment())) {
      sku_codes.insert(song.comment());

      QTreeWidgetItem* item = new QTreeWidgetItem(ui_->albums);
      item->setText(0, song.artist() + " - " + song.album());
      item->setData(0, Qt::UserRole, song.comment());
    }
  }

  ui_->button_box->button(QDialogButtonBox::Ok)->show();
  ui_->format->setCurrentIndex(service_->preferred_format());

  show();
}

void MagnatuneDownloadDialog::accept() {
  QFileInfo dir(ui_->directory->text());
  if (!dir.exists() || !dir.isDir() || !dir.isWritable()) {
    ShowError(tr("The directory %1 is not valid").arg(ui_->directory->text()));
    ui_->directory->setFocus();
    return;
  }

  ui_->button_box->button(QDialogButtonBox::Ok)->hide();
  ui_->button_box->button(QDialogButtonBox::Close)->hide();
  ui_->button_box->button(QDialogButtonBox::Cancel)->show();
  ui_->options->setEnabled(false);

  // Reset all the progress bars
  for (int i = 0; i < ui_->albums->topLevelItemCount(); ++i) {
    ui_->albums->topLevelItem(i)->setData(1, Qt::DisplayRole, QVariant());
  }

  next_row_ = 0;
  DownloadNext();
}

void MagnatuneDownloadDialog::DownloadNext() {
  // We have to download an XML file first that gives us the URLs of the actual
  // zipped album.

  if (next_row_ >= ui_->albums->topLevelItemCount()) {
    AllFinished(false);
    return;
  }

  QTreeWidgetItem* item = ui_->albums->topLevelItem(next_row_);
  QString sku = item->data(0, Qt::UserRole).toString();
  item->setData(1, Qt::DisplayRole, tr("Starting..."));

  QUrl url(MagnatuneService::kDownloadUrl);
  url.setUserName(service_->username());
  url.setPassword(service_->password());

  QUrlQuery url_query;
  url_query.addQueryItem("id", MagnatuneService::kPartnerId);
  url_query.addQueryItem("sku", sku);

  current_reply_ = network_->get(QNetworkRequest(url));

  connect(current_reply_, SIGNAL(error(QNetworkReply::NetworkError)),
          SLOT(Error(QNetworkReply::NetworkError)));
  connect(current_reply_, SIGNAL(finished()), SLOT(MetadataFinished()));
}

void MagnatuneDownloadDialog::Error(QNetworkReply::NetworkError e) {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();

  QUrl url = reply->url();
  url.setPassword(QString());

  QMetaEnum error_enum = QNetworkReply::staticMetaObject.enumerator(
      QNetworkReply::staticMetaObject.indexOfEnumerator("NetworkError"));

  QString message = tr("Unable to download %1 (%2)").arg(url.toString()).arg(
      error_enum.valueToKey(e));
  ShowError(message);
}

void MagnatuneDownloadDialog::MetadataFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  reply->deleteLater();

  // The reply isn't valid XML so we can't use QtXML to parse it :(
  QString data = QString::fromUtf8(reply->readAll());

  // Check for errors
  if (data.contains("<ERROR>")) {
    ShowError(tr("There was a problem fetching the metadata from Magnatune"));
    return;
  }

  // Work out what format we want
  QString type;
  switch (ui_->format->currentIndex()) {
    case MagnatuneService::Format_Ogg:
      type = "URL_OGGZIP";
      break;
    case MagnatuneService::Format_Flac:
      type = "URL_FLACZIP";
      break;
    case MagnatuneService::Format_Wav:
      type = "URL_WAVZIP";
      break;
    case MagnatuneService::Format_MP3_VBR:
      type = "URL_VBRZIP";
      break;
    case MagnatuneService::Format_MP3_128:
      type = "URL_128KMP3ZIP";
      break;
  }

  // Parse the XML (lol) to find the URL
  QRegExp re(QString("<%1>([^<]+)</%2>").arg(type, type));
  if (re.indexIn(data) == -1) {
    ShowError(tr("This album is not available in the requested format"));
    return;
  }

  // Munge the URL a bit
  QString url_text = Utilities::DecodeHtmlEntities(re.cap(1));

  QUrl url = QUrl(url_text);
  url.setUserName(service_->username());
  url.setPassword(service_->password());

  qLog(Debug) << "Downloading" << url;

  // Start the actual download
  current_reply_ = network_->get(QNetworkRequest(url));

  connect(current_reply_, SIGNAL(error(QNetworkReply::NetworkError)),
          SLOT(Error(QNetworkReply::NetworkError)));
  connect(current_reply_, SIGNAL(finished()), SLOT(DownloadFinished()));
  connect(current_reply_, SIGNAL(downloadProgress(qint64, qint64)),
          SLOT(DownloadProgress(qint64, qint64)));
  connect(current_reply_, SIGNAL(readyRead()), SLOT(DownloadReadyRead()));

  // Close any open file
  download_file_.reset();

  // Open the output file
  QString output_filename = GetOutputFilename();
  download_file_.reset(new QFile(output_filename));
  if (!download_file_->open(QIODevice::WriteOnly)) {
    ShowError(tr("Couldn't open output file %1").arg(output_filename));
  }
}

void MagnatuneDownloadDialog::DownloadFinished() {
  current_reply_->deleteLater();

  next_row_++;
  DownloadNext();
}

void MagnatuneDownloadDialog::DownloadProgress(qint64 received, qint64 total) {
  int percent = static_cast<float>(received) / total * 100;
  ui_->albums->topLevelItem(next_row_)->setData(1, Qt::DisplayRole, percent);
}

void MagnatuneDownloadDialog::DownloadReadyRead() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  download_file_->write(reply->readAll());
}

void MagnatuneDownloadDialog::ShowError(const QString& message) {
  QMessageBox::critical(this, tr("Error"), message, QMessageBox::Close);
  AllFinished(true);
}

void MagnatuneDownloadDialog::AllFinished(bool error) {
  current_reply_ = nullptr;

  if (error) ui_->button_box->button(QDialogButtonBox::Ok)->show();
  ui_->button_box->button(QDialogButtonBox::Close)->show();
  ui_->button_box->button(QDialogButtonBox::Cancel)->hide();
  ui_->options->setEnabled(true);

  // Make the taskbar flash
  QApplication::alert(this);

  if (!error) {
    QStringList albums;
    for (int i = 0; i < ui_->albums->topLevelItemCount(); ++i) {
      albums << ui_->albums->topLevelItem(i)->text(0);
    }
    emit Finished(albums);
  }
}

void MagnatuneDownloadDialog::Browse() {
  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Download directory"), ui_->directory->text());

  QSettings s;
  s.beginGroup(MagnatuneService::kSettingsGroup);
  s.setValue("download_dir", dir);

  ui_->directory->setText(dir);
}

QString MagnatuneDownloadDialog::GetOutputFilename() {
  QString album = ui_->albums->topLevelItem(next_row_)->text(0);
  QString extension;

  switch (MagnatuneService::PreferredFormat(ui_->format->currentIndex())) {
    case MagnatuneService::Format_Ogg:
      extension = "ogg";
      break;
    case MagnatuneService::Format_Flac:
      extension = "flac";
      break;
    case MagnatuneService::Format_Wav:
      extension = "wav";
      break;
    case MagnatuneService::Format_MP3_VBR:
      extension = "vbr";
      break;
    case MagnatuneService::Format_MP3_128:
      extension = "mp3";
      break;
  }

  return QString("%1/%2-%3.zip").arg(ui_->directory->text(), album, extension);
}

void MagnatuneDownloadDialog::closeEvent(QCloseEvent* e) {
  if (current_reply_ && current_reply_->isRunning()) {
    std::unique_ptr<QMessageBox> message_box(
        new QMessageBox(QMessageBox::Question, tr("Really cancel?"),
                        tr("Closing this window will cancel the download."),
                        QMessageBox::Abort, this));
    message_box->addButton(tr("Don't stop!"), QMessageBox::AcceptRole);

    if (message_box->exec() != QMessageBox::Abort) {
      e->ignore();
    }
  }
}
