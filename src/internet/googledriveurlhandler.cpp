#include "googledriveurlhandler.h"

#include "googledriveservice.h"

GoogleDriveUrlHandler::GoogleDriveUrlHandler(GoogleDriveService* service,
                                             QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult GoogleDriveUrlHandler::StartLoading(const QUrl& url) {
  QString file_id = url.path();
  QUrl real_url = service_->GetStreamingUrlFromSongId(file_id);
  return LoadResult(url, LoadResult::TrackAvailable, real_url);
}
