#include "googledriveservice.h"

#include <qjson/parser.h>

#include "core/closure.h"
#include "internetmodel.h"
#include "oauthenticator.h"

namespace {

const char* kGoogleDriveFiles = "https://www.googleapis.com/drive/v2/files";

}

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
  }
}
