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

#include <QFuture>
#include <QIcon>
#include <QMetaType>
#include <QObject>

#include "core/song.h"

class Application;
class InternetService;
class MimeData;

class SearchProvider : public QObject {
  Q_OBJECT

 public:
  SearchProvider(Application* app, QObject* parent = nullptr);

  static const int kArtHeight;

  struct Result {
    Result(SearchProvider* provider = 0)
        : provider_(provider), group_automatically_(true) {}

    // This must be set by the provider using the constructor.
    SearchProvider* provider_;

    // If this is set to true, the view will group this result into
    // artist/album categories as appropriate.
    bool group_automatically_;

    // Must be set by the provider.
    Song metadata_;

    // This is set and used by the GlobalSearch engine itself.
    QString pixmap_cache_key_;
  };
  typedef QList<Result> ResultList;

  enum Hint {
    NoHints = 0x00,

    // Indicates that queries to this provider mean making requests to a third
    // party.  To be polite, queries should be buffered by a few milliseconds
    // instead of executing them each time the user types a character.
    WantsDelayedQueries = 0x01,

    // Indicates that this provider wants to be given art queries one after the
    // other (serially), instead of all at once (in parallel).
    WantsSerialisedArtQueries = 0x02,

    // Indicates that album cover art is probably going to be loaded remotely.
    // If a third-party application is making art requests over dbus and has
    // to get all the art it can before showing results to the user, it might
    // not load art from this provider.
    ArtIsProbablyRemote = 0x04,

    // Indicates the art URL (or filename) for each result is stored in the
    // normal place in the song metadata.  LoadArtAsync will never be called and
    // WantsSerialisedArtQueries and ArtIsProbablyRemote will be ignored if
    // they are set as well.  The GlobalSearch engine will load the art itself.
    ArtIsInSongMetadata = 0x08,

    // Indicates this provider has a config dialog that can be shown by calling
    // ShowConfig.  If this is not set then the button will be greyed out
    // in the GUI.
    CanShowConfig = 0x10,

    // This provider can provide some example search strings to display in the
    // UI.
    CanGiveSuggestions = 0x20,

    // Normally providers get enabled unless the user chooses otherwise.
    // Setting this flag indicates that this provider is disabled by default
    // instead.
    DisabledByDefault = 0x40,

    // The default implementation of LoadTracksAsync normally creates a
    // SongMimeData containing the entire metadata for each result being loaded.
    // Setting this flag will cause a plain MimeData to be created containing
    // only the URLs of the results.
    MimeDataContainsUrlsOnly = 0x80
  };
  Q_DECLARE_FLAGS(Hints, Hint)

  const QString& name() const { return name_; }
  const QString& id() const { return id_; }
  const QIcon& icon() const { return icon_; }
  const QImage& icon_as_image() const { return icon_as_image_; }

  Hints hints() const { return hints_; }
  bool wants_delayed_queries() const { return hints() & WantsDelayedQueries; }
  bool wants_serialised_art() const {
    return hints() & WantsSerialisedArtQueries;
  }
  bool art_is_probably_remote() const { return hints() & ArtIsProbablyRemote; }
  bool art_is_in_song_metadata() const { return hints() & ArtIsInSongMetadata; }
  bool can_show_config() const { return hints() & CanShowConfig; }
  bool can_give_suggestions() const { return hints() & CanGiveSuggestions; }
  bool is_disabled_by_default() const { return hints() & DisabledByDefault; }
  bool is_enabled_by_default() const { return !is_disabled_by_default(); }
  bool mime_data_contains_urls_only() const {
    return hints() & MimeDataContainsUrlsOnly;
  }

  // Starts a search.  Must emit ResultsAvailable zero or more times and then
  // SearchFinished exactly once, using this ID.
  virtual void SearchAsync(int id, const QString& query) = 0;

  // Starts loading an icon for a result that was previously emitted by
  // ResultsAvailable.  Must emit ArtLoaded exactly once with this ID.
  virtual void LoadArtAsync(int id, const Result& result);

  // Loads tracks for results that were previously emitted by ResultsAvailable.
  // The default implementation creates a SongMimeData with one Song for each
  // Result, unless the MimeDataContainsUrlsOnly flag is set.
  virtual MimeData* LoadTracks(const ResultList& results);

  // Returns some example search strings to display in the UI.  The provider
  // should pick some of its items at random and return between 0 and count
  // strings.  Remember to set the CanGiveSuggestions hint.
  virtual QStringList GetSuggestions(int count) { return QStringList(); }

  // If provider needs user login to search and play songs, this method should
  // be reimplemented
  virtual bool IsLoggedIn() { return true; }
  virtual void ShowConfig() {}  // Remember to set the CanShowConfig hint
  // Returns the Internet service in charge of this provider, or nullptr if
  // there is none.
  virtual InternetService* internet_service() { return nullptr; }

  static QImage ScaleAndPad(const QImage& image);

 signals:
  void ResultsAvailable(int id, const SearchProvider::ResultList& results);
  void SearchFinished(int id);

  void ArtLoaded(int id, const QImage& image);

 protected:
  // These functions treat queries in the same way as LibraryQuery.  They're
  // useful for figuring out whether you got a result because it matched in
  // the song title or the artist/album name.
  static QStringList TokenizeQuery(const QString& query);
  static bool Matches(const QStringList& tokens, const QString& string);

  // Subclasses must call this from their constructors.
  void Init(const QString& name, const QString& id, const QIcon& icon,
            Hints hints = NoHints);
  void SetHint(Hint hint, bool set = true);

  struct PendingState {
    PendingState() : orig_id_(-1) {}
    PendingState(int orig_id, QStringList tokens)
        : orig_id_(orig_id), tokens_(tokens) {}
    int orig_id_;
    QStringList tokens_;

    bool operator<(const PendingState& b) const {
      return orig_id_ < b.orig_id_;
    }

    bool operator==(const PendingState& b) const {
      return orig_id_ == b.orig_id_;
    }
  };

 protected:
  Application* app_;

 private:
  QString name_;
  QString id_;
  QIcon icon_;
  Hints hints_;
  QImage icon_as_image_;
};

Q_DECLARE_METATYPE(SearchProvider::Result)
Q_DECLARE_METATYPE(SearchProvider::ResultList)
Q_DECLARE_OPERATORS_FOR_FLAGS(SearchProvider::Hints)

class BlockingSearchProvider : public SearchProvider {
  Q_OBJECT

 public:
  BlockingSearchProvider(Application* app, QObject* parent = nullptr);

  void SearchAsync(int id, const QString& query);
  virtual ResultList Search(int id, const QString& query) = 0;

 private slots:
  void BlockingSearchFinished(QFuture<ResultList> future, const int id);
};

Q_DECLARE_METATYPE(SearchProvider*)

#endif  // SEARCHPROVIDER_H
