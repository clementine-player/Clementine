/* This file is part of Clementine.
   Copyright 2011-2012, David Sansome <me@davidsansome.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "spotifyblobdownloader.h"

#include "config.h"
#include "spotifyservice.h"
#include "core/arraysize.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QMessageBox>
#include <QNetworkReply>
#include <QProgressDialog>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

#ifdef HAVE_CRYPTOPP
#include <cryptopp/pkcspad.h>
#include <cryptopp/rsa.h>
#endif  // HAVE_CRYPTOPP

namespace {
static const char PEM_HEADER[] = "-----BEGIN PUBLIC KEY-----\n";
static const char PEM_FOOTER[] = "\n-----END PUBLIC KEY-----";
}

const char* SpotifyBlobDownloader::kSignatureSuffix = ".sha512";

SpotifyBlobDownloader::SpotifyBlobDownloader(const QString& version,
                                             const QString& path,
                                             QObject* parent)
    : QObject(parent),
      version_(version),
      path_(path),
      network_(new NetworkAccessManager(this)),
      progress_(new QProgressDialog(tr("Downloading Spotify plugin"),
                                    tr("Cancel"), 0, 0)) {
  progress_->setWindowTitle(QCoreApplication::applicationName());
  connect(progress_, SIGNAL(canceled()), SLOT(Cancel()));
}

SpotifyBlobDownloader::~SpotifyBlobDownloader() {
  qDeleteAll(replies_);
  replies_.clear();

  delete progress_;
}

bool SpotifyBlobDownloader::Prompt() {
  QMessageBox::StandardButton ret = QMessageBox::question(
      nullptr, tr("Spotify plugin not installed"),
      tr("An additional plugin is required to use Spotify in Clementine.  "
         "Would you like to download and install it now?"),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
  return ret == QMessageBox::Yes;
}

void SpotifyBlobDownloader::Start() {
  qDeleteAll(replies_);
  replies_.clear();

  const QStringList filenames =
      QStringList() << "blob"
                    << "blob" + QString(kSignatureSuffix)
                    << "libspotify.so.12.1.45"
                    << "libspotify.so.12.1.45" + QString(kSignatureSuffix);

  for (const QString& filename : filenames) {
    const QUrl url(SpotifyService::kBlobDownloadUrl + version_ + "/" +
                   filename);
    qLog(Info) << "Downloading" << url;

    QNetworkReply* reply = network_->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), SLOT(ReplyFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            SLOT(ReplyProgress()));

    replies_ << reply;
  }

  progress_->show();
}

void SpotifyBlobDownloader::ReplyFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply->error() != QNetworkReply::NoError) {
    // Handle network errors
    ShowError(reply->errorString());
    return;
  }

  // Is everything finished?
  for (QNetworkReply* reply : replies_) {
    if (!reply->isFinished()) {
      return;
    }
  }

  // Read files into memory first.
  QMap<QString, QByteArray> file_data;
  QStringList signature_filenames;

  for (QNetworkReply* reply : replies_) {
    const QString filename = reply->url().path().section('/', -1, -1);

    if (filename.endsWith(kSignatureSuffix)) {
      signature_filenames << filename;
    }

    file_data[filename] = reply->readAll();
  }

  if (!CheckSignature(file_data, signature_filenames)) {
    qLog(Warning) << "Signature checks failed";
    return;
  }

  // Make the destination directory and write the files into it
  QDir().mkpath(path_);

  for (const QString& filename : file_data.keys()) {
    const QString dest_path = path_ + "/" + filename;

    if (filename.endsWith(kSignatureSuffix)) continue;

    qLog(Info) << "Writing" << dest_path;

    QFile file(dest_path);
    if (!file.open(QIODevice::WriteOnly)) {
      ShowError("Failed to open " + dest_path + " for writing");
      return;
    }

    file.write(file_data[filename]);
    file.close();
    file.setPermissions(QFile::Permissions(0x7755));

#ifdef Q_OS_UNIX
    const int so_pos = filename.lastIndexOf(".so.");
    if (so_pos != -1) {
      QString link_path = path_ + "/" + filename.left(so_pos + 3);
      QStringList version_parts = filename.mid(so_pos + 4).split('.');

      while (!version_parts.isEmpty()) {
        qLog(Debug) << "Linking" << dest_path << "to" << link_path;
        int ret = symlink(dest_path.toLocal8Bit().constData(),
                          link_path.toLocal8Bit().constData());

        if (ret != 0) {
          qLog(Warning) << "Creating symlink failed with return code" << ret;
        }

        link_path += "." + version_parts.takeFirst();
      }
    }
#endif  // Q_OS_UNIX
  }

  EmitFinished();
}

bool SpotifyBlobDownloader::CheckSignature(
    const QMap<QString, QByteArray>& file_data,
    const QStringList& signature_filenames) {
#ifdef HAVE_CRYPTOPP
  QFile public_key_file(":/clementine-spotify-public.pem");
  public_key_file.open(QIODevice::ReadOnly);
  QByteArray public_key_data = ConvertPEMToDER(public_key_file.readAll());

  try {
    CryptoPP::ByteQueue bytes;
    bytes.Put(reinterpret_cast<const byte*>(public_key_data.constData()),
              public_key_data.size());
    bytes.MessageEnd();

    CryptoPP::RSA::PublicKey public_key;
    public_key.Load(bytes);

    CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA512>::Verifier verifier(
        public_key);

    for (const QString& signature_filename : signature_filenames) {
      QString actual_filename = signature_filename;
      actual_filename.remove(kSignatureSuffix);

      const bool result = verifier.VerifyMessage(
          reinterpret_cast<const byte*>(file_data[actual_filename].constData()),
          file_data[actual_filename].size(),
          reinterpret_cast<const byte*>(
              file_data[signature_filename].constData()),
          file_data[signature_filename].size());
      qLog(Debug) << "Verifying" << actual_filename << "against"
                  << signature_filename << result;
      if (!result) {
        ShowError("Invalid signature: " + actual_filename);
        return false;
      }
    }
  } catch (std::exception e) {
    // This should only happen if we fail to parse our own key.
    qLog(Debug) << "Verifying spotify blob signature failed:" << e.what();
    return false;
  }
  return true;
#else
  return false;
#endif  // HAVE_CRYPTOPP
}

QByteArray SpotifyBlobDownloader::ConvertPEMToDER(const QByteArray& pem) {
  // Ensure we used char[] not char*
  static_assert(sizeof(PEM_HEADER) > sizeof(char*), "PEM_HEADER mis-declared");
  static_assert(sizeof(PEM_FOOTER) > sizeof(char*), "PEM_FOOTER mis-declared");
  int start = pem.indexOf(PEM_HEADER) + arraysize(PEM_HEADER) - 1;
  int length = pem.indexOf(PEM_FOOTER) - start;
  return QByteArray::fromBase64(pem.mid(start, length));
}

void SpotifyBlobDownloader::ReplyProgress() {
  int progress = 0;
  int total = 0;

  for (QNetworkReply* reply : replies_) {
    progress += reply->bytesAvailable();
    total += reply->rawHeader("Content-Length").toInt();
  }

  progress_->setMaximum(total);
  progress_->setValue(progress);
}

void SpotifyBlobDownloader::Cancel() { deleteLater(); }

void SpotifyBlobDownloader::ShowError(const QString& message) {
  // Stop any remaining replies before showing the dialog so they don't
  // carry on in the background
  for (QNetworkReply* reply : replies_) {
    disconnect(reply, 0, this, 0);
    reply->abort();
  }

  qLog(Warning) << message;
  QMessageBox::warning(nullptr, tr("Error downloading Spotify plugin"), message,
                       QMessageBox::Close);
  deleteLater();
}

void SpotifyBlobDownloader::EmitFinished() {
  emit Finished();
  deleteLater();
}
