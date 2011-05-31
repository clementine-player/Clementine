#ifndef RESOLVER_H
#define RESOLVER_H

#include <QObject>

#include "core/song.h"

class Resolver : public QObject {
  Q_OBJECT
 public:
  Resolver(QObject* parent = 0) : QObject(parent) {}
  virtual ~Resolver() {}

  virtual int ResolveSong(const Song& song) = 0;

 signals:
  void ResolveFinished(int id, SongList songs);
};

#endif  // RESOLVER_H
