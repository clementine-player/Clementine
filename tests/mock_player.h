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

#ifndef MOCK_PLAYER_H
#define MOCK_PLAYER_H

#include "core/player.h"
#include "core/song.h"
#include "core/settingsprovider.h"
#include "library/sqlrow.h"

#include <gmock/gmock.h>

class MockPlayer : public PlayerInterface {
public:
  MockPlayer() {}

  MOCK_CONST_METHOD0(engine, EngineBase*());
  MOCK_CONST_METHOD0(GetState, Engine::State());
  MOCK_CONST_METHOD0(GetVolume, int());

  MOCK_CONST_METHOD0(GetCurrentItem, PlaylistItemPtr());
  MOCK_CONST_METHOD1(GetItemAt, PlaylistItemPtr(int));
  MOCK_CONST_METHOD0(playlists, PlaylistManagerInterface*());

  MOCK_METHOD1(RegisterUrlHandler, void(UrlHandler*));
  MOCK_METHOD1(UnregisterUrlHandler, void(UrlHandler*));

  MOCK_METHOD0(ReloadSettings, void());

  MOCK_METHOD3(PlayAt, void(int, Engine::TrackChangeFlags, bool));
  MOCK_METHOD0(PlayPause, void());

  MOCK_METHOD0(Next, void());

  MOCK_METHOD0(Previous, void());
  MOCK_METHOD1(SetVolume, void(int));
  MOCK_METHOD0(VolumeUp, void());
  MOCK_METHOD0(VolumeDown, void());
  MOCK_METHOD1(SeekTo, void(int));
  MOCK_METHOD0(SeekForward, void());
  MOCK_METHOD0(SeekBackward, void());
  MOCK_METHOD1(CurrentMetadataChanged, void(const Song&));

  MOCK_METHOD0(Mute, void());
  MOCK_METHOD0(Pause, void());
  MOCK_METHOD0(Stop, void());
  MOCK_METHOD0(Play, void());
  MOCK_METHOD0(ShowOSD, void());
};

#endif // MOCK_PLAYER_H
