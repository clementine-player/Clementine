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

#include "config.h"
#include "crashreporting.h"
#include "core/logging.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QUrl>
#include <QtDebug>

#if QT_VERSION >= 0x040800
  #include <QHttpMultiPart>
#endif


const char* CrashSender::kUploadURL = "http://crashes.clementine-player.org/getuploadurl";
const char* CrashReporting::kSendCrashReportOption = "--send-crash-report";
char* CrashReporting::sPath = NULL;


bool CrashReporting::SendCrashReport(int argc, char** argv) {
  if (argc != 4 || strcmp(argv[1], kSendCrashReportOption) != 0) {
    return false;
  }

  QApplication a(argc, argv);

  CrashSender sender(QString("%1/%2.dmp").arg(argv[2], argv[3]));
  if (sender.Start()) {
    a.exec();
  }

  return true;
}

void CrashReporting::SetApplicationPath(const QString& path) {
  sPath = strdup(path.toLocal8Bit().constData());
}


CrashSender::CrashSender(const QString& path)
  : network_(new QNetworkAccessManager(this)),
    path_(path),
    file_(new QFile(path_, this)),
    progress_(NULL) {
}

bool CrashSender::Start() {
  if (!file_->open(QIODevice::ReadOnly)) {
    qLog(Warning) << "Failed to open crash report" << path_;
    return false;
  }

  // No tr() here.
  QMessageBox prompt(QMessageBox::Warning, "Clementine has crashed!",
      "Clementine has crashed!  A crash report has been created and saved to "
      "disk.  With your permission it can be automatically sent to our server "
      "so the developers can find out what happened.");
  prompt.addButton("Don't send", QMessageBox::RejectRole);
  prompt.addButton("Send crash report", QMessageBox::AcceptRole);
  if (prompt.exec() == QDialog::Rejected) {
    return false;
  }

  progress_ = new QProgressDialog("Uploading crash report", "Cancel", 0, 0);
  progress_->show();

  // We'll get a redirect first, so don't start POSTing data yet.
  QNetworkReply* reply = network_->get(QNetworkRequest(QUrl(kUploadURL)));
  connect(reply, SIGNAL(finished()), SLOT(RedirectFinished()));

  return true;
}

void CrashSender::RedirectFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    progress_->close();
    return;
  }

  reply->deleteLater();

  QUrl url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
  if (!url.isValid()) {
    printf("Response didn't have a redirection target - HTTP %d\n",
           reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
    progress_->close();
    return;
  }

  printf("Uploading crash report to %s\n", url.toEncoded().constData());

  QNetworkRequest req(url);

#if QT_VERSION >= 0x040800
  QHttpPart part;
  part.setHeader(QNetworkRequest::ContentDispositionHeader,
                 "form-data; name=\"data\", filename=\"data.dmp\"");
  part.setBodyDevice(file_);

  QHttpMultiPart* multi_part = new QHttpMultiPart(QHttpMultiPart::FormDataType);
  multi_part->append(part);

  reply = network_->post(req, multi_part);
#else
  // Read the file's data
  QByteArray file_data = file_->readAll();

  // Find a boundary that doesn't exist in the file
  QByteArray boundary;
  forever {
    boundary = "--------------" + QString::number(qrand(), 16).toAscii();
    if (!file_data.contains(boundary)) {
      break;
    }
  }

  req.setHeader(QNetworkRequest::ContentTypeHeader,
                QString("multipart/form-data; boundary=" + boundary));

  // Construct the multipart/form-data
  QByteArray form_data;
  form_data.reserve(file_data.size() + 1024);
  form_data.append("--");
  form_data.append(boundary);
  form_data.append("\nContent-Disposition: form-data; name=\"data\"; filename=\"data.dmp\"\n");
  form_data.append("Content-Type: application/octet-stream\n\n");
  form_data.append(file_data);
  form_data.append("\n--");
  form_data.append(boundary);
  form_data.append("--");

  progress_->setMaximum(form_data.size());

  // Upload the data
  reply = network_->post(req, form_data);
#endif

  connect(reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(UploadProgress(qint64,qint64)));
  connect(reply, SIGNAL(finished()), progress_, SLOT(close()));
}

void CrashSender::UploadProgress(qint64 bytes, qint64 total) {
  printf("Uploaded %lld of %lld bytes\n", bytes, total);
  progress_->setValue(bytes);
}
