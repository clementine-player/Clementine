#include "ubuntuoneservice.h"

#include <QDateTime>

#include <qjson/parser.h>

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "core/utilities.h"
#include "internet/internetmodel.h"
#include "internet/ubuntuoneauthenticator.h"
#include "internet/ubuntuoneurlhandler.h"

const char* UbuntuOneService::kServiceName = "Ubuntu One";
const char* UbuntuOneService::kSettingsGroup = "Ubuntu One";

namespace {
static const char* kFileStorageEndpoint =
    "https://one.ubuntu.com/api/file_storage/v1/~/Ubuntu One/";
static const char* kOAuthSSOFinishedEndpoint =
    "https://one.ubuntu.com/oauth/sso-finished-so-get-tokens/";
static const char* kContentRoot = "https://files.one.ubuntu.com";
static const char* kOAuthHeaderPrefix = "OAuth realm=\"\", ";
}

UbuntuOneService::UbuntuOneService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      root_(nullptr),
      network_(new NetworkAccessManager(this)) {
  app->player()->RegisterUrlHandler(new UbuntuOneUrlHandler(this, this));
}

QStandardItem* UbuntuOneService::CreateRootItem() {
  root_ = new QStandardItem(QIcon(), "Ubuntu One");
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void UbuntuOneService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      Connect();
      break;

    default:
      break;
  }
}

void UbuntuOneService::Connect() {
  UbuntuOneAuthenticator* authenticator = new UbuntuOneAuthenticator;
  authenticator->StartAuthorisation(
      "Username",
      "Password");
  NewClosure(authenticator, SIGNAL(Finished()),
             this, SLOT(AuthenticationFinished(UbuntuOneAuthenticator*)),
             authenticator);
}

QByteArray UbuntuOneService::GenerateAuthorisationHeader() {
  typedef QPair<QString, QString> Param;
  QString timestamp = QString::number(
      QDateTime::currentMSecsSinceEpoch() / kMsecPerSec);
  QList<Param> parameters;
  parameters << Param("oauth_nonce", QString::number(qrand()))
             << Param("oauth_timestamp", timestamp)
             << Param("oauth_version", "1.0")
             << Param("oauth_consumer_key", consumer_key_)
             << Param("oauth_token", token_)
             << Param("oauth_signature_method", "PLAINTEXT");
  qSort(parameters.begin(), parameters.end());
  QStringList encoded_params;
  for (const Param& p : parameters) {
    encoded_params << QString("%1=%2").arg(p.first, p.second);
  }

  QString signing_key =
      consumer_secret_ + "&" + token_secret_;
  QByteArray signature = QUrl::toPercentEncoding(signing_key);

  // Construct authorisation header
  parameters << Param("oauth_signature", signature);
  QStringList header_params;
  for (const Param& p : parameters) {
    header_params << QString("%1=\"%2\"").arg(p.first, p.second);
  }
  QString authorisation_header = header_params.join(", ");
  authorisation_header.prepend(kOAuthHeaderPrefix);

  return authorisation_header.toAscii();
}

void UbuntuOneService::AuthenticationFinished(
    UbuntuOneAuthenticator* authenticator) {
  authenticator->deleteLater();

  consumer_key_ = authenticator->consumer_key();
  consumer_secret_ = authenticator->consumer_secret();
  token_ = authenticator->token();
  token_secret_ = authenticator->token_secret();

  QUrl sso_url(kOAuthSSOFinishedEndpoint);
  QNetworkRequest request(sso_url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  request.setRawHeader("Accept", "application/json");

  qLog(Debug) << "Sending SSO copy request";
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(SSORequestFinished(QNetworkReply*)), reply);
}

void UbuntuOneService::SSORequestFinished(QNetworkReply* reply) {
  qLog(Debug) << Q_FUNC_INFO;
  QUrl files_url(kFileStorageEndpoint);
  files_url.addQueryItem("include_children", "true");
  QNetworkRequest request(files_url);
  request.setRawHeader("Authorization", GenerateAuthorisationHeader());
  request.setRawHeader("Accept", "application/json");

  qLog(Debug) << "Sending files request";
  QNetworkReply* files_reply = network_->get(request);
  NewClosure(files_reply, SIGNAL(finished()),
             this, SLOT(FileListRequestFinished(QNetworkReply*)), files_reply);
}

void UbuntuOneService::FileListRequestFinished(QNetworkReply* reply) {
  QByteArray data = reply->readAll();
  qLog(Debug) << reply->url();
  qLog(Debug) << data;
  qLog(Debug) << reply->rawHeaderList();
  qLog(Debug) << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  qLog(Debug) << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);

  QJson::Parser parser;
  QVariantMap result = parser.parse(data).toMap();

  QVariantList children = result["children"].toList();
  for (const QVariant& c : children) {
    QVariantMap child = c.toMap();
    QString content_path = child["content_path"].toString();

    QUrl content_url;
    content_url.setScheme("ubuntuonefile");
    content_url.setPath(content_path);

    Song song;
    song.set_title("One More Chance");
    song.set_artist("Bloc Party");
    song.set_url(content_url);

    root_->appendRow(CreateSongItem(song));
  }
}

QUrl UbuntuOneService::GetStreamingUrlFromSongId(const QString& song_id) {
  QUrl url(kContentRoot);
  url.setPath(song_id);
  url.setFragment(GenerateAuthorisationHeader());
  return url;
}
