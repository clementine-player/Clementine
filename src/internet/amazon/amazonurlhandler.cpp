#include "internet/amazon/amazonurlhandler.h"

#include "internet/amazon/amazonclouddrive.h"

AmazonUrlHandler::AmazonUrlHandler(AmazonCloudDrive* service, QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult AmazonUrlHandler::StartLoading(const QUrl& url) {
  return LoadResult(url, LoadResult::TrackAvailable,
                    service_->GetStreamingUrlFromSongId(url));
}
