#include "webremoteclient.h"

#include <QByteArray>

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"

namespace {

const char* kEndpoint = "http://localhost:8080/channel/push";

}  // namespace

WebRemoteClient::WebRemoteClient(
    ClementineWebPage* web_channel, Application* app, QObject* parent)
    : RemoteClient(app, parent),
      web_channel_(web_channel),
      network_(new NetworkAccessManager) {
}

WebRemoteClient::~WebRemoteClient() {}

void WebRemoteClient::SendData(pb::remote::Message* msg) {
  qLog(Debug) << "Sending:" << msg->DebugString().c_str();
  std::string data = msg->SerializeAsString();
  QByteArray base64 = QByteArray(data.data(), data.size()).toBase64();
  QNetworkRequest request = QNetworkRequest(QUrl(kEndpoint));
  QNetworkReply* reply = network_->post(request, base64);
  NewClosure(reply, SIGNAL(finished()), [&]() {
    reply->deleteLater();
    qLog(Debug) << reply->error()
                << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                << reply->readAll();
  });
}

void WebRemoteClient::DisconnectClient(pb::remote::ReasonDisconnect reason) {
  qLog(Debug) << Q_FUNC_INFO << reason;
}

QAbstractSocket::SocketState WebRemoteClient::state() {
  return QAbstractSocket::ConnectedState;
}
