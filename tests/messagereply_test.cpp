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

#include "core/messagereply.h"

#include <QCoreApplication>
#include <QSignalSpy>
#include <thread>

#include "gtest/gtest.h"
#include "tagreadermessages.pb.h"
#include "test_utils.h"

namespace {

typedef MessageReply<cpb::tagreader::Message> Reply;

// A reply to request |id|, as a handler would set it.
cpb::tagreader::Message Response(int id) {
  cpb::tagreader::Message message;
  message.set_id(id);
  message.mutable_is_media_file_response()->set_success(true);
  return message;
}

Reply* NewReply(int id) {
  cpb::tagreader::Message request;
  request.set_id(id);
  return new Reply(request);
}

TEST(MessageReplyTest, WaitingGetsTheReply) {
  std::unique_ptr<Reply> reply(NewReply(1));
  std::thread handler([&reply]() { reply->SetReply(Response(1)); });

  EXPECT_TRUE(reply->WaitForFinished());
  handler.join();
  EXPECT_TRUE(reply->is_finished());
  EXPECT_TRUE(reply->message().is_media_file_response().success());
}

TEST(MessageReplyTest, FinishedArrivesOnTheRepliesThread) {
  std::unique_ptr<Reply> reply(NewReply(1));
  QSignalSpy finished(reply.get(), &_MessageReplyBase::Finished);

  std::thread handler([&reply]() { reply->SetReply(Response(1)); });
  handler.join();

  // Not emitted on the handler's thread, but here, from the event loop.
  EXPECT_EQ(0, finished.count());
  ASSERT_TRUE(finished.wait(5000));
  EXPECT_TRUE(finished[0][0].toBool());
}

TEST(MessageReplyTest, AbortingIsAnUnsuccessfulFinish) {
  std::unique_ptr<Reply> reply(NewReply(1));
  QSignalSpy finished(reply.get(), &_MessageReplyBase::Finished);

  std::thread handler([&reply]() { reply->Abort(); });
  EXPECT_FALSE(reply->WaitForFinished());
  handler.join();

  ASSERT_TRUE(finished.wait(5000));
  EXPECT_FALSE(finished[0][0].toBool());
}

TEST(MessageReplyTest, CanBeDeletedAsSoonAsTheWaitIsOver) {
  // What TagReaderClient's blocking calls do. The handler's thread used to
  // emit Finished after waking the waiter, which by then could have deleted
  // the reply.
  for (int i = 0; i < 2000; ++i) {
    Reply* reply = NewReply(i);
    std::thread handler([reply, i]() { reply->SetReply(Response(i)); });
    EXPECT_TRUE(reply->WaitForFinished());
    delete reply;
    handler.join();
  }
}

TEST(MessageReplyTest, DeletingItDropsAFinishedNotYetDelivered) {
  Reply* reply = NewReply(1);
  bool finished = false;
  QObject::connect(reply, &_MessageReplyBase::Finished,
                   [&finished]() { finished = true; });

  std::thread handler([reply]() { reply->SetReply(Response(1)); });
  EXPECT_TRUE(reply->WaitForFinished());
  handler.join();
  delete reply;

  QCoreApplication::processEvents();
  EXPECT_FALSE(finished);
}

}  // namespace
