#ifndef DROPBOXURLHANDLER_H
#define DROPBOXURLHANDLER_H

#include "core/urlhandler.h"

class DropboxService;

class DropboxUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  DropboxUrlHandler(DropboxService* service, QObject* parent = nullptr);

  QString scheme() const { return "dropbox"; }
  QIcon icon() const { return QIcon(":providers/dropbox.png"); }
  LoadResult StartLoading(const QUrl& url);

 private:
  DropboxService* service_;
};

#endif
