#ifndef KITTENLOADER_H
#define KITTENLOADER_H

#include "albumcoverloader.h"

#include <QQueue>
#include <QUrl>

class QNetworkReply;

class KittenLoader : public AlbumCoverLoader {
  Q_OBJECT
 public:
  KittenLoader(QObject* parent = nullptr);

  virtual quint64 LoadKitten(const AlbumCoverLoaderOptions& options);

 private slots:
  void KittensRetrieved(QNetworkReply* reply);
  void FetchMoreKittens();

 private:
  static const char* kFlickrKittenUrl;
  static const char* kFlickrPhotoUrl;

  QQueue<Task> pending_kittens_;
  QQueue<QUrl> kitten_urls_;
};

#endif
