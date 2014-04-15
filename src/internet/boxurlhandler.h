#ifndef BOXURLHANDLER_H
#define BOXURLHANDLER_H

#include "core/urlhandler.h"

class BoxService;

class BoxUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  BoxUrlHandler(BoxService* service, QObject* parent = nullptr);

  QString scheme() const { return "box"; }
  QIcon icon() const { return QIcon(":/providers/box.png"); }
  LoadResult StartLoading(const QUrl& url);

 private:
  BoxService* service_;
};

#endif  // BOXURLHANDLER_H
