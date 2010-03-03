#ifndef MOCK_NETWORKACCESSAMANGER_H
#define MOCK_NETWORKACCESSMANAGER_H

#include <QList>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#include "test_utils.h"
#include "gmock/gmock.h"

class MockNetworkReply : public QNetworkReply {
  Q_OBJECT
 public:
  MockNetworkReply();
  MockNetworkReply(const char* data);
  virtual ~MockNetworkReply() {}

  void SetData(const char* data);
  virtual void setAttribute(QNetworkRequest::Attribute code, const QVariant& value);

  void Done();

 protected:
  MOCK_METHOD0(abort, void());
  virtual qint64 readData(char* data, qint64);
  virtual qint64 writeData(const char* data, qint64);

  const char* data_;
  qint64 size_;
  qint64 pos_;
};


class MockNetworkAccessManager : public QNetworkAccessManager {
  Q_OBJECT
 public:
  virtual ~MockNetworkAccessManager();
  MockNetworkReply* ExpectGet(
      const QString& contains,
      const QMap<QString, QString>& params,
      int status,
      const char* ret_data);
 protected:
  MOCK_METHOD3(createRequest, QNetworkReply*(Operation, const QNetworkRequest&, QIODevice*));
};

#endif
