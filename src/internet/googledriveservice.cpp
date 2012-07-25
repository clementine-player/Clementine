#include "googledriveservice.h"

#include <QEventLoop>

#include <qjson/parser.h>

#include <taglib/id3v2framefactory.h>
#include <taglib/mpegfile.h>
#include <taglib/tiostream.h>
using TagLib::ByteVector;

#include "core/closure.h"
#include "internetmodel.h"
#include "oauthenticator.h"

namespace {

const char* kGoogleDriveFiles = "https://www.googleapis.com/drive/v2/files";

}


class DriveStream : public TagLib::IOStream {
 public:
  DriveStream(const QUrl& url,
              const QString& filename,
              const long length,
              const QString& auth,
              QNetworkAccessManager* network)
      : url_(url),
        filename_(filename),
        encoded_filename_(filename_.toUtf8()),
        length_(length),
        auth_(auth),
        cursor_(0),
        network_(network) {
    qLog(Debug) << Q_FUNC_INFO;
  }

  virtual TagLib::FileName name() const {
    qLog(Debug) << Q_FUNC_INFO;
    return encoded_filename_.data();
  }

  virtual TagLib::ByteVector readBlock(ulong length) {
    qLog(Debug) << Q_FUNC_INFO;
    QNetworkRequest request = QNetworkRequest(url_);
    request.setRawHeader(
        "Authorization", QString("Bearer %1").arg(auth_).toUtf8());

    const int start = cursor_;
    const int end = cursor_ + length;
    request.setRawHeader(
        "Range", QString("bytes=%1-%2").arg(start, end).toUtf8());

    qLog(Debug) << "Requesting:" << start << "-" << end << "from:" << url_;
    qLog(Debug) << request.rawHeaderList();


    QNetworkReply* reply = network_->get(request);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    qLog(Debug) << "Starting loop";
    loop.exec();
    qLog(Debug) << "Finished loop";
    reply->deleteLater();

    QByteArray data = reply->readAll();
    TagLib::ByteVector bytes(data.data(), data.size());
    return bytes;
  }

  virtual void writeBlock(const ByteVector&) {
    qLog(Debug) << Q_FUNC_INFO << "not implemented";
  }

  virtual void insert(const ByteVector&, ulong, ulong) {
    qLog(Debug) << Q_FUNC_INFO << "not implemented";
  }

  virtual void removeBlock(ulong, ulong) {
    qLog(Debug) << Q_FUNC_INFO << "not implemented";
  }

  virtual bool readOnly() const {
    qLog(Debug) << Q_FUNC_INFO;
    return true;
  }

  virtual bool isOpen() const {
    qLog(Debug) << Q_FUNC_INFO;
    return true;
  }

  virtual void seek(long offset, TagLib::IOStream::Position p) {
    qLog(Debug) << Q_FUNC_INFO;
    switch (p) {
      case TagLib::IOStream::Beginning:
        cursor_ = offset;
        break;

      case TagLib::IOStream::Current:
        cursor_ = qMin(cursor_ + offset, length_);
        break;

      case TagLib::IOStream::End:
        cursor_ = qMax(0L, length_ - offset);
        break;
    }
  }

  virtual void clear() {
    qLog(Debug) << Q_FUNC_INFO;
    cursor_ = 0;
  }

  virtual long tell() const {
    qLog(Debug) << Q_FUNC_INFO;
    return cursor_;
  }

  virtual long length() {
    qLog(Debug) << Q_FUNC_INFO;
    return length_;
  }

  virtual void truncate(long) {
    qLog(Debug) << Q_FUNC_INFO << "not implemented";
  }

 private:
  const QUrl url_;
  const QString filename_;
  const QByteArray encoded_filename_;
  const long length_;
  const QString auth_;

  int cursor_;
  QNetworkAccessManager* network_;
};


GoogleDriveService::GoogleDriveService(Application* app, InternetModel* parent)
    : InternetService("Google Drive", app, parent, parent),
      root_(NULL),
      oauth_(new OAuthenticator(this)) {
  connect(oauth_, SIGNAL(AccessTokenAvailable(QString)), SLOT(AccessTokenAvailable(QString)));
}

QStandardItem* GoogleDriveService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(":providers/googledrive.png"), "Google Drive");
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void GoogleDriveService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      Connect();
      break;
    default:
      break;
  }
}

void GoogleDriveService::Connect() {
  oauth_->StartAuthorisation();
}

void GoogleDriveService::AccessTokenAvailable(const QString& token) {
  access_token_ = token;
  QUrl url = QUrl(kGoogleDriveFiles);
  url.addQueryItem("q", "mimeType = 'audio/mpeg'");

  QNetworkRequest request = QNetworkRequest(url);
  request.setRawHeader(
      "Authorization", QString("Bearer %1").arg(token).toUtf8());
  QNetworkReply* reply = network_.get(request);
  NewClosure(reply, SIGNAL(finished()), this, SLOT(ListFilesFinished(QNetworkReply*)), reply);
}

void GoogleDriveService::ListFilesFinished(QNetworkReply* reply) {
  reply->deleteLater();

  QJson::Parser parser;
  bool ok = false;
  QVariantMap result = parser.parse(reply, &ok).toMap();
  if (!ok) {
    qLog(Error) << "Failed to request files from Google Drive";
    return;
  }

  QVariantList items = result["items"].toList();
  foreach (const QVariant& v, items) {
    QVariantMap file = v.toMap();
    Song song;
    song.set_title(file["title"].toString());
    QString url = file["downloadUrl"].toString() + "#" + access_token_;
    song.set_url(url);
    song.set_filesize(file["fileSize"].toInt());
    root_->appendRow(CreateSongItem(song));

    qLog(Debug) << "Creating stream";
    DriveStream stream(
        file["downloadUrl"].toUrl(),
        file["title"].toString(),
        file["fileSize"].toUInt(),
        access_token_,
        &network_);
    qLog(Debug) << "Creating tag";
    TagLib::MPEG::File tag(
        &stream,
        TagLib::ID3v2::FrameFactory::instance(),
        TagLib::AudioProperties::Fast);
    qLog(Debug) << "Tagging done";
    qLog(Debug) << tag.tag()->artist().toCString();
  }
}
