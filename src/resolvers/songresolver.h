#ifndef SONGRESOLVER_H
#define SONGRESOLVER_H

#include <QEventLoop>
#include <QList>
#include <QObject>

#include "core/song.h"

class LibraryBackendInterface;
class Resolver;

class SongResolver : public QObject {
  Q_OBJECT
 public:
  SongResolver(LibraryBackendInterface* library, QObject* parent = 0);
  virtual ~SongResolver();

  // Blocking
  bool ResolveSong(Song* song);

 private slots:
  void ResolveFinished(int, SongList resolved_songs);

 private:
  void RegisterResolver(Resolver* resolver);

  QList<Resolver*> resolvers_;
  Song* song_;

  QEventLoop loop_;

  int resolvers_finished_;
  bool resolved_;
};

#endif
