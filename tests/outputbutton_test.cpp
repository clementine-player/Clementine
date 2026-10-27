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

#include "widgets/outputbutton.h"

#include <QMenu>
#include <QSignalSpy>

#include "engines/enginerouter.h"
#include "gtest/gtest.h"
#include "test_utils.h"
#include "ui/iconloader.h"

namespace {

// An output that does nothing, with a name and an icon.
class FakeEngine : public Engine::Base {
 public:
  explicit FakeEngine(const QString& name = QString(),
                      const QString& icon = QString())
      : name_(name), icon_(icon) {}

  bool Init() override { return true; }
  bool Play(quint64) override { return true; }
  void Stop(bool) override {}
  void Pause() override {}
  void Unpause() override {}
  void Seek(quint64) override {}
  Engine::State state() const override { return Engine::Empty; }
  qint64 position_nanosec() const override { return 0; }
  qint64 length_nanosec() const override { return 0; }
  QString output_name() const override { return name_; }
  QString output_icon() const override { return icon_; }
  QHostAddress output_address() const override { return address_; }

  QString name_;
  QString icon_;
  QHostAddress address_;

 protected:
  void SetVolumeSW(uint) override {}
};

// Answers the question about the network remote as told, instead of asking.
class TestOutputButton : public OutputButton {
 public:
  TestOutputButton(EngineRouter* router, RendererRegistry* registry)
      : OutputButton(router, registry) {}

  Answer answer_ = Answer::Cancel;
  QList<RendererRegistry::Reach> asked_;

 protected:
  Answer Ask(const QString&, const QString&,
             RendererRegistry::Reach reach) override {
    asked_ << reach;
    return answer_;
  }
};

class OutputButtonTest : public ::testing::Test {
 protected:
  OutputButtonTest()
      : router_(new FakeEngine),
        speaker_("Bedroom speaker", "cast"),
        phone_("Pixel", "phone") {
    // As main() does.
    IconLoader::Init();
  }

  QStringList MenuItems() const {
    QStringList items;
    for (QAction* action : button_->menu()->actions()) {
      if (action->isSeparator()) {
        items << "-";
      } else {
        items << (action->font().bold() ? "* " : "  ") + action->text();
      }
    }
    return items;
  }

  QAction* MenuItem(const QString& text) const {
    for (QAction* action : button_->menu()->actions()) {
      if (action->text() == text) return action;
    }
    return nullptr;
  }

  EngineRouter router_;
  FakeEngine speaker_;
  FakeEngine phone_;
  std::unique_ptr<OutputButton> button_;
};

TEST_F(OutputButtonTest, OnlyOfferedWithSomewhereElseToPlay) {
  button_.reset(new OutputButton(&router_));
  EXPECT_TRUE(button_->isHidden());

  router_.AddOutput(&speaker_);
  EXPECT_FALSE(button_->isHidden());

  router_.RemoveOutput(&speaker_);
  EXPECT_TRUE(button_->isHidden());
}

TEST_F(OutputButtonTest, ListsThisComputerThenTheOutputs) {
  router_.AddOutput(&speaker_);
  router_.AddOutput(&phone_);
  button_.reset(new OutputButton(&router_));

  EXPECT_EQ(
      QStringList({"* This computer", "-", "  Bedroom speaker", "  Pixel"}),
      MenuItems());
  for (QAction* action : button_->menu()->actions()) {
    if (!action->isSeparator()) EXPECT_FALSE(action->icon().isNull());
  }
  EXPECT_EQ("Play on another device", button_->toolTip());
}

TEST_F(OutputButtonTest, ChoosingAnOutputMovesPlaybackThere) {
  router_.AddOutput(&speaker_);
  button_.reset(new OutputButton(&router_));

  MenuItem("Bedroom speaker")->trigger();
  EXPECT_EQ(&speaker_, router_.active_engine());
  EXPECT_EQ(QStringList({"  This computer", "-", "* Bedroom speaker"}),
            MenuItems());
  EXPECT_EQ("Playing on Bedroom speaker", button_->toolTip());

  MenuItem("This computer")->trigger();
  EXPECT_TRUE(router_.is_local());
  EXPECT_EQ("Play on another device", button_->toolTip());
}

TEST_F(OutputButtonTest, FollowsChangesMadeElsewhere) {
  router_.AddOutput(&speaker_);
  button_.reset(new OutputButton(&router_));

  // Eg. a remote choosing the output.
  router_.SetOutput(&speaker_);
  EXPECT_EQ(QStringList({"  This computer", "-", "* Bedroom speaker"}),
            MenuItems());

  speaker_.name_ = "Kitchen speaker";
  router_.NotifyOutputsChanged();
  EXPECT_EQ(QStringList({"  This computer", "-", "* Kitchen speaker"}),
            MenuItems());
}

class OutputButtonRemoteTest : public OutputButtonTest {
 protected:
  OutputButtonRemoteTest() : registry_(nullptr, &router_) {
    // A Cast device, which fetches media from the network remote.
    speaker_.address_ = QHostAddress("127.0.0.2");
    router_.AddOutput(&speaker_);
    router_.AddOutput(&phone_);
    test_button_ = new TestOutputButton(&router_, &registry_);
    button_.reset(test_button_);
  }

