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


class GlobalSearch : public QObject {
  Q_OBJECT

public:
  GlobalSearch(QObject* parent = 0);

  static const int kDelayedSearchTimeoutMs;
  static const char* kSettingsGroup;

  void AddProvider(SearchProvider* provider, bool enable_by_default = true);
  // Try to change provider state. Returns false if we can't (e.g. we can't
  // enable a provider because it requires the user to be logged-in)
  bool SetProviderEnabled(const SearchProvider* provider, bool enabled);

  int SearchAsync(const QString& query);
  int LoadArtAsync(const SearchProvider::Result& result);
  int LoadTracksAsync(const SearchProvider::Result& result);

  void CancelSearch(int id);
  void CancelArt(int id);

  bool FindCachedPixmap(const SearchProvider::Result& result, QPixmap* pixmap) const;

  QList<SearchProvider*> providers() const;
  bool is_provider_enabled(const SearchProvider* provider) const;

public slots:
  void ReloadSettings();

signals:
  void ResultsAvailable(int id, const SearchProvider::ResultList& results);
  void ProviderSearchFinished(int id, const SearchProvider* provider);
  void SearchFinished(int id);

  void ArtLoaded(int id, const QPixmap& pixmap);

  void TracksLoaded(int id, MimeData* mime_data);

  void ProviderAdded(const SearchProvider* provider);
  void ProviderRemoved(const SearchProvider* provider);
  void ProviderToggled(const SearchProvider* provider, bool enabled);

protected:
  void timerEvent(QTimerEvent* e);

private slots:
  void ResultsAvailableSlot(int id, SearchProvider::ResultList results);
  void SearchFinishedSlot(int id);

  void ArtLoadedSlot(int id, const QImage& image);

  void ProviderDestroyedSlot(QObject* object);

private:
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

  QMap<SearchProvider*, ProviderData> providers_;

  QMap<int, DelayedSearch> delayed_searches_;

  int next_id_;
  QMap<int, int> pending_search_providers_;

  QPixmapCache pixmap_cache_;
  QMap<int, QString> pending_art_searches_;

  QMap<QString, bool> providers_state_preference_;
};

#endif // GLOBALSEARCH_H
