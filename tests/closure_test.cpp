#include "gtest/gtest.h"

#include <QEventLoop>
#include <QFuture>
#include <QPromise>
#include <QString>

#include "config.h"
#include "core/closure.h"
#include "test_utils.h"

namespace {

// Runs the event loop until |done| is set, or until it gives up.
void SpinUntil(const bool& done) {
  QEventLoop loop;
  QTimer giving_up;
  giving_up.setSingleShot(true);
  QObject::connect(&giving_up, &QTimer::timeout, &loop, &QEventLoop::quit);
  giving_up.start(5000);
  while (!done && giving_up.isActive()) {
    loop.processEvents(QEventLoop::AllEvents, 10);
  }
}

}  // namespace

// The rules the overloads enforce, asserted in both directions. A receiver
// takes what the sender reports followed by the bound arguments, and nothing
// else compiles.
namespace {

using Invoke = decltype(&TestQObject::Invoke);
using InvokeWithArgs = decltype(&TestQObject::InvokeWithArgs);

// Signal arguments reach the receiver...
static_assert(_detail::ReceiverAccepts<InvokeWithArgs, TestQObject, int,
                                       QString>);
// ... and leaving them off is what a lambda is for.
static_assert(!_detail::ReceiverAccepts<Invoke, TestQObject, int, QString>);

// Bound arguments follow the signal's own.
static_assert(_detail::ReceiverAccepts<InvokeWithArgs, TestQObject, int,
                                       QString>);
static_assert(!_detail::ReceiverAccepts<InvokeWithArgs, TestQObject, QString,
                                        int>);

// A receiver of no arguments suits a signal that reports none.
static_assert(_detail::ReceiverAccepts<Invoke, TestQObject>);

// Callables follow the same rule, without the receiver.
static_assert(_detail::CallableAccepts<void (*)(int, QString), int, QString>);
static_assert(!_detail::CallableAccepts<void (*)(int), int, QString>);

// A future's result reaches the callable, unless it is a QFuture<void>.
static_assert(_detail::TakesFutureResult<int, void (*)(int)>::value);
static_assert(!_detail::TakesFutureResult<int, void (*)()>::value);
static_assert(_detail::TakesFutureResult<void, void (*)()>::value);
static_assert(!_detail::TakesFutureResult<void, void (*)(int)>::value);

// ... followed by the bound arguments.
static_assert(_detail::TakesFutureResult<int, void (*)(int, QString),
                                         QString>::value);
static_assert(!_detail::TakesFutureResult<int, void (*)(int), QString>::value);

}  // namespace

TEST(ClosureTest, CallsReceiver) {
  TestQObject sender;
  TestQObject receiver;
  NewClosure(&sender, &TestQObject::Emitted, &receiver, &TestQObject::Invoke);
  EXPECT_EQ(0, receiver.invoked());
  sender.Emit();
  EXPECT_EQ(1, receiver.invoked());
}

TEST(ClosureTest, CallsReceiverOnlyOnce) {
  TestQObject sender;
  TestQObject receiver;
  NewClosure(&sender, &TestQObject::Emitted, &receiver, &TestQObject::Invoke);
  sender.Emit();
  sender.Emit();
  EXPECT_EQ(1, receiver.invoked());
}

TEST(ClosureTest, ForwardsSignalArguments) {
  TestQObject sender;
  TestQObject receiver;
  NewClosure(&sender, &TestQObject::EmittedWithArgs, &receiver,
             &TestQObject::InvokeWithArgs);
  sender.EmitWithArgs(42, "towel");
  EXPECT_EQ(1, receiver.invoked());
  EXPECT_EQ(42, receiver.number());
  EXPECT_EQ("towel", receiver.text());
}

TEST(ClosureTest, PassesBoundArguments) {
  TestQObject sender;
  TestQObject context;
  int answer = 0;
  NewClosure(
      &sender, &TestQObject::Emitted, &context,
      [&answer](int bound) { answer = bound; }, 42);
  EXPECT_EQ(0, answer);
  sender.Emit();
  EXPECT_EQ(42, answer);
}

TEST(ClosureTest, PassesSignalArgumentsBeforeBoundOnes) {
  TestQObject sender;
  TestQObject context;
  QString seen;
  NewClosure(
      &sender, &TestQObject::EmittedWithArgs, &context,
      [&seen](int number, const QString& text, const QString& bound) {
        seen = QString("%1 %2 %3").arg(number).arg(text, bound);
      },
      QString("bound"));
  sender.EmitWithArgs(42, "signal");
  EXPECT_EQ("42 signal bound", seen);
}

TEST(ClosureTest, BoundArgumentsAreCopied) {
  TestQObject sender;
  TestQObject context;
  QString seen;
  QString bound("before");
  NewClosure(
      &sender, &TestQObject::Emitted, &context,
      [&seen](const QString& value) { seen = value; }, bound);
  bound = "after";
  sender.Emit();
  EXPECT_EQ("before", seen);
}

TEST(ClosureTest, StopsWhenReceiverIsDestroyed) {
  TestQObject sender;
  TestQObject* receiver = new TestQObject;
  bool called = false;
  NewClosure(&sender, &TestQObject::Emitted, receiver,
             [&called]() { called = true; });
  delete receiver;
  sender.Emit();
  EXPECT_FALSE(called);
}

TEST(ClosureTest, PassesFutureResult) {
  TestQObject receiver;
  QPromise<int> promise;
  QFuture<int> future = promise.future();

  bool called = false;
  int answer = 0;
  NewClosure(future, &receiver, [&](int result) {
    answer = result;
    called = true;
  });
  EXPECT_FALSE(called);

  promise.start();
  promise.addResult(42);
  promise.finish();

  SpinUntil(called);
  EXPECT_EQ(42, answer);
}

TEST(ClosureTest, PassesFutureResultBeforeBoundArguments) {
  TestQObject receiver;
  QPromise<int> promise;
  QFuture<int> future = promise.future();

  NewClosure(future, &receiver, &TestQObject::InvokeWithArgs,
             QString("bound"));

  promise.start();
  promise.addResult(42);
  promise.finish();

  bool done = false;
  DoAfter(&receiver, [&done]() { done = true; }, std::chrono::milliseconds(50));
  SpinUntil(done);
  EXPECT_EQ(42, receiver.number());
  EXPECT_EQ("bound", receiver.text());
}

TEST(ClosureTest, VoidFuturePassesOnlyBoundArguments) {
  TestQObject receiver;
  QPromise<void> promise;
  QFuture<void> future = promise.future();

  bool called = false;
  int answer = 0;
  NewClosure(
      future, &receiver,
      [&](int bound) {
        answer = bound;
        called = true;
      },
      42);

  promise.start();
  promise.finish();

  SpinUntil(called);
  EXPECT_EQ(42, answer);
}

TEST(ClosureTest, DoAfterCallsReceiver) {
  TestQObject receiver;
  DoAfter(&receiver, &TestQObject::Invoke, std::chrono::milliseconds(1));
  EXPECT_EQ(0, receiver.invoked());

  bool done = false;
  DoAfter(
      &receiver, [&done]() { done = true; }, std::chrono::milliseconds(50));
  SpinUntil(done);
  EXPECT_EQ(1, receiver.invoked());
}
