#include "seafileurlhandler.h"

#include "seafileservice.h"

SeafileUrlHandler::SeafileUrlHandler(SeafileService* service, QObject* parent)
    : UrlHandler(parent), service_(service) {}

UrlHandler::LoadResult SeafileUrlHandler::StartLoading(const QUrl& url) {
  QString file_library_and_path = url.path();
  QRegExp reg("/([^/]+)(/.*)$");

  if(reg.indexIn(file_library_and_path) == -1) {
      qLog(Debug) << "Can't find repo and file path in " << url;
  }

  QString library = reg.cap(1);
  QString filepath = reg.cap(2);

  QUrl real_url = service_->GetStreamingUrlFromSongId(library, filepath);

  return LoadResult(url, LoadResult::TrackAvailable, real_url);
}
