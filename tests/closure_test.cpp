#include "gtest/gtest.h"

#include <QCoreApplication>
#include <QPointer>
#include <QSharedPointer>
#include <QSignalSpy>

#include "core/closure.h"
#include "test_utils.h"

TEST(ClosureTest, ClosureInvokesReceiver) {
  TestQObject sender;
  TestQObject receiver;
  _detail::Closure* closure = NewClosure(
      &sender, SIGNAL(Emitted()),
      &receiver, SLOT(Invoke()));
  EXPECT_EQ(0, receiver.invoked());
  sender.Emit();
  EXPECT_EQ(1, receiver.invoked());
}

TEST(ClosureTest, ClosureDeletesSelf) {
  TestQObject sender;
  TestQObject receiver;
  _detail::Closure* closure = NewClosure(
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

TEST(ClosureTest, ClosureDoesNotCrashWithSharedPointerSender) {
  TestQObject receiver;
  TestQObject* sender;
  boost::scoped_ptr<QSignalSpy> spy;
  QPointer<_detail::Closure> closure;
  {
    QSharedPointer<TestQObject> sender_shared(new TestQObject);
    sender = sender_shared.data();
    closure = QPointer<_detail::Closure>(NewClosure(
        sender_shared, SIGNAL(Emitted()),
        &receiver, SLOT(Invoke())));
    spy.reset(new QSignalSpy(sender, SIGNAL(destroyed())));
  }
  EXPECT_EQ(0, receiver.invoked());
  sender->Emit();
  EXPECT_EQ(1, receiver.invoked());

  EXPECT_EQ(0, spy->count());
  QEventLoop loop;
  QObject::connect(sender, SIGNAL(destroyed()), &loop, SLOT(quit()));
  loop.exec();
  EXPECT_EQ(1, spy->count());
  EXPECT_TRUE(closure.isNull());
}
