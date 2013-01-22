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

#include "crashreporting_config.h"
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

#ifdef Q_OS_DARWIN
  #include "core/mac_startup.h"
#endif


const char* CrashSender::kUploadURL =
    "http://" CRASHREPORTING_HOSTNAME "/api/upload/minidump";

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

  Utilities::ArgList args;
  ClientInfo(&args);
  QByteArray data = Utilities::UrlEncode(args).toAscii();

  // POST the metadata.
  QNetworkReply* reply = network_->post(QNetworkRequest(QUrl(kUploadURL)), data);
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

  int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (code > 400) {
    printf("Metadata upload caused HTTP %d: %s\n", code, reply->readAll().constData());
    progress_->close();
    return;
  }

  // "Parse" the response.
  QUrl minidump_upload_url = QUrl::fromEncoded(reply->readLine());
  QUrl log_upload_url = QUrl::fromEncoded(reply->readLine());

  // Start the uploads.
  if (!minidump_upload_url.isEmpty()) {
    StartUpload(minidump_upload_url, "application/octet-stream", minidump_);
    printf("Uploading crash report to %s\n",
           minidump_upload_url.toEncoded().constData());
  }

  if (!log_upload_url.isEmpty()) {
    StartUpload(log_upload_url, "text/plain", log_);
    printf("Uploading log to %s\n", log_upload_url.toEncoded().constData());
  }

  CheckUploadsFinished();
}

void CrashSender::StartUpload(const QUrl& url, const QString& content_type,
                              QIODevice* file) {
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, content_type);

  QNetworkReply* reply = network_->put(request, file);
  connect(reply, SIGNAL(uploadProgress(qint64,qint64)),
          SLOT(UploadProgress(qint64,qint64)));
  connect(reply, SIGNAL(finished()), SLOT(UploadFinished()));

  UploadState state;
  state.total_size_ = file->size();
  state.progress_ = 0;
  state.done_ = false;
  upload_state_[reply] = state;
}

void CrashSender::UploadProgress(qint64 bytes, qint64 total) {
  UploadState* state = &upload_state_[qobject_cast<QNetworkReply*>(sender())];
  state->progress_ = bytes;
  state->total_size_ = total;
  CheckUploadsFinished();
}

void CrashSender::UploadFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);

  reply->deleteLater();

  int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (code != 200) {
    printf("Upload caused HTTP %d: %s\n", code, reply->readAll().constData());
  }

  UploadState* state = &upload_state_[reply];
  state->done_ = true;

  CheckUploadsFinished();
}

void CrashSender::CheckUploadsFinished() {
  qint64 total_sum = 0;
  qint64 progress_sum = 0;
  bool downloads_pending = false;

  foreach (const UploadState& state, upload_state_) {
    total_sum += state.total_size_;
    progress_sum += state.progress_;
    if (!state.done_) {
      downloads_pending = true;
    }
  }

  if (!downloads_pending) {
    progress_->close();
  } else {
    progress_->setMaximum(int(total_sum / 1024));
    progress_->setValue(int(progress_sum / 1024));
  }
}

void CrashSender::ClientInfo(Utilities::ArgList* args) const {
  args->append(Utilities::Arg("version", CLEMENTINE_VERSION_DISPLAY));
  args->append(Utilities::Arg("qt_version", qVersion()));

  // Get the OS version
#if defined(Q_OS_MAC)
  args->append(Utilities::Arg("os", "mac"));
  args->append(Utilities::Arg("os_version", mac::GetOSXVersion()));
#elif defined(Q_OS_WIN)
  args->append(Utilities::Arg("os", "win"));
  args->append(Utilities::Arg("os_version", QString::number(QSysInfo::WindowsVersion)));
#else
  args->append(Utilities::Arg("os", "linux"));

  QFile lsb_release("/etc/lsb-release");
  if (lsb_release.open(QIODevice::ReadOnly)) {
    args->append(Utilities::Arg("os_version",
                                QString::fromUtf8(lsb_release.readAll()).simplified()));
  }
#endif
}
