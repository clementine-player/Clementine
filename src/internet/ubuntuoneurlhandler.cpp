#include "ubuntuoneurlhandler.h"

#include "ubuntuoneservice.h"

UbuntuOneUrlHandler::UbuntuOneUrlHandler(UbuntuOneService* service,
                                         QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult UbuntuOneUrlHandler::StartLoading(const QUrl& url) {
  QString file_id = url.path();
  QUrl real_url = service_->GetStreamingUrlFromSongId(file_id);
  return LoadResult(url, LoadResult::TrackAvailable, real_url);
}
