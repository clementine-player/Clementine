#include "dropboxauthenticator.h"

#include <time.h>

#include <QStringList>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"

namespace {
static const char* kAppKey = "qh6ca27eclt9p2k";
static const char* kAppSecret = "pg7y68h5efap8r6";

static const char* kRequestTokenEndpoint =
    "https://api.dropbox.com/1/oauth/request_token";
}  // namespace

DropboxAuthenticator::DropboxAuthenticator(QObject* parent)
    : QObject(parent),
      network_(new NetworkAccessManager(this)) {
}

void DropboxAuthenticator::StartAuthorisation(const QString& email) {
  QUrl url(kRequestTokenEndpoint);
  typedef QPair<QString, QString> Param;

  QByteArray signature = QUrl::toPercentEncoding(QString(kAppSecret) + "&");
  QList<Param> params;
  params << Param("oauth_consumer_key", kAppKey)
         << Param("oauth_signature_method", "PLAINTEXT")
         << Param("oauth_timestamp", QString::number(time(NULL)))
         << Param("oauth_nonce", QString::number(qrand()))
         << Param("oauth_signature", signature);
  QStringList encoded_params;
  foreach (const Param& p, params) {
    encoded_params << QString("%1=\"%2\"").arg(p.first, p.second);
  }
  QString authorisation_header = QString("OAuth ") + encoded_params.join(", ");
  qLog(Debug) << authorisation_header;

  QNetworkRequest request(url);
  request.setRawHeader("Authorization", authorisation_header.toUtf8());

  QNetworkReply* reply = network_->post(request, QByteArray());
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(RequestTokenFinished(QNetworkReply*)), reply);
}

void DropboxAuthenticator::RequestTokenFinished(QNetworkReply* reply) {
  qLog(Debug) << reply->readAll();
}
