#include "subsonicservice.h"
#include "subsonicurlhandler.h"

SubsonicUrlHandler::SubsonicUrlHandler(SubsonicService* service, QObject* parent)
  : UrlHandler(parent),
    service_(service) {
}

UrlHandler::LoadResult SubsonicUrlHandler::StartLoading(const QUrl& url) {
  if (service_->login_state() != SubsonicService::LoginState_Loggedin)
    return LoadResult();

  QUrl newurl(url);
  newurl.setScheme(realscheme());
  return LoadResult(url, LoadResult::TrackAvailable, newurl);
}
