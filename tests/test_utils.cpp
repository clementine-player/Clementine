#include "test_utils.h"

#include <QNetworkRequest>
#include <QString>
#include <QUrl>

std::ostream& operator<<(std::ostream& stream, const QString& str) {
  stream << str.toStdString();
  return stream;
}

std::ostream& operator <<(std::ostream& stream, const QUrl& url) {
  stream << url.toString().toStdString();
  return stream;
}

std::ostream& operator <<(std::ostream& stream, const QNetworkRequest& req) {
  stream << req.url().toString().toStdString();
  return stream;
}
