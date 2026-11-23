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

#include "core/backgroundstreams.h"

#include <QCoreApplication>
#include <QSettings>
#include <QSignalSpy>
#include <memory>

#include "engines/enginebase.h"
#include "gtest/gtest.h"
#include "test_utils.h"

namespace {

// Starts background streams with the ids it's told to, and records the
// volumes it's asked to set.
class FakeEngine : public Engine::Base {
 public:
  bool Init() override { return true; }
  bool Play(quint64) override { return true; }
  void Stop(bool) override {}
  void Pause() override {}
  void Unpause() override {}
  void Seek(quint64) override {}
  Engine::State state() const override { return Engine::Empty; }
  qint64 position_nanosec() const override { return 0; }
  qint64 length_nanosec() const override { return 0; }

  int AddBackgroundStream(const QUrl&) override { return next_stream_id; }
  void SetBackgroundStreamVolume(int id, int volume) override {
    volumes_set << qMakePair(id, volume);
  }

  int next_stream_id = -1;
  QList<QPair<int, int>> volumes_set;

 protected:
  void SetVolumeSW(uint) override {}
};

class BackgroundStreamsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    QCoreApplication::setOrganizationName("ClementineTests");
    QCoreApplication::setApplicationName("backgroundstreams_test");
    Clear();
    streams_.reset(new BackgroundStreams(&engine_));
    streams_->LoadStreams();
  }

  void TearDown() override {
    streams_.reset();
    Clear();
  }

  void Clear() { QSettings().remove("BackgroundStreams"); }

  FakeEngine engine_;
  std::unique_ptr<BackgroundStreams> streams_;
};

TEST_F(BackgroundStreamsTest, StartsStreamAtItsVolume) {
  engine_.next_stream_id = 7;
  QSignalSpy started(streams_.get(), &BackgroundStreams::StreamStarted);

  streams_->EnableStream("Rain", true);

  EXPECT_TRUE(streams_->IsPlaying("Rain"));
  ASSERT_EQ(1, engine_.volumes_set.size());
  EXPECT_EQ(7, engine_.volumes_set[0].first);
  EXPECT_EQ(1, started.count());
}

// When the engine can't start a stream, nothing is sent on to a stream that
// doesn't exist, and it isn't shown as playing.
TEST_F(BackgroundStreamsTest, StreamThatFailsToStartIsNotPlaying) {
  engine_.next_stream_id = -1;
  QSignalSpy started(streams_.get(), &BackgroundStreams::StreamStarted);

  streams_->EnableStream("Rain", true);
  streams_->SetStreamVolume("Rain", 80);

  EXPECT_FALSE(streams_->IsPlaying("Rain"));
  EXPECT_TRUE(engine_.volumes_set.isEmpty());
  EXPECT_EQ(0, started.count());
}

}  // namespace
