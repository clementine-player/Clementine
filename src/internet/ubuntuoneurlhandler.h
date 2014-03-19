#ifndef UBUNTUONEURLHANDLER_H
#define UBUNTUONEURLHANDLER_H

#include "core/urlhandler.h"

class UbuntuOneService;

class UbuntuOneUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  UbuntuOneUrlHandler(UbuntuOneService* service, QObject* parent = nullptr);

  QString scheme() const { return "ubuntuonefile"; }
  QIcon icon() const { return QIcon(":providers/ubuntuone.png"); }
  LoadResult StartLoading(const QUrl& url);

 private:
  UbuntuOneService* service_;
};

#endif  // UBUNTUONEURLHANDLER_H
