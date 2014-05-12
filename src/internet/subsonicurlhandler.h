#ifndef SUBSONICURLHANDLER_H
#define SUBSONICURLHANDLER_H

#include "core/urlhandler.h"

class SubsonicService;

// Subsonic URL handler: subsonic://id
class SubsonicUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  SubsonicUrlHandler(SubsonicService* service, QObject* parent);

  QString scheme() const { return "subsonic"; }
  QIcon icon() const { return QIcon(":providers/subsonic-32.png"); }
  LoadResult StartLoading(const QUrl& url);
  // LoadResult LoadNext(const QUrl& url);

 private:
  SubsonicService* service_;
};

#endif  // SUBSONICURLHANDLER_H
