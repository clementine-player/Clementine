#ifndef INTERNET_AMAZON_AMAZONURLHANDLER_H_
#define INTERNET_AMAZON_AMAZONURLHANDLER_H_

#include "core/urlhandler.h"

class AmazonCloudDrive;

class AmazonUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  explicit AmazonUrlHandler(
      AmazonCloudDrive* service, QObject* parent = nullptr);

  QString scheme() const { return "amazonclouddrive"; }
  QIcon icon() const { return QIcon(":providers/amazonclouddrive.png"); }
  LoadResult StartLoading(const QUrl& url);

 private:
  AmazonCloudDrive* service_;
};

#endif  // INTERNET_AMAZON_AMAZONURLHANDLER_H_
