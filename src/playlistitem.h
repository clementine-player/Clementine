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

class Song;
class SettingsProvider;

class PlaylistItem {
 public:
  PlaylistItem() {}
  virtual ~PlaylistItem() {}

  static PlaylistItem* NewFromType(const QString& type);

  enum Type {
    Type_Song,
    Type_Radio,
  };

  enum Option {
    Default = 0x00,

    SpecialPlayBehaviour = 0x01,
    ContainsMultipleTracks = 0x02,
    PauseDisabled = 0x04,
    LastFMControls = 0x08,
  };
  Q_DECLARE_FLAGS(Options, Option);

  virtual Type type() const = 0;
  QString type_string() const;

  virtual Options options() const { return Default; }

  virtual void Save(SettingsProvider* settings) const = 0;
  virtual void Restore(const SettingsProvider& settings) = 0;
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
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlaylistItem::Options);

#endif // PLAYLISTITEM_H
