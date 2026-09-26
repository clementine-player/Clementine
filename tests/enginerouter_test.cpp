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

#include "engines/enginerouter.h"

#include <QSignalSpy>

#include "gtest/gtest.h"
#include "test_utils.h"

namespace {

// Records what it's asked to do, and reports whatever state it's told to.
class FakeEngine : public Engine::Base {
 public:
  bool Init() { return true; }
  bool Load(const MediaPlaybackRequest& req, Engine::TrackChangeFlags change,
            bool force_stop_at_end, quint64 beginning_nanosec,
            qint64 end_nanosec) {
    Engine::Base::Load(req, change, force_stop_at_end, beginning_nanosec,
                       end_nanosec);
    calls << "Load " + req.MediaUrl().toString();
    return true;
  }
  bool Play(quint64 offset_nanosec) {
    calls << QString("Play %1").arg(offset_nanosec);
    state_ = Engine::Playing;
    emit StateChanged(state_);
    return true;
  }
  void Stop(bool) {
    calls << "Stop";
    state_ = Engine::Empty;
    emit StateChanged(state_);
  }
  void Pause() {
    calls << "Pause";
    state_ = Engine::Paused;
    emit StateChanged(state_);
  }
  void Unpause() { calls << "Unpause"; }
  void Seek(quint64 offset_nanosec) {
    calls << QString("Seek %1").arg(offset_nanosec);
  }
  Engine::State state() const { return state_; }
  qint64 position_nanosec() const { return position_; }
  qint64 length_nanosec() const { return 0; }

  void EmitTrackEnded() { emit TrackEnded(); }

  QStringList calls;
  Engine::State state_ = Engine::Empty;
  qint64 position_ = 0;
  uint volume_set_ = 0;

 protected:
  void SetVolumeSW(uint) { volume_set_ = volume(); }
};

class EngineRouterTest : public ::testing::Test {
 protected:
  void SetUp() {
    local_ = new FakeEngine;
    router_.reset(new EngineRouter(local_));
    router_->AddOutput(&remote_);
  }

  // Base::Play(req, ...) is hidden by the Play(offset) overrides.
  void PlayUrl(const char* url) {
    static_cast<Engine::Base*>(router_.get())
        ->Play(MediaPlaybackRequest(QUrl(url)), Engine::First, false, 0, 0);
  }

  FakeEngine* local_;  // Owned by the router.
  FakeEngine remote_;
  std::unique_ptr<EngineRouter> router_;
};

}  // namespace

TEST_F(EngineRouterTest, ForwardsToTheLocalEngineByDefault) {
  EXPECT_TRUE(router_->is_local());
  PlayUrl("file:///a.mp3");
  EXPECT_EQ(QStringList({"Load file:///a.mp3", "Play 0"}), local_->calls);
  EXPECT_TRUE(remote_.calls.isEmpty());
}

TEST_F(EngineRouterTest, OnlyTheActiveEnginesSignalsGetThrough) {
  QSignalSpy ended(router_.get(), SIGNAL(TrackEnded()));
  remote_.EmitTrackEnded();
  EXPECT_EQ(0, ended.count());
  local_->EmitTrackEnded();
  EXPECT_EQ(1, ended.count());
}

TEST_F(EngineRouterTest, HandsOverPlaybackAtThePosition) {
  PlayUrl("file:///a.mp3");
  local_->position_ = 42;
  local_->calls.clear();

  QSignalSpy states(router_.get(), SIGNAL(StateChanged(Engine::State)));
  router_->SetOutput(&remote_);

  EXPECT_FALSE(router_->is_local());
  EXPECT_EQ(QStringList({"Stop"}), local_->calls);
  EXPECT_EQ(QStringList({"Load file:///a.mp3", "Play 42"}), remote_.calls);
  // The local engine stopping isn't reported; the remote one playing is.
  ASSERT_EQ(1, states.count());
  EXPECT_EQ(Engine::Playing, states[0][0].value<Engine::State>());
}

TEST_F(EngineRouterTest, HandsOverPausedPlaybackPaused) {
  PlayUrl("file:///a.mp3");
  router_->Pause();
  router_->SetOutput(&remote_);
  EXPECT_EQ(QStringList({"Load file:///a.mp3", "Play 0", "Pause"}),
            remote_.calls);
  EXPECT_EQ(Engine::Paused, router_->state());
}

TEST_F(EngineRouterTest, NothingPlayingMeansNothingToHandOver) {
  router_->SetOutput(&remote_);
  EXPECT_TRUE(remote_.calls.isEmpty());
  EXPECT_FALSE(router_->is_local());
}

TEST_F(EngineRouterTest, LosingTheActiveOutputFallsBackPaused) {
  router_->SetOutput(&remote_);
  PlayUrl("file:///a.mp3");
  remote_.position_ = 7;
  local_->calls.clear();

  router_->RemoveOutput(&remote_);
  EXPECT_TRUE(router_->is_local());
  EXPECT_EQ(QStringList({"Load file:///a.mp3", "Play 7", "Pause"}),
            local_->calls);
}

TEST_F(EngineRouterTest, NoCrossfadingOnARemoteOutput) {
  router_->SetOutput(&remote_);
  EXPECT_FALSE(router_->is_fadeout_enabled());
  EXPECT_FALSE(router_->is_crossfade_enabled());
  EXPECT_FALSE(router_->is_autocrossfade_enabled());
}

TEST_F(EngineRouterTest, VolumeGoesToTheNewOutput) {
  router_->SetVolume(30);
  router_->SetOutput(&remote_);
  EXPECT_EQ(30u, remote_.volume());
}
