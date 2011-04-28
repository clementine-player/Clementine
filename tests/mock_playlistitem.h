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

#ifndef MOCK_PLAYLISTITEM_H
#define MOCK_PLAYLISTITEM_H

#include "core/song.h"
#include "core/settingsprovider.h"
#include "library/sqlrow.h"
#include "playlist/playlistitem.h"

#include <gmock/gmock.h>

class MockPlaylistItem : public PlaylistItem {
 public:
  MockPlaylistItem();

  MOCK_CONST_METHOD0(options,
      Options());
  MOCK_METHOD1(InitFromQuery,
      bool(const SqlRow& settings));
  MOCK_METHOD0(Reload,
      void());
  MOCK_CONST_METHOD0(Metadata,
      Song());
  MOCK_CONST_METHOD0(Url,
      QUrl());
  MOCK_METHOD1(SetTemporaryMetadata,
      void(const Song& metadata));
  MOCK_METHOD0(ClearTemporaryMetadata,
      void());
  MOCK_METHOD1(DatabaseValue,
      QVariant(DatabaseColumn));
};

#endif // MOCK_PLAYLISTITEM_H
