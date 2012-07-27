#include "googledriveservice.h"

#include <QEventLoop>
#include <QSortFilterProxyModel>

#include <qjson/parser.h>

#include <google/sparsetable>

#include <taglib/id3v2framefactory.h>
#include <taglib/mpegfile.h>
#include <taglib/tiostream.h>
using TagLib::ByteVector;

#include "core/application.h"
#include "core/closure.h"
#include "core/database.h"
#include "core/mergedproxymodel.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "library/librarybackend.h"
#include "library/librarymodel.h"
#include "googledriveurlhandler.h"
#include "internetmodel.h"
#include "oauthenticator.h"

namespace {

static const char* kGoogleDriveFiles = "https://www.googleapis.com/drive/v2/files";
static const char* kGoogleDriveFile = "https://www.googleapis.com/drive/v2/files/%1";
static const char* kSettingsGroup = "GoogleDrive";

static const char* kSongsTable = "google_drive_songs";
static const char* kFtsTable = "google_drive_songs_fts";

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
  }

  virtual TagLib::FileName name() const {
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
    const uint start = cursor_;
    const uint end = qMin(cursor_ + length - 1, length_ - 1);

    if (end <= start) {
      return TagLib::ByteVector();
    }

    if (CheckCache(start, end)) {
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
    return true;
  }

  virtual void seek(long offset, TagLib::IOStream::Position p) {
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
    cursor_ = 0;
  }

  virtual long tell() const {
    return cursor_;
  }

  virtual long length() {
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
      oauth_(new OAuthenticator(this)),
      library_sort_model_(new QSortFilterProxyModel(this)) {
  connect(oauth_, SIGNAL(AccessTokenAvailable(QString)), SLOT(AccessTokenAvailable(QString)));
  connect(oauth_, SIGNAL(RefreshTokenAvailable(QString)), SLOT(RefreshTokenAvailable(QString)));

  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(app_->database()->thread());
  library_backend_->Init(app_->database(), kSongsTable,
                         QString::null, QString::null, kFtsTable);
  library_model_ = new LibraryModel(library_backend_, app_, this);

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->sort(0);

  app->player()->RegisterUrlHandler(new GoogleDriveUrlHandler(this, this));
  app_->global_search()->AddProvider(new LibrarySearchProvider(
      library_backend_,
      tr("Google Drive"),
      "google_drive",
      QIcon(":/providers/googledrive.png"),
      true, app_, this));
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
      library_model_->Init();
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
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
    MaybeAddFileToDatabase(file);
  }
}

void GoogleDriveService::MaybeAddFileToDatabase(const QVariantMap& file) {
  QString url = QString("googledrive:%1").arg(file["id"].toString());
  Song song = library_backend_->GetSongByUrl(QUrl(url));
  // Song already in index.
  // TODO: Check etag and maybe update.
  if (song.is_valid()) {
    return;
  }

  // Song not in index; tag and add.
  DriveStream* stream = new DriveStream(
      file["downloadUrl"].toUrl(),
      file["title"].toString(),
      file["fileSize"].toUInt(),
      access_token_,
      &network_);
  TagLib::MPEG::File tag(
      stream,  // Takes ownership.
      TagLib::ID3v2::FrameFactory::instance(),
      TagLib::AudioProperties::Fast);
  if (tag.tag()) {
    Song song;
    song.set_title(tag.tag()->title().toCString(true));
    song.set_artist(tag.tag()->artist().toCString(true));
    song.set_album(tag.tag()->album().toCString(true));

    song.set_url(url);
    song.set_filesize(file["fileSize"].toInt());
    song.set_etag(file["etag"].toString().remove('"'));

    QString modified_date = file["modifiedDate"].toString();
    QString created_date = file["createdDate"].toString();

    song.set_mtime(QDateTime::fromString(modified_date, Qt::ISODate).toTime_t());
    song.set_ctime(QDateTime::fromString(created_date, Qt::ISODate).toTime_t());

    song.set_filetype(Song::Type_Stream);
    song.set_directory_id(0);

    if (tag.audioProperties()) {
      song.set_length_nanosec(tag.audioProperties()->length() * kNsecPerSec);
    }

    SongList songs;
    songs << song;
    qLog(Debug) << "Adding song to db:" << song.title();
    library_backend_->AddOrUpdateSongs(songs);
  } else {
    qLog(Debug) << "Failed to tag:" << url;
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
