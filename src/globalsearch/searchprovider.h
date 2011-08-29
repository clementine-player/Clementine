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

#ifndef SEARCHPROVIDER_H
#define SEARCHPROVIDER_H

#include <QIcon>
#include <QMetaType>
#include <QObject>

#include "core/song.h"

class MimeData;


class SearchProvider : public QObject {
  Q_OBJECT

public:
  SearchProvider(QObject* parent = 0);

  static const int kArtHeight;

  struct Result {
    Result(SearchProvider* provider = 0)
      : provider_(provider), album_size_(0) {}

    // The order of types here is the order they'll appear in the UI.
    enum Type {
      Type_Track = 0,
      Type_Album
    };

    enum MatchQuality {
      // A token in the search string matched at the beginning of the song
      // metadata.
      Quality_AtStart = 0,

      // A token matched somewhere else.
      Quality_Middle,

      Quality_None
    };

    // This must be set by the provder using the constructor.
    SearchProvider* provider_;

    // These must be set explicitly by the provider.
    Type type_;
    Song metadata_;
    MatchQuality match_quality_;

    // How many songs in the album - valid only if type == Type_Album.
    int album_size_;

    QString pixmap_cache_key_;
  };
  typedef QList<Result> ResultList;

  const QString& name() const { return name_; }
  const QIcon& icon() const { return icon_; }
  const bool wants_delayed_queries() const { return delay_searches_; }
  const bool wants_serialised_art() const { return serialised_art_; }

  // Starts a search.  Must emit ResultsAvailable zero or more times and then
  // SearchFinished exactly once, using this ID.
  virtual void SearchAsync(int id, const QString& query) = 0;

  // Starts loading an icon for a result that was previously emitted by
  // ResultsAvailable.  Must emit ArtLoaded exactly once with this ID.
  virtual void LoadArtAsync(int id, const Result& result) = 0;

  // Starts loading tracks for a result that was previously emitted by
  // ResultsAvailable.  Must emit TracksLoaded exactly once with this ID.
  virtual void LoadTracksAsync(int id, const Result& result) = 0;

  static QImage ScaleAndPad(const QImage& image);

signals:
  void ResultsAvailable(int id, const SearchProvider::ResultList& results);
  void SearchFinished(int id);

  void ArtLoaded(int id, const QImage& image);

  void TracksLoaded(int id, MimeData* mime_data);

protected:
  // These functions treat queries in the same way as LibraryQuery.  They're
  // useful for figuring out whether you got a result because it matched in
  // the song title or the artist/album name.
  static QStringList TokenizeQuery(const QString& query);
  static Result::MatchQuality MatchQuality(const QStringList& tokens, const QString& string);

  // Subclasses must call this from their constructor
  void Init(const QString& name, const QIcon& icon,
            bool delay_searches, bool serialised_art);

private:
  QString name_;
  QIcon icon_;
  bool delay_searches_;
  bool serialised_art_;
};

Q_DECLARE_METATYPE(SearchProvider::Result)


class BlockingSearchProvider : public SearchProvider {
  Q_OBJECT

public:
  BlockingSearchProvider(QObject* parent = 0);

  void SearchAsync(int id, const QString& query);

protected:
  virtual ResultList Search(int id, const QString& query) = 0;

private slots:
  void BlockingSearchFinished();
};

#endif // SEARCHPROVIDER_H
