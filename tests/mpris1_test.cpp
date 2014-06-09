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

#include <memory>

#include "core/encoding.h"
#include "core/mpris1.h"
#include "core/song.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistsequence.h"
#ifdef HAVE_LIBLASTFM
  #include "internet/lastfmcompat.h"
#endif

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "test_utils.h"
#include "mock_engine.h"
#include "mock_player.h"
#include "mock_playlistmanager.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTextCodec>

#include <id3v2tag.h>

using ::testing::_;
using ::testing::Return;

namespace {

class Mpris1BasicTest : public ::testing::Test {
protected:
  void SetUp() {
    sequence_.reset(new PlaylistSequence);

    EXPECT_CALL(player_, engine()).WillRepeatedly(Return(&engine_));
    EXPECT_CALL(player_, playlists()).WillRepeatedly(Return(&playlists_));
    EXPECT_CALL(playlists_, sequence()).WillRepeatedly(Return(sequence_.get()));
  }

  QString service_name() const {
    return "org.clementine.unittest" +
        QString::number(QCoreApplication::applicationPid());
  }

  MockEngine engine_;
  MockPlayer player_;
  MockPlaylistManager playlists_;

  std::unique_ptr<PlaylistSequence> sequence_;
};

TEST_F(Mpris1BasicTest, CreatesDBusService) {
  EXPECT_FALSE(QDBusConnection::sessionBus().interface()->
               isServiceRegistered(service_name()));

  std::unique_ptr<mpris::Mpris1> mpris(
      new mpris::Mpris1(&player_, nullptr, nullptr, service_name()));
  EXPECT_TRUE(QDBusConnection::sessionBus().interface()->
              isServiceRegistered(service_name()));

  mpris.reset();
  EXPECT_FALSE(QDBusConnection::sessionBus().interface()->
               isServiceRegistered(service_name()));
}


class Mpris1Test : public Mpris1BasicTest {
protected:
  void SetUp() {
    Mpris1BasicTest::SetUp();

    mpris_.reset(new mpris::Mpris1(&player_, nullptr, nullptr, service_name()));
  }

  std::unique_ptr<mpris::Mpris1> mpris_;
};

TEST_F(Mpris1Test, CorrectNameAndVersion) {
  QCoreApplication::setApplicationName("Banana");
  QCoreApplication::setApplicationVersion("Cheese");
  EXPECT_EQ("Banana Cheese", mpris_->root()->Identity());

  Version version = mpris_->root()->MprisVersion();
  EXPECT_EQ(1, version.major);
  EXPECT_EQ(0, version.minor);
}

TEST_F(Mpris1Test, Mutes) {
  EXPECT_CALL(player_, Mute());
  mpris_->player()->Mute();
}

TEST_F(Mpris1Test, GetsVolume) {
  EXPECT_CALL(player_, GetVolume()).WillOnce(Return(50));
  EXPECT_EQ(50, mpris_->player()->VolumeGet());
}

TEST_F(Mpris1Test, SetsVolume) {
  EXPECT_CALL(player_, SetVolume(42));
  mpris_->player()->VolumeSet(42);
}

TEST_F(Mpris1Test, RaisesVolume) {
  EXPECT_CALL(player_, GetVolume()).WillOnce(Return(50));
  EXPECT_CALL(player_, SetVolume(51));
  mpris_->player()->VolumeUp(1);
}

TEST_F(Mpris1Test, LowersVolume) {
  EXPECT_CALL(player_, GetVolume()).WillOnce(Return(50));
  EXPECT_CALL(player_, SetVolume(49));
  mpris_->player()->VolumeDown(1);
}

TEST_F(Mpris1Test, Pauses) {
  EXPECT_CALL(player_, PlayPause());
  mpris_->player()->Pause();
}

TEST_F(Mpris1Test, Stops) {
  EXPECT_CALL(player_, Stop());
  mpris_->player()->Stop();
}

TEST_F(Mpris1Test, Plays) {
  EXPECT_CALL(player_, Play());
  mpris_->player()->Play();
}

TEST_F(Mpris1Test, GoesPrevious) {
  EXPECT_CALL(player_, Previous());
  mpris_->player()->Prev();
}

TEST_F(Mpris1Test, GoesNext) {
  EXPECT_CALL(player_, Next());
  mpris_->player()->Next();
}

TEST_F(Mpris1Test, SetsPosition) {
  EXPECT_CALL(player_, SeekTo(42));
  mpris_->player()->PositionSet(42000);
}

TEST_F(Mpris1Test, GetsStatus) {
  // Engine statuses
  EXPECT_CALL(player_, GetState()).WillOnce(Return(Engine::Empty));
  DBusStatus status = mpris_->player()->GetStatus();
  EXPECT_EQ(DBusStatus::Mpris_Stopped, status.play);

  EXPECT_CALL(player_, GetState()).WillOnce(Return(Engine::Idle));
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(DBusStatus::Mpris_Stopped, status.play);

  EXPECT_CALL(player_, GetState()).WillOnce(Return(Engine::Paused));
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(DBusStatus::Mpris_Paused, status.play);

  EXPECT_CALL(player_, GetState()).WillOnce(Return(Engine::Playing));
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(DBusStatus::Mpris_Playing, status.play);

  EXPECT_CALL(player_, GetState()).WillRepeatedly(Return(Engine::Empty));

  // Repeat modes
  sequence_->SetRepeatMode(PlaylistSequence::Repeat_Off);
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(0, status.repeat);
  EXPECT_EQ(0, status.repeat_playlist);

  sequence_->SetRepeatMode(PlaylistSequence::Repeat_Album);
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(0, status.repeat);
  EXPECT_EQ(1, status.repeat_playlist);

  sequence_->SetRepeatMode(PlaylistSequence::Repeat_Playlist);
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(0, status.repeat);
  EXPECT_EQ(1, status.repeat_playlist);

  sequence_->SetRepeatMode(PlaylistSequence::Repeat_Track);
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(1, status.repeat);
  EXPECT_EQ(1, status.repeat_playlist);

  // Shuffle modes
  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_Off);
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(0, status.random);

  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_InsideAlbum);
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(1, status.random);

  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_Albums);
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(1, status.random);

  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_All);
  status = mpris_->player()->GetStatus();
  EXPECT_EQ(1, status.random);
}

TEST_F(Mpris1Test, HandlesShuffleModeChanged) {
  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_Off);
  EXPECT_CALL(player_, GetState()).WillRepeatedly(Return(Engine::Empty));

  QSignalSpy spy(mpris_->player(), SIGNAL(StatusChange(DBusStatus)));

  playlists_.EmitPlaylistManagerInitialized();
  EXPECT_EQ(0, spy.count());

  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_All);
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(1, spy[0][0].value<DBusStatus>().random);
  spy.clear();

  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_All);
  ASSERT_EQ(0, spy.count());

  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_InsideAlbum);
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(1, spy[0][0].value<DBusStatus>().random);
  spy.clear();

  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_Albums);
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(1, spy[0][0].value<DBusStatus>().random);
  spy.clear();

  sequence_->SetShuffleMode(PlaylistSequence::Shuffle_Off);
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(0, spy[0][0].value<DBusStatus>().random);
  spy.clear();
}

}  // namespace

