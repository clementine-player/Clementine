#include "boxurlhandler.h"

#include "boxservice.h"

BoxUrlHandler::BoxUrlHandler(BoxService* service, QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult BoxUrlHandler::StartLoading(const QUrl& url) {
  QString file_id = url.path();
  QUrl real_url = service_->GetStreamingUrlFromSongId(file_id);
  return LoadResult(url, LoadResult::TrackAvailable, real_url);
}
