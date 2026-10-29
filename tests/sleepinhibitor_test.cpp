/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "core/sleepinhibitor.h"

#include <QCoreApplication>
#include <QSettings>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <memory>
#include <string>

#include "gtest/gtest.h"

namespace {

using Activity = SleepInhibitor::Activity;
using Keep = SleepInhibitor::Keep;

// Records what it's asked, as "Inhibit DisplayOn (reason)" and the like.
class FakeBackend : public SleepInhibitor::Backend {
 public:
  explicit FakeBackend(QStringList* calls) : calls_(calls) {}

  void Inhibit(Keep keep, const QString& reason) override {
    EXPECT_FALSE(reason.isEmpty());
    *calls_ << "Inhibit " + Name(keep) + " (" + reason + ")";
  }
  void Uninhibit(Keep keep) override { *calls_ << "Uninhibit " + Name(keep); }

 private:
  static QString Name(Keep keep) {
    return keep == Keep::DisplayOn ? "DisplayOn" : "Awake";
  }

  QStringList* calls_;
};

class SleepInhibitorTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    settings_dir_ = new QTemporaryDir;
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                       settings_dir_->path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       settings_dir_->path());
  }

  static void TearDownTestCase() {
    delete settings_dir_;
    settings_dir_ = nullptr;
  }

  void SetUp() override {
    QSettings().remove(SleepInhibitor::kSettingsGroup);
    CreateInhibitor();
  }

  void CreateInhibitor() {
    inhibitor_ = std::make_unique<SleepInhibitor>(
        std::make_unique<FakeBackend>(&calls_));
    // Tests of the grace period set their own.
    inhibitor_->SetGraceMsec(0);
  }

  static void SetEnabled(Activity activity, bool enabled) {
    QSettings s;
    s.beginGroup(SleepInhibitor::kSettingsGroup);
    s.setValue(SleepInhibitor::SettingsKey(activity), enabled);
  }

  static QTemporaryDir* settings_dir_;

  // What the backend was asked, in order, as "Inhibit DisplayOn, ...", and
  // why if |with_reasons|.
  std::string Calls(bool with_reasons = false) const {
    QStringList calls;
    for (const QString& call : calls_) {
      calls << (with_reasons ? call : call.section(" (", 0, 0));
    }
    return calls.join(", ").toStdString();
  }

  QStringList calls_;
  std::unique_ptr<SleepInhibitor> inhibitor_;
};

QTemporaryDir* SleepInhibitorTest::settings_dir_ = nullptr;

TEST_F(SleepInhibitorTest, InhibitsWhileHeld) {
  std::unique_ptr<SleepInhibitor::Hold> hold =
      inhibitor_->Acquire(Activity::FullScreenVisualizations);
  EXPECT_EQ("Inhibit DisplayOn", Calls());

  hold.reset();
  EXPECT_EQ("Inhibit DisplayOn, Uninhibit DisplayOn", Calls());
}

TEST_F(SleepInhibitorTest, AsksOnceForManyHolds) {
  std::unique_ptr<SleepInhibitor::Hold> first =
      inhibitor_->Acquire(Activity::FullScreenVisualizations);
  std::unique_ptr<SleepInhibitor::Hold> second =
      inhibitor_->Acquire(Activity::FullScreenVisualizations);

  first.reset();
  EXPECT_EQ("Inhibit DisplayOn", Calls());

  second.reset();
  EXPECT_EQ("Inhibit DisplayOn, Uninhibit DisplayOn", Calls());
}

TEST_F(SleepInhibitorTest, DisabledActivityDoesNothing) {
  SetEnabled(Activity::FullScreenVisualizations, false);
  inhibitor_->ReloadSettings();

  std::unique_ptr<SleepInhibitor::Hold> hold =
      inhibitor_->Acquire(Activity::FullScreenVisualizations);
  hold.reset();
  EXPECT_TRUE(calls_.isEmpty());
}

TEST_F(SleepInhibitorTest, SettingsApplyToHoldsAlreadyTaken) {
  std::unique_ptr<SleepInhibitor::Hold> hold =
      inhibitor_->Acquire(Activity::FullScreenVisualizations);

  SetEnabled(Activity::FullScreenVisualizations, false);
  inhibitor_->ReloadSettings();
  EXPECT_EQ("Inhibit DisplayOn, Uninhibit DisplayOn", Calls());

  SetEnabled(Activity::FullScreenVisualizations, true);
  inhibitor_->ReloadSettings();
  EXPECT_EQ("Inhibit DisplayOn, Uninhibit DisplayOn, Inhibit DisplayOn",
            Calls());
}

