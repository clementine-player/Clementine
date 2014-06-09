#include "dropboxurlhandler.h"

#include "internet/dropboxservice.h"

DropboxUrlHandler::DropboxUrlHandler(DropboxService* service, QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult DropboxUrlHandler::StartLoading(const QUrl& url) {
  return LoadResult(url, LoadResult::TrackAvailable,
                    service_->GetStreamingUrlFromSongId(url));
}
