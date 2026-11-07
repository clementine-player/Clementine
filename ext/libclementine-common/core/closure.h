/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#ifndef CLOSURE_H
#define CLOSURE_H

#include <chrono>
#include <type_traits>

#include <QFuture>
#include <QFutureWatcher>
#include <QMetaObject>
#include <QObject>
#include <QTimer>

// Calls a receiver the first time a signal is emitted, then disconnects,
// passing along arguments bound at the point the connection was made:
//
//   NewClosure(reply, &QNetworkReply::finished,
//              this, &Foo::HandleReply, reply, id);
//     -> HandleReply(reply, id)
//
// The receiver is handed the signal's own arguments first, then the bound
// ones, and has to take both:
//
//   NewClosure(reply, &QNetworkReply::errorOccurred,
//              this, &Foo::HandleError, reply, id);
//     -> HandleError(QNetworkReply::NetworkError, reply, id)
//
// Ignoring what a signal reports is worth saying out loud, so do it with a
// lambda rather than by leaving arguments off the receiver:
//
//   NewClosure(reply, &QNetworkReply::errorOccurred, this,
//              [this, id](QNetworkReply::NetworkError) { Failed(id); });
//
// All of it is checked while compiling: that the sender has the signal, that
// the receiver has the method, and that the method accepts what it is about
// to be handed. Overloaded signals need naming, as
// qOverload<bool>(&Foo::Finished).
//
// The connection ends with either the sender or the receiver, whichever goes
// first. Bound arguments are copied when the connection is made, so a raw
// pointer among them has to outlive the signal.

namespace _detail {

// The rule every overload below enforces, named so that the tests can assert
// on it directly: a receiver takes whatever the sender reports - a signal's
// arguments, or a future's result - followed by the bound arguments.
template <typename Slot, typename Receiver, typename... Args>
inline constexpr bool ReceiverAccepts =
    std::is_invocable_v<Slot, Receiver*, Args...>;

// The same rule for a lambda or any other callable.
template <typename Func, typename... Args>
inline constexpr bool CallableAccepts = std::is_invocable_v<Func, Args...>;

// A QFuture<void> reports no result, so only the bound arguments reach the
// callable.
template <typename T, typename Func, typename... BoundArgs>
struct TakesFutureResult
    : std::bool_constant<CallableAccepts<Func, T, BoundArgs...>> {};

template <typename Func, typename... BoundArgs>
struct TakesFutureResult<void, Func, BoundArgs...>
    : std::bool_constant<CallableAccepts<Func, BoundArgs...>> {};

}  // namespace _detail

template <typename Sender, typename SignalOwner, typename... SignalArgs,
          typename Receiver, typename Slot, typename... BoundArgs>
  requires std::is_member_function_pointer_v<Slot>
QMetaObject::Connection NewClosure(Sender* sender,
                                   void (SignalOwner::*signal)(SignalArgs...),
                                   Receiver* receiver, Slot slot,
                                   BoundArgs... bound) {
  static_assert(
      _detail::ReceiverAccepts<Slot, Receiver, SignalArgs..., BoundArgs...>,
      "The receiver must take the signal's arguments followed by the bound "
      "arguments. Pass a lambda instead to ignore the signal's arguments.");
  return QObject::connect(
      sender, signal, receiver,
      [receiver, slot, bound...](SignalArgs... signal_args) {
        (receiver->*slot)(signal_args..., bound...);
      },
      Qt::SingleShotConnection);
}

// As above, for a lambda or any other callable. |context| decides both which
// thread it runs on and how long the connection lives, so pass the object
// whose lifetime the callable depends on.
template <typename Sender, typename SignalOwner, typename... SignalArgs,
          typename Func, typename... BoundArgs>
  requires(!std::is_member_function_pointer_v<std::decay_t<Func>>)
QMetaObject::Connection NewClosure(Sender* sender,
                                   void (SignalOwner::*signal)(SignalArgs...),
                                   QObject* context, Func func,
                                   BoundArgs... bound) {
  static_assert(
      _detail::CallableAccepts<Func, SignalArgs..., BoundArgs...>,
      "The callable must take the signal's arguments followed by the bound "
      "arguments.");
  return QObject::connect(
      sender, signal, context,
      [func, bound...](SignalArgs... signal_args) {
        func(signal_args..., bound...);
      },
      Qt::SingleShotConnection);
}

// Calls the receiver once the future finishes, with its result followed by
// the bound arguments - the same order as a signal's own arguments:
//
//   NewClosure(future, this, &Foo::Loaded, id);
//     -> Loaded(SongList, int)
//
// A QFuture<void> has no result to report, so those receivers take only the
// bound arguments. To reach the future itself - to check whether it was
// cancelled, or to read more than its first result - use the callable
// overload and capture it in a lambda.
template <typename T, typename Receiver, typename Slot, typename... BoundArgs>
  requires std::is_member_function_pointer_v<Slot>
