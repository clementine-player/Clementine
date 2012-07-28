#ifndef KITTENLOADER_H
#define KITTENLOADER_H

#include "albumcoverloader.h"

#include <QQueue>
#include <QUrl>

class KittenLoader : public AlbumCoverLoader {
  Q_OBJECT
 public:
  KittenLoader(QObject* parent = 0);

  virtual quint64 LoadKitten(const AlbumCoverLoaderOptions& options);

 private slots:
  void KittensRetrieved();
  void FetchMoreKittens();

 private:
  static const char* kFlickrKittenUrl;
  static const char* kFlickrPhotoUrl;

  QQueue<Task> pending_kittens_;
  QQueue<QUrl> kitten_urls_;
};

#endif
