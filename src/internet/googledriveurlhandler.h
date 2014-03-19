#ifndef GOOGLEDRIVEURLHANDLER_H
#define GOOGLEDRIVEURLHANDLER_H

#include "core/urlhandler.h"

class GoogleDriveService;

class GoogleDriveUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  GoogleDriveUrlHandler(GoogleDriveService* service, QObject* parent = nullptr);

  QString scheme() const { return "googledrive"; }
  QIcon icon() const { return QIcon(":providers/googledrive.png"); }
  LoadResult StartLoading(const QUrl& url);

 private:
  GoogleDriveService* service_;
};

#endif
