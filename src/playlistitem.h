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

class Song;

class QSqlQuery;

class PlaylistItem {
 public:
  PlaylistItem(const QString& type) : type_(type) {}
  virtual ~PlaylistItem() {}

  static PlaylistItem* NewFromType(const QString& type);

  enum Option {
    Default = 0x00,

    SpecialPlayBehaviour = 0x01,
    ContainsMultipleTracks = 0x02,
    PauseDisabled = 0x04,
    LastFMControls = 0x08,
  };
  Q_DECLARE_FLAGS(Options, Option);

  virtual QString type() const { return type_; }

  virtual Options options() const { return Default; }

  virtual bool InitFromQuery(const QSqlQuery& query) = 0;
  void BindToQuery(QSqlQuery* query) const;
  virtual void Reload() {}

  virtual Song Metadata() const = 0;

  // If the item needs to do anything special before it can play (eg. start
  // streaming the radio stream), then it should implement StartLoading() and
  // return true.  If it returns false then the URL from Url() will be passed
  // directly to xine instead.
  virtual void StartLoading() {}
  virtual QUrl Url() const = 0;

  // If the item is a radio station that can play another song after one has
  // finished then it should do so and return true
  virtual void LoadNext() {}

  virtual void SetTemporaryMetadata(const Song& metadata) {Q_UNUSED(metadata)}
  virtual void ClearTemporaryMetadata() {}

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
};
typedef QList<boost::shared_ptr<PlaylistItem> > PlaylistItemList;

Q_DECLARE_OPERATORS_FOR_FLAGS(PlaylistItem::Options);

#endif // PLAYLISTITEM_H