  void Serve(quint16 port) {
    RendererRegistry::Serving serving;
    serving.enabled = true;
    serving.port = port;
    registry_.SetServing(serving);
  }

  RendererRegistry registry_;
  TestOutputButton* test_button_;
};

TEST_F(OutputButtonRemoteTest, PlaysOnDevicesThatCanFetchMedia) {
  Serve(5500);
  MenuItem("Bedroom speaker")->trigger();
  EXPECT_EQ(&speaker_, router_.active_engine());
  EXPECT_TRUE(test_button_->asked_.isEmpty());
}

TEST_F(OutputButtonRemoteTest, RenderersDontNeedAsking) {
  // They connect through the remote, so it must be on already.
  MenuItem("Pixel")->trigger();
  EXPECT_EQ(&phone_, router_.active_engine());
  EXPECT_TRUE(test_button_->asked_.isEmpty());
}

TEST_F(OutputButtonRemoteTest, AsksWhenTheRemoteIsOff) {
  QSignalSpy settings(test_button_, SIGNAL(ShowNetworkRemoteSettings()));

  MenuItem("Bedroom speaker")->trigger();
  EXPECT_EQ(QList<RendererRegistry::Reach>{RendererRegistry::Reach::RemoteOff},
            test_button_->asked_);
  EXPECT_TRUE(router_.is_local());

  test_button_->answer_ = OutputButton::Answer::Settings;
  MenuItem("Bedroom speaker")->trigger();
  EXPECT_EQ(1, settings.count());
  EXPECT_TRUE(router_.is_local());
}

TEST_F(OutputButtonRemoteTest, PlaysOnceTheRemoteIsTurnedOn) {
  QSignalSpy turn_on(test_button_, SIGNAL(TurnOnNetworkRemote()));
  test_button_->answer_ = OutputButton::Answer::TurnOn;

  MenuItem("Bedroom speaker")->trigger();
  EXPECT_EQ(1, turn_on.count());
  EXPECT_TRUE(router_.is_local());

  // Restarting the remote stops it first.
  registry_.SetServing(RendererRegistry::Serving());
  EXPECT_TRUE(router_.is_local());

  Serve(5500);
  EXPECT_EQ(&speaker_, router_.active_engine());
  EXPECT_EQ(1, test_button_->asked_.size());

  // Not again.
  router_.SetLocalOutput();
  Serve(5500);
  EXPECT_TRUE(router_.is_local());
}

TEST_F(OutputButtonRemoteTest, SaysWhenTurningTheRemoteOnWasntEnough) {
  test_button_->answer_ = OutputButton::Answer::TurnOn;
  MenuItem("Bedroom speaker")->trigger();

  // Its port is taken.
  test_button_->answer_ = OutputButton::Answer::Cancel;
  Serve(0);
  EXPECT_EQ(
      QList<RendererRegistry::Reach>({RendererRegistry::Reach::RemoteOff,
                                      RendererRegistry::Reach::NotListening}),
      test_button_->asked_);
  EXPECT_TRUE(router_.is_local());
}

}  // namespace
