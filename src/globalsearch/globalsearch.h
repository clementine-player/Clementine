/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GLOBALSEARCH_H
#define GLOBALSEARCH_H

#include <QObject>
#include <QPixmapCache>

#include "searchprovider.h"
#include "covers/albumcoverloaderoptions.h"

class AlbumCoverLoader;
class Application;
class UrlSearchProvider;

class GlobalSearch : public QObject {
  Q_OBJECT

 public:
  GlobalSearch(Application* app, QObject* parent = nullptr);

  static const int kDelayedSearchTimeoutMs;
  static const char* kSettingsGroup;
  static const int kMaxResultsPerEmission;

  Application* application() const { return app_; }

  void AddProvider(SearchProvider* provider);
  // Try to change provider state. Returns false if we can't (e.g. we can't
  // enable a provider because it requires the user to be logged-in)
  bool SetProviderEnabled(const SearchProvider* provider, bool enabled);

  int SearchAsync(const QString& query);
  int LoadArtAsync(const SearchProvider::Result& result);
  MimeData* LoadTracks(const SearchProvider::ResultList& results);
  QStringList GetSuggestions(int count);

  void CancelSearch(int id);
  void CancelArt(int id);

  bool FindCachedPixmap(const SearchProvider::Result& result,
                        QPixmap* pixmap) const;

  // "enabled" is the user preference.  "usable" is enabled AND logged in.
  QList<SearchProvider*> providers() const;
  bool is_provider_enabled(const SearchProvider* provider) const;
  bool is_provider_usable(SearchProvider* provider) const;

 public slots:

  void ReloadSettings();

signals:
  void SearchAsyncSig(int id, const QString& query);
  void ResultsAvailable(int id, const SearchProvider::ResultList& results);
  void ProviderSearchFinished(int id, const SearchProvider* provider);
  void SearchFinished(int id);

  void ArtLoaded(int id, const QPixmap& pixmap);

  void ProviderAdded(const SearchProvider* provider);
  void ProviderRemoved(const SearchProvider* provider);
  void ProviderToggled(const SearchProvider* provider, bool enabled);

 protected:
  void timerEvent(QTimerEvent* e);

 private slots:
  void DoSearchAsync(int id, const QString& query);
  void ResultsAvailableSlot(int id, SearchProvider::ResultList results);
  void SearchFinishedSlot(int id);

  void ArtLoadedSlot(int id, const QImage& image);
  void AlbumArtLoaded(quint64 id, const QImage& image);

  void ProviderDestroyedSlot(QObject* object);

 private:
  void ConnectProvider(SearchProvider* provider);
  void HandleLoadedArt(int id, const QImage& image, SearchProvider* provider);
  void TakeNextQueuedArt(SearchProvider* provider);
  QString PixmapCacheKey(const SearchProvider::Result& result) const;

  void SaveProvidersSettings();

 private:
  struct DelayedSearch {
    int id_;
    QString query_;
    QList<SearchProvider*> providers_;
  };

  struct QueuedArt {
    int id_;
    SearchProvider::Result result_;
  };

  struct ProviderData {
    QList<QueuedArt> queued_art_;
    bool enabled_;
  };

  Application* app_;

  QMap<SearchProvider*, ProviderData> providers_;

  QMap<int, DelayedSearch> delayed_searches_;

  int next_id_;
  QMap<int, int> pending_search_providers_;

  QPixmapCache pixmap_cache_;
  QMap<int, QString> pending_art_searches_;

  // Used for providers with ArtIsInSongMetadata set.
  AlbumCoverLoaderOptions cover_loader_options_;
  QMap<quint64, int> cover_loader_tasks_;

  // Special search provider that's used for queries that look like URLs
  UrlSearchProvider* url_provider_;
};

#endif  // GLOBALSEARCH_H
