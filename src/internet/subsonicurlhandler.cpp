#include "subsonicservice.h"
#include "subsonicurlhandler.h"

SubsonicUrlHandler::SubsonicUrlHandler(SubsonicService* service,
                                       QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult SubsonicUrlHandler::StartLoading(const QUrl& url) {
  if (service_->login_state() != SubsonicService::LoginState_Loggedin)
    return LoadResult();

  QUrl newurl = service_->BuildRequestUrl("stream");
  newurl.addQueryItem("id", url.host());
  return LoadResult(url, LoadResult::TrackAvailable, newurl);
}
