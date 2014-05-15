#include "webremoteclient.h"

#include <QByteArray>

#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "networkremote/clementinewebpage.h"

namespace {

const char* kEndpointPath = "/channel/clementine/push/%1";

}  // namespace

WebRemoteClient::WebRemoteClient(
    ClementineWebPage* web_channel, Application* app, QObject* parent)
    : RemoteClient(app, parent),
      web_channel_(web_channel),
      network_(new NetworkAccessManager) {
  connect(web_channel, SIGNAL(MessageReceived(const pb::remote::Message&)),
          SIGNAL(Parse(const pb::remote::Message&)));
}

WebRemoteClient::~WebRemoteClient() {}

void WebRemoteClient::SendData(pb::remote::Message* msg) {
  qLog(Debug) << "Sending:" << msg->DebugString().c_str();
  std::string data = msg->SerializeAsString();
  QByteArray base64 = QByteArray(data.data(), data.size()).toBase64();

  QUrl url(app_->remote_base_url());
  url.setPath(QString(kEndpointPath).arg(web_channel_->id()));

  QNetworkRequest request = QNetworkRequest(url);
  QNetworkReply* reply = network_->post(request, base64);
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(SendDataFinished(QNetworkReply*)), reply);
}

void WebRemoteClient::SendDataFinished(QNetworkReply* reply) {
  reply->deleteLater();
  qLog(Debug) << reply->error()
              << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
              << reply->readAll();
}

void WebRemoteClient::DisconnectClient(pb::remote::ReasonDisconnect reason) {
  qLog(Debug) << Q_FUNC_INFO << reason;
}

QAbstractSocket::SocketState WebRemoteClient::state() {
  return QAbstractSocket::ConnectedState;
}
