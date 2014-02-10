#ifndef SKYDRIVEURLHANDLER_H
#define SKYDRIVEURLHANDLER_H

#include "core/urlhandler.h"

class SkydriveService;

class SkydriveUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  SkydriveUrlHandler(SkydriveService* service, QObject* parent = nullptr);

  QString scheme() const { return "skydrive"; }
  QIcon icon() const { return QIcon(":providers/skydrive.png"); }
  LoadResult StartLoading(const QUrl& url);

 private:
  SkydriveService* service_;
};

#endif  // SKYDRIVEURLHANDLER_H
