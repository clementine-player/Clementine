#include "skydriveurlhandler.h"

#include "skydriveservice.h"

SkydriveUrlHandler::SkydriveUrlHandler(SkydriveService* service,
                                       QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult SkydriveUrlHandler::StartLoading(const QUrl& url) {
  QString file_id(url.path());
  QUrl real_url = service_->GetStreamingUrlFromSongId(file_id);
  return LoadResult(url, LoadResult::TrackAvailable, real_url);
}
