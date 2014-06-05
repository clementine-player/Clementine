#ifndef SEAFILEURLHANDLER_H
#define SEAFILEURLHANDLER_H

#include "core/urlhandler.h"

class SeafileService;

class SeafileUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  SeafileUrlHandler(SeafileService* service, QObject* parent = nullptr);

  QString scheme() const { return "seafile"; }
  QIcon icon() const { return QIcon(":/providers/seafile.png"); }
  LoadResult StartLoading(const QUrl& url);

 private:
  SeafileService* service_;
};

#endif  // SEAFILEURLHANDLER_H
