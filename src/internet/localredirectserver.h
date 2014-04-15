#ifndef LOCALREDIRECTSERVER_H
#define LOCALREDIRECTSERVER_H

#include <QByteArray>
#include <QObject>
#include <QUrl>

class QTcpServer;
class QTcpSocket;

class LocalRedirectServer : public QObject {
  Q_OBJECT
 public:
  LocalRedirectServer(QObject* parent = nullptr);

  // Causes the server to listen for _one_ request.
  void Listen();

  // Returns the HTTP URL of this server.
  const QUrl& url() const { return url_; }

  // Returns the URL requested by the OAuth redirect.
  const QUrl& request_url() const { return request_url_; }

signals:
  void Finished();

 private slots:
  void NewConnection();
  void ReadyRead(QTcpSocket* socket, QByteArray buffer);

 private:
  void WriteTemplate(QTcpSocket* socket) const;
  QUrl ParseUrlFromRequest(const QByteArray& request) const;

 private:
  QTcpServer* server_;
  QUrl url_;
  QUrl request_url_;
};

#endif  // LOCALREDIRECTSERVER_H
