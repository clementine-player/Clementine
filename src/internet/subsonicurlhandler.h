#ifndef SUBSONICURLHANDLER_H
#define SUBSONICURLHANDLER_H

#include "core/urlhandler.h"

class SubsonicService;

// Subsonic URL handler.
// For now, at least, Subsonic URLs are just HTTP URLs but with the scheme
// replaced with "subsonic" (or "subsonics" for HTTPS).  This is a hook to
// allow magic to be implemented later.
class SubsonicUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  SubsonicUrlHandler(SubsonicService* service, QObject* parent);

  QString scheme() const { return "subsonic"; }
  QIcon icon() const { return QIcon(":providers/subsonic-32.png"); }
  LoadResult StartLoading(const QUrl& url);
  //LoadResult LoadNext(const QUrl& url);

 protected:
  virtual QString realscheme() const { return "http"; }

 private:
  SubsonicService* service_;
};

class SubsonicHttpsUrlHandler : public SubsonicUrlHandler {
  Q_OBJECT
 public:
  SubsonicHttpsUrlHandler(SubsonicService* service, QObject* parent)
    : SubsonicUrlHandler(service, parent) {}

  QString scheme() const { return "subsonics"; }

 protected:
  QString realscheme() const { return "https"; }
};

#endif // SUBSONICURLHANDLER_H
