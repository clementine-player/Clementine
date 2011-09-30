#ifndef GROOVESHARKSEARCHPROVIDER_H
#define GROOVESHARKSEARCHPROVIDER_H

#include "searchprovider.h"

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

 private:
  GrooveSharkService* service_;
  QMap<int, int> pending_searches_;

};

#endif
