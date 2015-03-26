#include "internet/amazon/amazonclouddrive.h"

#include <QIcon>

#include <qjson/parser.h>

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "internet/core/oauthenticator.h"
#include "internet/amazon/amazonurlhandler.h"
#include "ui/settingsdialog.h"

const char* AmazonCloudDrive::kServiceName = "Cloud Drive";
const char* AmazonCloudDrive::kSettingsGroup = "AmazonCloudDrive";

namespace {
static const char* kServiceId = "amazon_cloud_drive";
static const char* kClientId =
    "amzn1.application-oa2-client.2b1157a7dadc45c3888567882b3a9f05";
static const char* kClientSecret =
    "acfbf95340cc4c381dd43fb75b5e111882d7fd1b02a02f3013ab124baf8d1655";
static const char* kOAuthScope = "clouddrive:read";
static const char* kOAuthEndpoint = "https://www.amazon.com/ap/oa";
static const char* kOAuthTokenEndpoint = "https://api.amazon.com/auth/o2/token";

static const char* kEndpointEndpoint =
    "https://drive.amazonaws.com/drive/v1/account/endpoint";
static const char* kChangesEndpoint = "%1/changes";
static const char* kDownloadEndpoint = "%1/nodes/%2/content";
}  // namespace

AmazonCloudDrive::AmazonCloudDrive(Application* app, InternetModel* parent)
    : CloudFileService(app, parent, kServiceName, kServiceId,
                       QIcon(":/providers/amazonclouddrive.png"),
                       SettingsDialog::Page_AmazonCloudDrive),
      network_(new NetworkAccessManager) {
  app->player()->RegisterUrlHandler(new AmazonUrlHandler(this, this));
}

bool AmazonCloudDrive::has_credentials() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  return !s.value("refresh_token").toString().isEmpty();
}

QUrl AmazonCloudDrive::GetStreamingUrlFromSongId(const QUrl& url) {
  QUrl download_url(
      QString(kDownloadEndpoint).arg(content_url_).arg(url.path()));
  download_url.setFragment(QString("Bearer %1").arg(access_token_));
  return download_url;
}

void AmazonCloudDrive::Connect() {
  OAuthenticator* oauth = new OAuthenticator(
      kClientId, kClientSecret,
      // Amazon forbids arbitrary query parameters so REMOTE_WITH_STATE is
      // required.
      OAuthenticator::RedirectStyle::REMOTE_WITH_STATE, this);
  oauth->StartAuthorisation(kOAuthEndpoint, kOAuthTokenEndpoint, kOAuthScope);
  NewClosure(oauth, SIGNAL(Finished()), this,
            SLOT(ConnectFinished(OAuthenticator*)), oauth);
}

void AmazonCloudDrive::ForgetCredentials() {

}

void AmazonCloudDrive::ConnectFinished(OAuthenticator* oauth) {
  oauth->deleteLater();

  qLog(Debug) << oauth->access_token()
              << oauth->expiry_time()
              << oauth->refresh_token();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("refresh_token", oauth->refresh_token());

  access_token_ = oauth->access_token();
  // TODO: Amazon expiry time is only an hour so refresh this regularly.
  expiry_time_ = oauth->expiry_time();

  FetchEndpoint();
}

void AmazonCloudDrive::FetchEndpoint() {
  QUrl url(kEndpointEndpoint);
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(FetchEndpointFinished(QNetworkReply*)), reply);
}

void AmazonCloudDrive::FetchEndpointFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();
  content_url_ = response["contentUrl"].toString();
  metadata_url_ = response["metadataUrl"].toString();
  qLog(Debug) << "content_url:" << content_url_;
  qLog(Debug) << "metadata_url:" << metadata_url_;
  RequestChanges();
}

void AmazonCloudDrive::RequestChanges() {
  QUrl url(QString(kChangesEndpoint).arg(metadata_url_));
  QNetworkRequest request(url);
  AddAuthorizationHeader(&request);
  QNetworkReply* reply = network_->post(request, QByteArray());
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RequestChangesFinished(QNetworkReply*)), reply);
}

void AmazonCloudDrive::RequestChangesFinished(QNetworkReply* reply) {
  reply->deleteLater();
  QJson::Parser parser;
  QVariantMap response = parser.parse(reply).toMap();

  QString checkpoint = response["checkpoint"].toString();
  QSettings settings;
  settings.beginGroup(kSettingsGroup);
  settings.setValue("checkpoint", checkpoint);

  QVariantList nodes = response["nodes"].toList();
  qLog(Debug) << "nodes:" << nodes.length();
  for (const QVariant& n : nodes) {
    QVariantMap node = n.toMap();

    qLog(Debug) << node["kind"] << node["status"];

    if (node["kind"].toString() == "FOLDER") {
      continue;
    }
    QString status = node["status"].toString();
    if (node["status"].toString() != "AVAILABLE") {
      continue;
    }

    QVariantMap content_properties = node["contentProperties"].toMap();
    QString mime_type = content_properties["contentType"].toString();

    QUrl url;
    url.setScheme("amazonclouddrive");
    url.setPath(node["id"].toString());

    qLog(Debug) << url << mime_type;

    if (ShouldIndexFile(url, mime_type)) {
      QString node_id = node["id"].toString();
      QUrl content_url(
          QString(kDownloadEndpoint).arg(content_url_).arg(node_id));
      QString md5 = content_properties["md5"].toString();

      Song song;
      song.set_url(url);
      song.set_etag(md5);
      song.set_mtime(node["modifiedDate"].toDateTime().toTime_t());
      song.set_ctime(node["createdDate"].toDateTime().toTime_t());
      song.set_title(node["name"].toString());
      song.set_filesize(content_properties["size"].toInt());

      qLog(Debug) << "Adding:"
                  << song.title()
                  << mime_type
                  << url
                  << content_url;
      MaybeAddFileToDatabase(song, mime_type, content_url, QString("Bearer %1").arg(access_token_));
    }
  }
}

void AmazonCloudDrive::AddAuthorizationHeader(QNetworkRequest* request) {
  request->setRawHeader("Authorization",
                        QString("Bearer %1").arg(access_token_).toUtf8());
}
