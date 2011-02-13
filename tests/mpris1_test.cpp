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

#include "core/encoding.h"
#include "core/mpris1.h"
#include "core/song.h"
#include "radio/fixlastfm.h"
#include <lastfm/Track>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "test_utils.h"
#include "mock_player.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QTemporaryFile>
#include <QTextCodec>

#include <id3v2tag.h>

namespace {

class Mpris1Test : public ::testing::Test {
protected:
  void SetUp() {
  }

  QString service_name() const {
    return "org.clementine.unittest" +
        QString::number(QCoreApplication::applicationPid());
  }

  MockPlayer player_;
};


TEST_F(Mpris1Test, CreatesDBusService) {
  EXPECT_FALSE(QDBusConnection::sessionBus().interface()->
               isServiceRegistered(service_name()));

  boost::scoped_ptr<mpris::Mpris1> mpris(
      new mpris::Mpris1(&player_, NULL, NULL, service_name()));
  EXPECT_TRUE(QDBusConnection::sessionBus().interface()->
              isServiceRegistered(service_name()));

  mpris.reset();
  EXPECT_FALSE(QDBusConnection::sessionBus().interface()->
               isServiceRegistered(service_name()));
}

}  // namespace

