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
#include <functional>
#include <memory>

#include <QFuture>
#include <QFutureWatcher>
#include <QMetaMethod>
#include <QMetaType>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>

namespace _detail {

class ObjectHelper;

// Interface for ObjectHelper to call on signal emission.
class ClosureBase {
 public:
  virtual ~ClosureBase();
  virtual void Invoke() = 0;

  // Tests only.
  ObjectHelper* helper() const;

 protected:
  explicit ClosureBase(ObjectHelper*);
  ObjectHelper* helper_;

 private:
  Q_DISABLE_COPY(ClosureBase)
};

// QObject helper as templated QObjects do not work.
// Connects to the given signal and invokes the closure when called.
// Deletes itself and the Closure after being invoked.
class ObjectHelper : public QObject {
  Q_OBJECT
 public:
  ObjectHelper(QObject* parent, const char* signal, ClosureBase* closure);

 public slots:
  void TearDown();

 private slots:
  void Invoked();

 private:
  QMetaObject::Connection connection_;
  std::unique_ptr<ClosureBase> closure_;
  Q_DISABLE_COPY(ObjectHelper)
};

// Helpers for unpacking a variadic template list.
//
// Not Q_ARG: in Qt6 that macro feeds the newer typed invokeMethod overload
// and returns QMetaMethodArgument, not QGenericArgument. QArgument<T> is
// what Q_ARG used to expand to and is what the legacy positional
// QMetaObject::invokeMethod(..., QGenericArgument, ...) overload below
// still needs.
//
// Also not QT_STRINGIFY(Arg): that stringifies the literal token "Arg"
// (this function's own template parameter name), not whatever type Arg is
// instantiated with - macros expand before templates are instantiated.
//
// Each argument is labelled with the type name |expected_types| says this
// slot parameter was actually registered with (from the receiver's
// QMetaMethod, computed once in Closure::Call below), not the canonical
// name QMetaType reports for Arg. Those differ whenever the slot's declared
// parameter type is a typedef alias, or is spelled with different
// namespace qualification than Arg's canonical spelling (eg. a nested
// typedef, or a type declared inside a namespace that the calling .cpp
// reaches via `using namespace`) - same real type, different text. Qt5's
// QMetaMethod::invoke() tolerated that text mismatch; Qt6 validates it and
// rejects the call ("cannot convert formal parameter"). Since this is still
// the exact same object/pointer either way, using the slot's own expected
// spelling for the validation check is always at least as correct as the
// canonical one - it's what the callee's generated thunk actually expects
// to see. (The |expected_types| list falls short only if the caller bound
// more arguments than the slot actually declares, which indexOfSlot()
// wouldn't have matched in the first place; the canonical name is kept as a
// fallback for that edge case.)

// Base case of no arguments.
void Unpack(QList<QGenericArgument>*, const QList<QByteArray>&, int);

template <typename Arg>
void Unpack(QList<QGenericArgument>* list,
           const QList<QByteArray>& expected_types, int index,
           const Arg& arg) {
  const char* type_name = index < expected_types.size()
                              ? expected_types.at(index).constData()
                              : QMetaType::fromType<Arg>().name();
  list->append(QArgument<Arg>(type_name, arg));
}

template <typename Head, typename... Tail>
void Unpack(QList<QGenericArgument>* list,
           const QList<QByteArray>& expected_types, int index,
           const Head& head, const Tail&... tail) {
  Unpack(list, expected_types, index, head);
  Unpack(list, expected_types, index + 1, tail...);
}

template <typename... Args>
class Closure : public ClosureBase {
 public:
  Closure(QObject* sender, const char* signal, QObject* receiver,
          const char* slot, const Args&... args)
      : ClosureBase(new ObjectHelper(sender, signal, this)),
        // std::bind is the easiest way to store an argument list.
        function_(std::bind(&Closure<Args...>::Call, this, args...)),
        receiver_(receiver) {
    const QMetaObject* meta_receiver = receiver->metaObject();
    QByteArray normalised_slot = QMetaObject::normalizedSignature(slot + 1);
    const int index = meta_receiver->indexOfSlot(normalised_slot.constData());
    Q_ASSERT(index != -1);
    slot_ = meta_receiver->method(index);
    // Use a direct connection to insure that this is called immediately when
    // the sender is destroyed. This will run on the signal thread, so TearDown
    // must be thread safe.
    QObject::connect(receiver_, SIGNAL(destroyed()), helper_, SLOT(TearDown()),
                     Qt::DirectConnection);
  }

  virtual void Invoke() { function_(); }

