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

#include "config.h"
#include "crashsender.h"
#include "version.h"
#include "core/logging.h"

#include <QCoreApplication>
#include <QFile>
#include <QHttpMultiPart>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QSysInfo>
#include <QUrl>


const char* CrashSender::kUploadURL =
    "http://" CRASHREPORTING_HOSTNAME "/upload/crash";

CrashSender::CrashSender(const QString& minidump_filename,
                         const QString& log_filename)
  : network_(new QNetworkAccessManager(this)),
    minidump_filename_(minidump_filename),
    log_filename_(log_filename),
    minidump_(new QFile(minidump_filename_, this)),
    log_(new QFile(log_filename_, this)),
    progress_(NULL) {
}

bool CrashSender::Start() {
  if (!minidump_->open(QIODevice::ReadOnly)) {
    qLog(Warning) << "Failed to open crash report" << minidump_filename_;
    return false;
  }

  if (!log_filename_.isEmpty()) {
    if (!log_->open(QIODevice::ReadOnly)) {
      qLog(Warning) << "Failed to open log file" << log_filename_;
    }
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

  QHttpMultiPart* multi_part = new QHttpMultiPart(QHttpMultiPart::FormDataType);

  // Create the HTTP part for the crash report file
  QHttpPart minidump_part;
  minidump_part.setHeader(QNetworkRequest::ContentDispositionHeader,
                          "form-data; name=\"data\"; filename=\"data.dmp\"");
  minidump_part.setBodyDevice(minidump_);
  multi_part->append(minidump_part);

  // Create the HTTP part for the log file.
  if (log_->isOpen()) {
    QHttpPart log_part;
    log_part.setHeader(QNetworkRequest::ContentDispositionHeader,
                       "form-data; name=\"log\"; filename=\"log.txt\"");
    log_part.setBodyDevice(log_);
    multi_part->append(log_part);
  }

  // Get some information about the thing that crashed and add that to the
  // request as well.
  QList<ClientInfoPair> info(ClientInfo());
  foreach (const ClientInfoPair& pair, info) {
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentDispositionHeader,
                   QString("form-data; name=\"" + pair.first + "\""));
    part.setBody(pair.second.toUtf8());
    multi_part->append(part);
  }

  // Start uploading the crash report
  reply = network_->post(req, multi_part);

  connect(reply, SIGNAL(uploadProgress(qint64,qint64)), SLOT(UploadProgress(qint64,qint64)));
  connect(reply, SIGNAL(finished()), SLOT(UploadFinished()));
}

void CrashSender::UploadProgress(qint64 bytes, qint64 total) {
  printf("Uploaded %lld of %lld bytes\n", bytes, total);
  progress_->setMaximum(total);
  progress_->setValue(bytes);
}

void CrashSender::UploadFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);

  reply->deleteLater();

  int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (code > 400) {
    printf("Upload caused HTTP %d: %s\n", code, reply->readAll().constData());
  }

  progress_->close();
}

QList<CrashSender::ClientInfoPair> CrashSender::ClientInfo() const {
  QList<ClientInfoPair> ret;

  ret.append(ClientInfoPair("version", CLEMENTINE_VERSION_DISPLAY));
  ret.append(ClientInfoPair("qt_version", qVersion()));

  // Get the OS version
#if defined(Q_OS_MAC)
  ret.append(ClientInfoPair("os", "mac"));
  ret.append(ClientInfoPair("os_version", QString::number(QSysInfo::MacintoshVersion)));
#elif defined(Q_OS_WIN)
  ret.append(ClientInfoPair("os", "win"));
  ret.append(ClientInfoPair("os_version", QString::number(QSysInfo::WindowsVersion)));
#else
  ret.append(ClientInfoPair("os", "linux"));

  QFile lsb_release("/etc/lsb-release");
  if (lsb_release.open(QIODevice::ReadOnly)) {
    ret.append(ClientInfoPair("os_version",
                              QString::fromUtf8(lsb_release.readAll()).simplified()));
  }
#endif

  return ret;
}
