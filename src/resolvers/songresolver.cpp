#include "songresolver.h"

#include "core/logging.h"
#include "core/song.h"
#include "internet/internetmodel.h"
#include "internet/spotifyservice.h"
#include "libraryresolver.h"
#include "spotifyresolver.h"

SongResolver::SongResolver(LibraryBackendInterface* library, QObject* parent)
    : QObject(parent),
      song_(NULL),
      resolvers_finished_(0),
      resolved_(false) {
  // Register in the order they should be checked.
  RegisterResolver(new LibraryResolver(library));
  RegisterResolver(new SpotifyResolver(InternetModel::Service<SpotifyService>()->server()));
}

SongResolver::~SongResolver() {
  qDeleteAll(resolvers_);
  resolvers_.clear();
}

void SongResolver::RegisterResolver(Resolver* resolver) {
  resolvers_ << resolver;
  connect(resolver, SIGNAL(ResolveFinished(int, SongList)), SLOT(ResolveFinished(int, SongList)));
}

bool SongResolver::ResolveSong(Song* song) {
  song_ = song;
  foreach (Resolver* resolver, resolvers_) {
    resolver->ResolveSong(*song);
  }
  loop_.exec();
  return resolved_;
}

void SongResolver::ResolveFinished(int, SongList resolved_songs) {
  ++resolvers_finished_;
  if (resolvers_finished_ == resolvers_.size()) {
    loop_.quit();
  }

  if (!resolved_songs.isEmpty()) {
    *song_ = resolved_songs.first();
    qLog(Debug) << "Resolved song:" << song_->title() << "from:" << sender()->metaObject()->className();
    resolved_ = true;
    loop_.quit();
  }
}
