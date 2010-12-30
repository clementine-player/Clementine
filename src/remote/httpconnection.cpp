#include "httpconnection.h"

#include <QStringList>
#include <QTcpSocket>

HttpConnection::HttpConnection(QTcpSocket* socket)
    : QObject(socket),
      socket_(socket),
      state_(WaitingForRequest) {
  connect(socket_, SIGNAL(readyRead()), SLOT(ReadyRead()));
}

void HttpConnection::ReadyRead() {
  switch (state_) {
    case WaitingForRequest:
      if (socket_->canReadLine()) {
        QString line = socket_->readLine();
        if (ParseRequest(line)) {
          state_ = WaitingForHeaders;
        } else {
          state_ = Error;
        }
      }
      break;
    case WaitingForHeaders:
      while (socket_->canReadLine()) {
        QByteArray line = socket_->readLine();
        if (line == "\r\n") {
          DoRequest();
          state_ = Finished;
        } else {
          // TODO: Parse headers.
        }
      }
      break;

    default:
      break;
  }

  if (state_ == Error) {
    socket_->close();
    return;
  }

  if (socket_->canReadLine()) {
    ReadyRead();
  }
}

bool HttpConnection::ParseRequest(const QString& line) {
  QStringList tokens = line.split(' ', QString::SkipEmptyParts);
  if (tokens.length() != 3) {
    return false;
  }
  const QString& method = tokens[0];
  if (method != "GET") {
    return false;
  }

  method_ = QNetworkAccessManager::GetOperation;
  path_ = tokens[1];
  return true;
}

void HttpConnection::DoRequest() {
  socket_->write("HTTP/1.0 200 OK\r\n");
  socket_->write("Content-type: application/json\r\n");
  socket_->write("\r\n");
  socket_->write("{content:'Hello World!'}");
  socket_->close();
}
