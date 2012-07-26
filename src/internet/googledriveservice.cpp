#include "googledriveservice.h"

#include <QEventLoop>

#include <qjson/parser.h>

#include <google/sparsetable>

#include <taglib/id3v2framefactory.h>
#include <taglib/mpegfile.h>
#include <taglib/tiostream.h>
using TagLib::ByteVector;

#include "core/application.h"
#include "core/closure.h"
#include "core/player.h"
#include "googledriveurlhandler.h"
#include "internetmodel.h"
#include "oauthenticator.h"

namespace {

static const char* kGoogleDriveFiles = "https://www.googleapis.com/drive/v2/files";
static const char* kGoogleDriveFile = "https://www.googleapis.com/drive/v2/files/%1";
static const char* kSettingsGroup = "GoogleDrive";

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
        network_(network),
        cache_(length) {
    qLog(Debug) << Q_FUNC_INFO
                << url_
                << filename_
                << length_;
  }

  virtual TagLib::FileName name() const {
    qLog(Debug) << Q_FUNC_INFO;
    return encoded_filename_.data();
  }

  bool CheckCache(int start, int end) {
    for (int i = start; i <= end; ++i) {
      if (!cache_.test(i)) {
        return false;
      }
    }
    return true;
  }

  void FillCache(int start, TagLib::ByteVector data) {
    for (int i = 0; i < data.size(); ++i) {
      cache_.set(start + i, data[i]);
    }
  }

  TagLib::ByteVector GetCached(int start, int end) {
    const uint size = end - start + 1;
    TagLib::ByteVector ret(size);
    for (int i = 0; i < size; ++i) {
      ret[i] = cache_.get(start + i);
    }
    return ret;
  }

  virtual TagLib::ByteVector readBlock(ulong length) {
    qLog(Debug) << Q_FUNC_INFO;
    const uint start = cursor_;
    const uint end = qMin(cursor_ + length - 1, length_ - 1);

    if (end <= start) {
      return TagLib::ByteVector();
    }

    if (CheckCache(start, end)) {
      qLog(Debug) << "Cache hit at:" << start << end;
      TagLib::ByteVector cached = GetCached(start, end);
      cursor_ += cached.size();
      return cached;
    }

    QNetworkRequest request = QNetworkRequest(url_);
    request.setRawHeader(
        "Authorization", QString("Bearer %1").arg(auth_).toUtf8());
    request.setRawHeader(
        "Range", QString("bytes=%1-%2").arg(start).arg(end).toUtf8());

    QNetworkReply* reply = network_->get(request);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    reply->deleteLater();

    QByteArray data = reply->readAll();
    TagLib::ByteVector bytes(data.data(), data.size());
    cursor_ += data.size();

    FillCache(start, bytes);
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
        cursor_ = qMin(ulong(cursor_ + offset), length_);
        break;

      case TagLib::IOStream::End:
        cursor_ = qMax(0UL, length_ - offset);
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
  const ulong length_;
  const QString auth_;

  int cursor_;
  QNetworkAccessManager* network_;

  google::sparsetable<char> cache_;
};


GoogleDriveService::GoogleDriveService(Application* app, InternetModel* parent)
    : InternetService("Google Drive", app, parent, parent),
      root_(NULL),
      oauth_(new OAuthenticator(this)) {
  connect(oauth_, SIGNAL(AccessTokenAvailable(QString)), SLOT(AccessTokenAvailable(QString)));
  connect(oauth_, SIGNAL(RefreshTokenAvailable(QString)), SLOT(RefreshTokenAvailable(QString)));

  app->player()->RegisterUrlHandler(new GoogleDriveUrlHandler(this, this));
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
  QSettings s;
  s.beginGroup(kSettingsGroup);

  if (s.contains("refresh_token")) {
    QString refresh_token = s.value("refresh_token").toString();
    RefreshAuthorisation(refresh_token);
  } else {
    oauth_->StartAuthorisation();
  }
}

void GoogleDriveService::RefreshAuthorisation(const QString& refresh_token) {
  oauth_->RefreshAuthorisation(refresh_token);
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

void GoogleDriveService::RefreshTokenAvailable(const QString& token) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("refresh_token", token);
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
    qLog(Debug) << "Creating stream";
    DriveStream* stream = new DriveStream(
        file["downloadUrl"].toUrl(),
        file["title"].toString(),
        file["fileSize"].toUInt(),
        access_token_,
        &network_);
    qLog(Debug) << "Creating tag";
    TagLib::MPEG::File tag(
        stream,  // Takes ownership.
        TagLib::ID3v2::FrameFactory::instance(),
        TagLib::AudioProperties::Fast);
    qLog(Debug) << "Tagging done";
    if (tag.tag()) {
      qLog(Debug) << tag.tag()->artist().toCString();
      Song song;
      song.set_title(tag.tag()->title().toCString(true));
      song.set_artist(tag.tag()->artist().toCString(true));
      song.set_album(tag.tag()->album().toCString(true));

      QString url = QString("googledrive:%1").arg(file["id"].toString());
      song.set_url(url);
      qLog(Debug) << "Set url to:" << url;

      song.set_filesize(file["fileSize"].toInt());
      root_->appendRow(CreateSongItem(song));
    } else {
      qLog(Debug) << "Tagging failed";
    }
  }
}

QUrl GoogleDriveService::GetStreamingUrlFromSongId(const QString& id) {
  QString url = QString(kGoogleDriveFile).arg(id);
  QNetworkRequest request = QNetworkRequest(url);
  request.setRawHeader(
      "Authorization", QString("Bearer %1").arg(access_token_).toUtf8());
  QNetworkReply* reply = network_.get(request);
  QEventLoop loop;
  connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();

  QJson::Parser parser;
  bool ok = false;
  QVariantMap result = parser.parse(reply, &ok).toMap();
  QString download_url = result["downloadUrl"].toString() + "#" + access_token_;
  return QUrl(download_url);
}
