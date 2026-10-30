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

#include "networkremote/remotesmartplaylists.h"

#include <QMimeData>
#include <QSignalSpy>
#include <memory>

#include "gtest/gtest.h"
#include "smartplaylists/generatormimedata.h"
#include "smartplaylists/querygenerator.h"
#include "test_utils.h"

namespace {

using smart_playlists::GeneratorMimeData;

class RemoteSmartPlaylistsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    qRegisterMetaType<QMimeData*>("QMimeData*");
    RemoteSmartPlaylists::Hooks hooks;
    hooks.list = []() {
      QList<cpb::remote::SmartPlaylist> list;
      cpb::remote::SmartPlaylist newest;
      newest.set_id("smart/library/0b6e");
      newest.set_name("Newest tracks");
      list << newest;
      cpb::remote::SmartPlaylist mix;
      mix.set_id("smart/library/7d21");
      mix.set_name("Dynamic random mix");
      list << mix;
      return list;
    };
    hooks.generator = [](const QString& id) {
      if (id != "smart/library/0b6e") return smart_playlists::GeneratorPtr();
      return smart_playlists::GeneratorPtr(new smart_playlists::QueryGenerator(
          "Newest tracks", smart_playlists::Search()));
    };
    smart_playlists_.reset(new RemoteSmartPlaylists(hooks));
    sent_.reset(new QSignalSpy(smart_playlists_.get(),
                               SIGNAL(SendToClient(int, QByteArray))));
    added_.reset(new QSignalSpy(smart_playlists_.get(),
                                SIGNAL(AddToPlaylist(QMimeData*))));
  }

  // Sends |msg| from client 1, and returns the answer.
  cpb::remote::Message Send(const cpb::remote::Message& msg) {
    const std::string data = msg.SerializeAsString();
    smart_playlists_->HandleMessage(1, QByteArray(data.data(), data.size()));
    cpb::remote::Message reply;
    EXPECT_FALSE(sent_->isEmpty());
    if (sent_->isEmpty()) return reply;
    EXPECT_EQ(1, sent_->last()[0].toInt());
    const QByteArray out = sent_->last()[1].toByteArray();
    reply.ParseFromArray(out.constData(), out.size());
    return reply;
  }

  cpb::remote::ResponseSmartPlaylistAdd Add(
      const std::string& id, cpb::remote::BrowseAddAction action) {
    cpb::remote::Message msg;
    msg.set_type(cpb::remote::REQUEST_SMART_PLAYLIST_ADD);
    msg.mutable_request_smart_playlist_add()->set_smart_playlist_id(id);
    if (action != cpb::remote::BROWSE_ADD_ACTION_UNSPECIFIED) {
      msg.mutable_request_smart_playlist_add()->set_action(action);
    }
    const cpb::remote::Message reply = Send(msg);
    EXPECT_EQ(cpb::remote::SMART_PLAYLIST_ADD_RESULT, reply.type());
    return reply.response_smart_playlist_add();
  }

  // What the last add put on the playlist.
  std::unique_ptr<QMimeData> Added() {
    if (added_->isEmpty()) return nullptr;
    return std::unique_ptr<QMimeData>(added_->last()[0].value<QMimeData*>());
  }

  std::unique_ptr<RemoteSmartPlaylists> smart_playlists_;
  std::unique_ptr<QSignalSpy> sent_;
  std::unique_ptr<QSignalSpy> added_;
};

TEST_F(RemoteSmartPlaylistsTest, ListsTheSmartPlaylists) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::REQUEST_SMART_PLAYLISTS);
  msg.mutable_request_smart_playlists();
  const cpb::remote::Message reply = Send(msg);
  EXPECT_EQ(cpb::remote::SMART_PLAYLISTS, reply.type());
  const cpb::remote::ResponseSmartPlaylists& response =
      reply.response_smart_playlists();
  ASSERT_EQ(2, response.smart_playlists_size());
  EXPECT_EQ("smart/library/0b6e", response.smart_playlists(0).id());
  EXPECT_EQ("Newest tracks", response.smart_playlists(0).name());
  EXPECT_EQ("Dynamic random mix", response.smart_playlists(1).name());
}

TEST_F(RemoteSmartPlaylistsTest, PutsOneOnThePlaylist) {
  const cpb::remote::ResponseSmartPlaylistAdd response =
      Add("smart/library/0b6e", cpb::remote::BROWSE_ADD_ACTION_REPLACE);
  EXPECT_EQ("smart/library/0b6e", response.smart_playlist_id());
  EXPECT_EQ(cpb::remote::BROWSE_ADD_RESULT_ADDED, response.result());
  ASSERT_EQ(1, added_->size());
  std::unique_ptr<QMimeData> data = Added();
  const auto* generator = qobject_cast<GeneratorMimeData*>(data.get());
  ASSERT_NE(nullptr, generator);
  EXPECT_EQ("Newest tracks", generator->generator_->name());
  EXPECT_EQ("Newest tracks", generator->name_for_new_playlist_);
  EXPECT_TRUE(generator->override_user_settings_);
  EXPECT_TRUE(generator->clear_first_);
  EXPECT_TRUE(generator->play_now_);

  Add("smart/library/0b6e", cpb::remote::BROWSE_ADD_ACTION_PLAY_NEXT);
  ASSERT_EQ(2, added_->size());
  data = Added();
  EXPECT_TRUE(qobject_cast<GeneratorMimeData*>(data.get())->enqueue_next_now_);
  EXPECT_FALSE(qobject_cast<GeneratorMimeData*>(data.get())->play_now_);
}

TEST_F(RemoteSmartPlaylistsTest, DoesWhatDoubleClickingDoesWithNoAction) {
  Add("smart/library/0b6e", cpb::remote::BROWSE_ADD_ACTION_UNSPECIFIED);
  ASSERT_EQ(1, added_->size());
  std::unique_ptr<QMimeData> data = Added();
  const auto* generator = qobject_cast<GeneratorMimeData*>(data.get());
  ASSERT_NE(nullptr, generator);
  EXPECT_TRUE(generator->from_doubleclick_);
  EXPECT_FALSE(generator->override_user_settings_);
}

TEST_F(RemoteSmartPlaylistsTest, AnswersGoneForOnesThatArentThere) {
  EXPECT_EQ(cpb::remote::BROWSE_ADD_RESULT_GONE,
            Add("smart/library/9f1c", cpb::remote::BROWSE_ADD_ACTION_APPEND)
                .result());
  EXPECT_EQ(cpb::remote::BROWSE_ADD_RESULT_GONE,
            Add("playlist/7", cpb::remote::BROWSE_ADD_ACTION_APPEND).result());
  EXPECT_TRUE(added_->isEmpty());
}

TEST_F(RemoteSmartPlaylistsTest, IgnoresOtherMessages) {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::REQUEST_BROWSE);
  const std::string data = msg.SerializeAsString();
  smart_playlists_->HandleMessage(1, QByteArray(data.data(), data.size()));
  EXPECT_TRUE(sent_->isEmpty());
}

}  // namespace
