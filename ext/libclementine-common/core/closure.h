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

#include <functional>

#include <QMetaMethod>
#include <QObject>
#include <QSharedPointer>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace _detail {

class ObjectHelper;

// Interface for ObjectHelper to call on signal emission.
class ClosureBase : boost::noncopyable {
 public:
  virtual ~ClosureBase();
  virtual void Invoke() = 0;

  // Tests only.
  ObjectHelper* helper() const;

 protected:
  explicit ClosureBase(ObjectHelper*);
  ObjectHelper* helper_;
};

// QObject helper as templated QObjects do not work.
// Connects to the given signal and invokes the closure when called.
// Deletes itself and the Closure after being invoked.
class ObjectHelper : public QObject {
  Q_OBJECT
 public:
  ObjectHelper(
      QObject* parent,
      const char* signal,
      ClosureBase* closure);

 private slots:
  void Invoked();

 private:
  boost::scoped_ptr<ClosureBase> closure_;
  Q_DISABLE_COPY(ObjectHelper);
};

// Helpers for unpacking a variadic template list.

// Base case of no arguments.
void Unpack(QList<QGenericArgument>*);

template <typename Arg>
void Unpack(QList<QGenericArgument>* list, const Arg& arg) {
  list->append(Q_ARG(Arg, arg));
}

template <typename Head, typename... Tail>
void Unpack(QList<QGenericArgument>* list, const Head& head, const Tail&... tail) {
  Unpack(list, head);
  Unpack(list, tail...);
}

template <typename... Args>
class Closure : public ClosureBase {
 public:
  Closure(
      QObject* sender,
      const char* signal,
      QObject* receiver,
      const char* slot,
      const Args&... args)
   :  ClosureBase(new ObjectHelper(sender, signal, this)),
      // boost::bind is the easiest way to store an argument list.
      function_(boost::bind(&Closure<Args...>::Call, this, args...)),
      receiver_(receiver) {
    const QMetaObject* meta_receiver = receiver->metaObject();
    QByteArray normalised_slot = QMetaObject::normalizedSignature(slot + 1);
    const int index = meta_receiver->indexOfSlot(normalised_slot.constData());
    Q_ASSERT(index != -1);
    slot_ = meta_receiver->method(index);
    QObject::connect(receiver_, SIGNAL(destroyed()), helper_, SLOT(deleteLater()));
  }

  virtual void Invoke() {
    function_();
  }

 private:
  void Call(const Args&... args) {
    QList<QGenericArgument> arg_list;
    Unpack(&arg_list, args...);

    slot_.invoke(
        receiver_,
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

  boost::function<void()> function_;
  QObject* receiver_;
  QMetaMethod slot_;
};

template <typename T, typename... Args>
class SharedClosure : public Closure<Args...> {
 public:
  SharedClosure(
      QSharedPointer<T> sender,
      const char* signal,
      QObject* receiver,
      const char* slot,
      const Args&... args)
    : Closure<Args...>(
        sender.data(),
        signal,
        receiver,
        slot,
        args...),
      data_(sender) {
  }

 private:
  QSharedPointer<T> data_;
};

class CallbackClosure : public ClosureBase {
 public:
  CallbackClosure(
      QObject* sender,
      const char* signal,
      boost::function<void()> callback);

  virtual void Invoke();

 private:
  boost::function<void()> callback_;
};

}  // namespace _detail

template <typename... Args>
_detail::ClosureBase* NewClosure(
    QObject* sender,
    const char* signal,
    QObject* receiver,
    const char* slot,
    const Args&... args) {
  return new _detail::Closure<Args...>(
      sender, signal, receiver, slot, args...);
}

// QSharedPointer variant
template <typename T, typename... Args>
_detail::ClosureBase* NewClosure(
    QSharedPointer<T> sender,
    const char* signal,
    QObject* receiver,
    const char* slot,
    const Args&... args) {
  return new _detail::SharedClosure<T, Args...>(
      sender, signal, receiver, slot, args...);
}

_detail::ClosureBase* NewClosure(
    QObject* sender,
    const char* signal,
    boost::function<void()> callback);

template <typename... Args>
_detail::ClosureBase* NewClosure(
    QObject* sender,
    const char* signal,
    boost::function<void(Args...)> callback,
    const Args&... args) {
  return NewClosure(sender, signal, boost::bind(callback, args...));
}

template <typename... Args>
_detail::ClosureBase* NewClosure(
    QObject* sender,
    const char* signal,
    void (*callback)(Args...),
    const Args&... args) {
  return NewClosure(sender, signal, boost::bind(callback, args...));
}

template <typename T, typename Unused, typename... Args>
_detail::ClosureBase* NewClosure(
    QObject* sender,
    const char* signal,
    T* receiver, Unused (T::*callback)(Args...),
    const Args&... args) {
  return NewClosure(sender, signal, boost::bind(callback, receiver, args...));
}


void DoAfter(QObject* receiver, const char* slot, int msec);
void DoInAMinuteOrSo(QObject* receiver, const char* slot);

#endif  // CLOSURE_H
