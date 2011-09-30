#include "groovesharksearchprovider.h"

#include "core/logging.h"
#include "internet/groovesharkservice.h"

GroovesharkSearchProvider::GroovesharkSearchProvider(QObject* parent)
    : service_(NULL) {
}

void GroovesharkSearchProvider::Init(GrooveSharkService* service) {
  service_ = service;
  SearchProvider::Init("GrooveShark", "grooveshark", QIcon(), true, true);
  connect(service_, SIGNAL(SimpleSearchResults(int, SongList)),
          SLOT(SearchDone(int, SongList)));
}

void GroovesharkSearchProvider::SearchAsync(int id, const QString& query) {
  const int service_id = service_->SimpleSearch(query);
  pending_searches_[service_id] = id;

  qLog(Debug) << "Searching grooveshark for:" << query;
}

void GroovesharkSearchProvider::SearchDone(int id, SongList songs) {
  qLog(Debug) << Q_FUNC_INFO;

  // Map back to the original id.
  const int global_search_id = pending_searches_.take(id);

  ResultList ret;
  foreach (const Song& song, songs) {
    Result result(this);
    result.type_ = Result::Type_Track;
    result.metadata_ = song;
    result.match_quality_ = Result::Quality_AtStart;

    ret << result;
  }

  qLog(Debug) << "Found:" << ret.size() << "songs from grooveshark";

  emit ResultsAvailable(global_search_id, ret);
  emit SearchFinished(global_search_id);
}

void GroovesharkSearchProvider::LoadArtAsync(int id, const Result& result) {

}

void GroovesharkSearchProvider::LoadTracksAsync(int id, const Result& result) {

}
