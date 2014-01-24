#include "gtest/gtest.h"

#include <QCoreApplication>
#include <QPointer>
#include <QSharedPointer>
#include <QSignalSpy>

#include <boost/function.hpp>

#include "config.h"
#include "core/closure.h"
#include "test_utils.h"

TEST(ClosureTest, ClosureInvokesReceiver) {
  TestQObject sender;
  TestQObject receiver;
  _detail::ClosureBase* closure = NewClosure(
      &sender, SIGNAL(Emitted()),
      &receiver, SLOT(Invoke()));
  EXPECT_EQ(0, receiver.invoked());
  sender.Emit();
  EXPECT_EQ(1, receiver.invoked());
}

TEST(ClosureTest, ClosureDeletesSelf) {
  TestQObject sender;
  TestQObject receiver;
  _detail::ClosureBase* closure = NewClosure(
      &sender, SIGNAL(Emitted()),
      &receiver, SLOT(Invoke()));
  _detail::ObjectHelper* helper = closure->helper();
  QSignalSpy spy(helper, SIGNAL(destroyed()));
  EXPECT_EQ(0, receiver.invoked());
  sender.Emit();
  EXPECT_EQ(1, receiver.invoked());

  EXPECT_EQ(0, spy.count());
  QEventLoop loop;
  QObject::connect(helper, SIGNAL(destroyed()), &loop, SLOT(quit()));
  loop.exec();
  EXPECT_EQ(1, spy.count());
}

TEST(ClosureTest, ClosureDoesNotCrashWithSharedPointerSender) {
  TestQObject receiver;
  TestQObject* sender;
  boost::scoped_ptr<QSignalSpy> spy;
  QPointer<_detail::ObjectHelper> closure;
  {
    QSharedPointer<TestQObject> sender_shared(new TestQObject);
    sender = sender_shared.data();
    closure = QPointer<_detail::ObjectHelper>(NewClosure(
        sender_shared, SIGNAL(Emitted()),
        &receiver, SLOT(Invoke()))->helper());
    spy.reset(new QSignalSpy(sender, SIGNAL(destroyed())));
  }
  ASSERT_EQ(0, receiver.invoked());
  sender->Emit();
  ASSERT_EQ(1, receiver.invoked());

  ASSERT_EQ(0, spy->count());
  QEventLoop loop;
  QObject::connect(sender, SIGNAL(destroyed()), &loop, SLOT(quit()));
  loop.exec();
  ASSERT_EQ(1, spy->count());
  EXPECT_TRUE(closure.isNull());
}

namespace {

void Foo(bool* called, int question, int* answer) {
  *called = true;
  *answer = question;
}

}  // namespace

TEST(ClosureTest, ClosureWorksWithFunctionPointers) {
  TestQObject sender;
  bool called = false;
  int question = 42;
  int answer = 0;
  NewClosure(
      &sender, SIGNAL(Emitted()),
      &Foo, &called, question, &answer);
  EXPECT_FALSE(called);
  sender.Emit();
  EXPECT_TRUE(called);
  EXPECT_EQ(question, answer);
}

TEST(ClosureTest, ClosureWorksWithStandardFunctions) {
  TestQObject sender;
  bool called = false;
  int question = 42;
  int answer = 0;
  boost::function<void(bool*,int,int*)> callback(&Foo);
  NewClosure(
      &sender, SIGNAL(Emitted()),
      callback, &called, question, &answer);
  EXPECT_FALSE(called);
  sender.Emit();
  EXPECT_TRUE(called);
  EXPECT_EQ(question, answer);
}

namespace {

class Bar {
 public:
  explicit Bar(int a) : foo_(a) {}
  bool Foo(int* answer) {
    *answer = foo_;
    return true;
  }

 private:
  int foo_;
};

}

TEST(ClosureTest, ClosureWorksWithMemberFunctionPointers) {
  TestQObject sender;
  Bar receiver(42);
  int q = 1;
  NewClosure(
      &sender, SIGNAL(Emitted()),
      &receiver, &Bar::Foo, &q);
  EXPECT_EQ(1, q);
  sender.Emit();
  EXPECT_EQ(42, q);
}

#ifdef HAVE_LAMBDAS
TEST(ClosureTest, ClosureCallsLambda) {
  TestQObject sender;
  bool called = false;
  NewClosure(
      &sender, SIGNAL(Emitted()),
      [&called] () { called = true; });
  EXPECT_FALSE(called);
  sender.Emit();
  EXPECT_TRUE(called);
}
#endif  // HAVE_LAMBDAS
