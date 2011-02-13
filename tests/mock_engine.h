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

#ifndef MOCK_ENGINE_H
#define MOCK_ENGINE_H

#include <gmock/gmock.h>

#include "engines/enginebase.h"

class MockEngine : public Engine::Base {
public:
  MOCK_METHOD0(Init, bool());
  MOCK_METHOD1(CanDecode, bool(const QUrl&));

  MOCK_METHOD1(StartPreloading, void(const QUrl&));
  MOCK_METHOD1(Play, bool(quint64));
  MOCK_METHOD0(Stop, void());
  MOCK_METHOD0(Pause, void());
  MOCK_METHOD0(Unpause, void());
  MOCK_METHOD1(Seek, void(quint64));

  MOCK_METHOD1(AddBackgroundStream, int(const QUrl&));
  MOCK_METHOD1(StopBackgroundStream, void(int));
  MOCK_METHOD2(SetBackgroundStreamVolume, void(int, int));

  MOCK_CONST_METHOD0(state, Engine::State());
  MOCK_CONST_METHOD0(position_nanosec, qint64());
  MOCK_CONST_METHOD0(length_nanosec, qint64());

  MOCK_METHOD1(SetVolumeSW, void(uint));
};

#endif // MOCK_ENGINE_H
