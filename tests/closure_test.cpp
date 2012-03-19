#include "gtest/gtest.h"

#include <QCoreApplication>
#include <QSignalSpy>

#include "core/closure.h"
#include "test_utils.h"

class ClosureTest : public ::testing::Test {

};

TEST_F(ClosureTest, ClosureInvokesReceiver) {
  TestQObject sender;
  TestQObject receiver;
  Closure* closure = NewClosure(
      &sender, SIGNAL(Emitted()),
      &receiver, SLOT(Invoke()));
  EXPECT_EQ(0, receiver.invoked());
  sender.Emit();
  EXPECT_EQ(1, receiver.invoked());
}

TEST_F(ClosureTest, ClosureDeletesSelf) {
  TestQObject sender;
  TestQObject receiver;
  Closure* closure = NewClosure(
      &sender, SIGNAL(Emitted()),
      &receiver, SLOT(Invoke()));
  QSignalSpy spy(closure, SIGNAL(destroyed()));
  EXPECT_EQ(0, receiver.invoked());
  sender.Emit();
  EXPECT_EQ(1, receiver.invoked());

  EXPECT_EQ(0, spy.count());
  QEventLoop loop;
  QObject::connect(closure, SIGNAL(destroyed()), &loop, SLOT(quit()));
  loop.exec();
  EXPECT_EQ(1, spy.count());
}