QMetaObject::Connection NewClosure(QFuture<T> future, Receiver* receiver,
                                   Slot slot, BoundArgs... bound) {
  if constexpr (std::is_void_v<T>) {
    static_assert(_detail::ReceiverAccepts<Slot, Receiver, BoundArgs...>,
                  "A QFuture<void> has no result, so the receiver must take "
                  "just the bound arguments.");
  } else {
    static_assert(_detail::ReceiverAccepts<Slot, Receiver, T, BoundArgs...>,
                  "The receiver must take the future's result followed by the "
                  "bound arguments.");
  }
  QFutureWatcher<T>* watcher = new QFutureWatcher<T>;
  QObject::connect(watcher, &QFutureWatcherBase::finished, watcher,
                   &QObject::deleteLater);
  // Connected before the future is handed over: an already finished one
  // reports as soon as it is set.
  QMetaObject::Connection connection = QObject::connect(
      watcher, &QFutureWatcherBase::finished, receiver,
      [receiver, slot, future, bound...]() {
        if constexpr (std::is_void_v<T>) {
          (receiver->*slot)(bound...);
        } else {
          (receiver->*slot)(future.result(), bound...);
        }
      },
      Qt::SingleShotConnection);
  watcher->setFuture(future);
  return connection;
}

template <typename T, typename Func, typename... BoundArgs>
  requires _detail::TakesFutureResult<T, Func, BoundArgs...>::value
QMetaObject::Connection NewClosure(QFuture<T> future, QObject* context,
                                   Func func, BoundArgs... bound) {
  QFutureWatcher<T>* watcher = new QFutureWatcher<T>;
  QObject::connect(watcher, &QFutureWatcherBase::finished, watcher,
                   &QObject::deleteLater);
  QMetaObject::Connection connection = QObject::connect(
      watcher, &QFutureWatcherBase::finished, context,
      [func, future, bound...]() {
        if constexpr (std::is_void_v<T>) {
          func(bound...);
        } else {
          func(future.result(), bound...);
        }
      },
      Qt::SingleShotConnection);
  watcher->setFuture(future);
  return connection;
}

// Calls a receiver's method, or any callable, on |context|'s thread, with the
// bound arguments - for handing work to an object that lives on another
// thread, from whichever thread the caller is on:
//
//   PostTo(registry_, &RendererRegistry::SetServing, serving);
//   PostTo(manager, [manager, id]() { manager->SetActivePlaylist(id); });
//
// PostTo always queues: the call runs once the context's thread gets back to
// its event loop, even when that is the caller's own thread, so it never runs
// inside the caller. CallOn runs straight away when the caller is already on
// the context's thread, and queues otherwise.
//
// Either way the call is dropped if |context| is destroyed before it runs, so
// pass the object whose lifetime the callable depends on. |context| itself
// has to be alive when posting. Bound arguments are copied when posting, so
// a raw pointer among them has to outlive the call.

namespace _detail {

template <typename Receiver, typename Slot, typename... BoundArgs>
void InvokeOn(Receiver* receiver, Slot slot, Qt::ConnectionType type,
              BoundArgs... bound) {
  static_assert(_detail::ReceiverAccepts<Slot, Receiver, BoundArgs...>,
                "The receiver must take the bound arguments.");
  QMetaObject::invokeMethod(
      receiver, [receiver, slot, bound...]() { (receiver->*slot)(bound...); },
      type);
}

template <typename Func, typename... BoundArgs>
void InvokeCallableOn(QObject* context, Func func, Qt::ConnectionType type,
                      BoundArgs... bound) {
  static_assert(_detail::CallableAccepts<Func, BoundArgs...>,
                "The callable must take the bound arguments.");
  QMetaObject::invokeMethod(
      context, [func, bound...]() mutable { func(bound...); }, type);
}

}  // namespace _detail

template <typename Receiver, typename Slot, typename... BoundArgs>
  requires std::is_member_function_pointer_v<Slot>
void PostTo(Receiver* receiver, Slot slot, BoundArgs... bound) {
  _detail::InvokeOn(receiver, slot, Qt::QueuedConnection, bound...);
}

template <typename Func, typename... BoundArgs>
  requires(!std::is_member_function_pointer_v<std::decay_t<Func>>)
void PostTo(QObject* context, Func func, BoundArgs... bound) {
  _detail::InvokeCallableOn(context, func, Qt::QueuedConnection, bound...);
}

template <typename Receiver, typename Slot, typename... BoundArgs>
  requires std::is_member_function_pointer_v<Slot>
void CallOn(Receiver* receiver, Slot slot, BoundArgs... bound) {
  _detail::InvokeOn(receiver, slot, Qt::AutoConnection, bound...);
}

template <typename Func, typename... BoundArgs>
  requires(!std::is_member_function_pointer_v<std::decay_t<Func>>)
void CallOn(QObject* context, Func func, BoundArgs... bound) {
  _detail::InvokeCallableOn(context, func, Qt::AutoConnection, bound...);
}

namespace _detail {
// Somewhere between one and two minutes, so that everything that wants to
// run "in a minute or so" doesn't wake up at once.
std::chrono::milliseconds AMinuteOrSo();
}  // namespace _detail

// Calls a receiver's method, or any callable, after the given delay.
template <typename Receiver, typename SlotOrCallable>
void DoAfter(Receiver* receiver, SlotOrCallable slot,
             std::chrono::milliseconds msec) {
  QTimer::singleShot(msec, receiver, slot);
}

template <typename Receiver, typename SlotOrCallable>
void DoInAMinuteOrSo(Receiver* receiver, SlotOrCallable slot) {
  QTimer::singleShot(_detail::AMinuteOrSo(), receiver, slot);
}

#endif  // CLOSURE_H
