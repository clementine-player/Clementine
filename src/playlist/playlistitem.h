/* This file is part of Clementine.

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

#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QStandardItem>
#include <QUrl>

#include <boost/shared_ptr.hpp>

#include "core/song.h"

class QSqlQuery;

class PlaylistItem {
 public:
  PlaylistItem(const QString& type) : type_(type) {}
  virtual ~PlaylistItem() {}

  static PlaylistItem* NewFromType(const QString& type);

  enum Option {
    Default = 0x00,

    // The URL returned by Url() isn't the actual URL of the music - the
    // item needs to do something special before it can get an actual URL.
    // Causes StartLoading() to get called when the user wants to play.
    SpecialPlayBehaviour = 0x01,

    // This item might be able to provide another track after one finishes, for
    // example in a radio stream.  Causes LoadNext() to get called when the
    // next URL is required.
    ContainsMultipleTracks = 0x02,

    // Disables the "pause" action.
    PauseDisabled = 0x04,

    // Enables the last.fm "ban" action.
    LastFMControls = 0x08,
  };
  Q_DECLARE_FLAGS(Options, Option);

  // Returned by StartLoading() and LoadNext(), indicates what the player
  // should do when it wants to load a playlist item that is marked
  // SpecialPlayBehaviour or ContainsMultipleTracks.
  struct SpecialLoadResult {
    enum Type {
      // There wasn't a track available, and the player should move on to the
      // next playlist item.
      NoMoreTracks,

      // There might be another track available, something will call the
      // player's HandleSpecialLoad() slot later with the same original_url.
      WillLoadAsynchronously,

      // There was a track available.  Its url is in media_url.
      TrackAvailable,
    };

    SpecialLoadResult(Type type = NoMoreTracks,
                      const QUrl& original_url = QUrl(),
                      const QUrl& media_url = QUrl());

    Type type_;

    // The url that the playlist items has in Url().
    // Might be something unplayable like lastfm://...
    QUrl original_url_;

    // The actual url to something that gstreamer can play.
    QUrl media_url_;
  };

  virtual QString type() const { return type_; }

  virtual Options options() const { return Default; }

  virtual bool InitFromQuery(const QSqlQuery& query) = 0;
  void BindToQuery(QSqlQuery* query) const;
  virtual void Reload() {}

  virtual Song Metadata() const = 0;
  virtual QUrl Url() const = 0;

  // Called by the Player if SpecialPlayBehaviour is set - gives the playlist
  // item a chance to do something clever to get a playable track.
  virtual SpecialLoadResult StartLoading() { return SpecialLoadResult(); }

  // Called by the player if ContainsMultipleTracks is set - gives the playlist
  // item a chance to get another track to play.
  virtual SpecialLoadResult LoadNext() { return SpecialLoadResult(); }

  void SetTemporaryMetadata(const Song& metadata);
  void ClearTemporaryMetadata();
  bool HasTemporaryMetadata() const { return temp_metadata_.is_valid(); }

 protected:
  enum DatabaseColumn {
    Column_LibraryId,
    Column_Url,
    Column_Title,
    Column_Artist,
    Column_Album,
    Column_Length,
    Column_RadioService,
  };

  virtual QVariant DatabaseValue(DatabaseColumn) const {
    return QVariant(QVariant::String); }

  QString type_;

  Song temp_metadata_;
};
typedef QList<boost::shared_ptr<PlaylistItem> > PlaylistItemList;

Q_DECLARE_OPERATORS_FOR_FLAGS(PlaylistItem::Options);

#endif // PLAYLISTITEM_H