TEST_F(SleepInhibitorTest, LetsGoWhenDeleted) {
  std::unique_ptr<SleepInhibitor::Hold> hold =
      inhibitor_->Acquire(Activity::FullScreenVisualizations);
  hold.reset();
  calls_.clear();

  hold = inhibitor_->Acquire(Activity::FullScreenVisualizations);
  // A hold mustn't outlive its inhibitor, so let go of it without telling it.
  hold.release();
  inhibitor_.reset();
  EXPECT_EQ("Inhibit DisplayOn, Uninhibit DisplayOn", Calls());
}

TEST_F(SleepInhibitorTest, HoldsWorkFromOtherThreads) {
  std::unique_ptr<SleepInhibitor::Hold> hold;
  QThread* thread = QThread::create([this, &hold]() {
    hold = inhibitor_->Acquire(Activity::FullScreenVisualizations);
  });
  thread->start();
  thread->wait();
  // Not until the inhibitor's thread gets to it.
  EXPECT_TRUE(calls_.isEmpty());
  QCoreApplication::processEvents();
  EXPECT_EQ("Inhibit DisplayOn", Calls());

  delete thread;
  thread = QThread::create([&hold]() { hold.reset(); });
  thread->start();
  thread->wait();
  QCoreApplication::processEvents();
  EXPECT_EQ("Inhibit DisplayOn, Uninhibit DisplayOn", Calls());
  delete thread;
}

TEST_F(SleepInhibitorTest, PlaybackKeepsAwake) {
  std::unique_ptr<SleepInhibitor::Hold> hold =
      inhibitor_->Acquire(Activity::LocalPlayback);
  EXPECT_EQ("Inhibit Awake", Calls());

  hold.reset();
  EXPECT_EQ("Inhibit Awake, Uninhibit Awake", Calls());
}

TEST_F(SleepInhibitorTest, KeepsAwakeForAWhileAfterwards) {
  inhibitor_->SetGraceMsec(50);
  std::unique_ptr<SleepInhibitor::Hold> hold =
      inhibitor_->Acquire(Activity::RemoteTransfer);
  hold.reset();
  // Such as between the files a remote downloads.
  hold = inhibitor_->Acquire(Activity::RemoteTransfer);
  hold.reset();
  EXPECT_EQ("Inhibit Awake", Calls());

  QTest::qWait(200);
  EXPECT_EQ("Inhibit Awake, Uninhibit Awake", Calls());
}

TEST_F(SleepInhibitorTest, DisplayGoesStraightAway) {
  inhibitor_->SetGraceMsec(60000);
  std::unique_ptr<SleepInhibitor::Hold> hold =
      inhibitor_->Acquire(Activity::FullScreenVisualizations);
  hold.reset();
  EXPECT_EQ("Inhibit DisplayOn, Uninhibit DisplayOn", Calls());
}

TEST_F(SleepInhibitorTest, AsksAgainWhenTheReasonChanges) {
  std::unique_ptr<SleepInhibitor::Hold> playback =
      inhibitor_->Acquire(Activity::LocalPlayback);
  std::unique_ptr<SleepInhibitor::Hold> transfer =
      inhibitor_->Acquire(Activity::RemoteTransfer);
  EXPECT_EQ(
      "Inhibit Awake (Playing music), Uninhibit Awake, "
      "Inhibit Awake (Sending songs to a remote)",
      Calls(/* with_reasons= */ true));

  // Still awake for playback, with its own reason.
  transfer.reset();
  EXPECT_EQ(
      "Inhibit Awake (Playing music), Uninhibit Awake, "
      "Inhibit Awake (Sending songs to a remote), Uninhibit Awake, "
      "Inhibit Awake (Playing music)",
      Calls(/* with_reasons= */ true));
}

TEST_F(SleepInhibitorTest, WhileOpenIsOffByDefault) {
  EXPECT_TRUE(calls_.isEmpty());

  SetEnabled(Activity::WhileOpen, true);
  inhibitor_->ReloadSettings();
  EXPECT_EQ("Inhibit Awake", Calls());
}

TEST_F(SleepInhibitorTest, WhileOpenFromTheStart) {
  SetEnabled(Activity::WhileOpen, true);
  inhibitor_.reset();
  calls_.clear();

  CreateInhibitor();
  EXPECT_EQ("Inhibit Awake", Calls());
}

TEST_F(SleepInhibitorTest, OtherDeviceCanBeTurnedOffAlone) {
  SetEnabled(Activity::PlaybackOnOtherDevice, false);
  inhibitor_->ReloadSettings();

  std::unique_ptr<SleepInhibitor::Hold> other =
      inhibitor_->Acquire(Activity::PlaybackOnOtherDevice);
  EXPECT_TRUE(calls_.isEmpty());
  std::unique_ptr<SleepInhibitor::Hold> local =
      inhibitor_->Acquire(Activity::LocalPlayback);
  EXPECT_EQ("Inhibit Awake", Calls());
}

}  // namespace