 private:
  void Call(const Args&... args) {
    // Kept alive for the rest of this function: the QGenericArguments built
    // below hold raw pointers into these QByteArrays' data (see Unpack()).
    const QList<QByteArray> expected_types = slot_.parameterTypes();
    QList<QGenericArgument> arg_list;
    Unpack(&arg_list, expected_types, 0, args...);

    slot_.invoke(receiver_,
                 arg_list.size() > 0 ? arg_list[0] : QGenericArgument(),
                 arg_list.size() > 1 ? arg_list[1] : QGenericArgument(),
                 arg_list.size() > 2 ? arg_list[2] : QGenericArgument(),
                 arg_list.size() > 3 ? arg_list[3] : QGenericArgument(),
                 arg_list.size() > 4 ? arg_list[4] : QGenericArgument(),
                 arg_list.size() > 5 ? arg_list[5] : QGenericArgument(),
                 arg_list.size() > 6 ? arg_list[6] : QGenericArgument(),
                 arg_list.size() > 7 ? arg_list[7] : QGenericArgument(),
                 arg_list.size() > 8 ? arg_list[8] : QGenericArgument(),
                 arg_list.size() > 9 ? arg_list[9] : QGenericArgument());
  }

  std::function<void()> function_;
  QObject* receiver_;
  QMetaMethod slot_;
};

template <typename T, typename... Args>
class SharedClosure : public Closure<Args...> {
 public:
  SharedClosure(QSharedPointer<T> sender, const char* signal, QObject* receiver,
                const char* slot, const Args&... args)
      : Closure<Args...>(sender.data(), signal, receiver, slot, args...),
        data_(sender) {}

 private:
  QSharedPointer<T> data_;
};

class CallbackClosure : public ClosureBase {
 public:
  CallbackClosure(QObject* sender, const char* signal,
                  std::function<void()> callback);

  virtual void Invoke();

 private:
  std::function<void()> callback_;
};

}  // namespace _detail

template <typename... Args>
_detail::ClosureBase* NewClosure(QObject* sender, const char* signal,
                                 QObject* receiver, const char* slot,
                                 const Args&... args) {
  return new _detail::Closure<Args...>(sender, signal, receiver, slot, args...);
}

// QSharedPointer variant
template <typename T, typename... Args>
_detail::ClosureBase* NewClosure(QSharedPointer<T> sender, const char* signal,
                                 QObject* receiver, const char* slot,
                                 const Args&... args) {
  return new _detail::SharedClosure<T, Args...>(sender, signal, receiver, slot,
                                                args...);
}

_detail::ClosureBase* NewClosure(QObject* sender, const char* signal,
                                 std::function<void()> callback);

template <typename... Args>
_detail::ClosureBase* NewClosure(QObject* sender, const char* signal,
                                 std::function<void(Args...)> callback,
                                 const Args&... args) {
  return NewClosure(sender, signal, std::bind(callback, args...));
}

template <typename... Args>
_detail::ClosureBase* NewClosure(QObject* sender, const char* signal,
                                 void (*callback)(Args...),
                                 const Args&... args) {
  return NewClosure(sender, signal, std::bind(callback, args...));
}

template <typename T, typename Unused, typename... Args>
_detail::ClosureBase* NewClosure(QObject* sender, const char* signal,
                                 T* receiver, Unused (T::*callback)(Args...),
                                 const Args&... args) {
  return NewClosure(sender, signal, std::bind(callback, receiver, args...));
}

template <typename T, typename... Args>
_detail::ClosureBase* NewClosure(QFuture<T> future, QObject* receiver,
                                 const char* slot, const Args&... args) {
  QFutureWatcher<T>* watcher = new QFutureWatcher<T>;
  QObject::connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
  watcher->setFuture(future);
  return NewClosure(watcher, SIGNAL(finished()), receiver, slot, args...);
}

template <typename T, typename F, typename... Args>
_detail::ClosureBase* NewClosure(QFuture<T> future, const F& callback,
                                 const Args&... args) {
  QFutureWatcher<T>* watcher = new QFutureWatcher<T>;
  QObject::connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
  watcher->setFuture(future);
  return NewClosure(watcher, SIGNAL(finished()), callback, args...);
}

void DoAfter(QObject* receiver, const char* slot, int msec);
void DoAfter(std::function<void()> callback, std::chrono::milliseconds msec);
void DoInAMinuteOrSo(QObject* receiver, const char* slot);

template <typename R, typename P>
void DoAfter(std::function<void()> callback,
             std::chrono::duration<R, P> duration) {
  QTimer* timer = new QTimer;
  timer->setSingleShot(true);
  NewClosure(timer, SIGNAL(timeout()), callback);
  QObject::connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));
  std::chrono::milliseconds msec =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  timer->start(msec.count());
}

#endif  // CLOSURE_H
