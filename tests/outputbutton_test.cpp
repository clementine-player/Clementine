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

  QString name_;
  QString icon_;

 protected:
  void SetVolumeSW(uint) override {}
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
        items << (action->isChecked() ? "* " : "  ") + action->text();
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

}  // namespace
