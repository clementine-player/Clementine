#ifndef GROOVESHARKSEARCHPROVIDER_H
#define GROOVESHARKSEARCHPROVIDER_H

#include "searchprovider.h"
#include "core/backgroundthread.h"

class AlbumCoverLoader;
class GrooveSharkService;

class GroovesharkSearchProvider : public SearchProvider {
  Q_OBJECT

 public:
  explicit GroovesharkSearchProvider(QObject* parent = 0);
  void Init(GrooveSharkService* service);

  // SearchProvider
  void SearchAsync(int id, const QString& query);
  void LoadArtAsync(int id, const Result& result);
  void LoadTracksAsync(int id, const Result& result);

 private slots:
  void SearchDone(int id, SongList songs);
  void AlbumArtLoaded(quint64 id, const QImage& image);

 private:
  GrooveSharkService* service_;
  QMap<int, int> pending_searches_;

  BackgroundThread<AlbumCoverLoader>* cover_loader_;
  QMap<quint64, int> cover_loader_tasks_;
};

#endif
