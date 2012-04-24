/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef CONCURRENTRUN_H
#define CONCURRENTRUN_H

#include <tr1/functional>

#include <QFuture>
#include <QRunnable>
#include <QThreadPool>

/*
  The aim of ThreadFunctor classes and ConcurrentRun::Run() functions is to
  complete QtConcurrentRun, which lack support for using a particular
  QThreadPool, as it always uses QThreadPool::globalInstance().

  This is problematic when we do not want to share the same thread pool over
  all the application, but want to keep the convenient QtConcurrent::run()
  functor syntax.
  With ConcurrentRun::Run(), time critical changes can be performed in their
  own pool, which is not empty by other actions (as it happens when using
  QtConcurrentRun::run()).

  ThreadFunctor classes are used to store a functor and its arguments, and
  Run() functions are used for convenience: to directly create a new
  ThreadFunctor object and start it.

  Currently, only functions with one or two arguments and a return value are
  supported, but other might be added easily for X arguments by defining a new
  ThreadFunctorBasX class, and add new Run() function.
*/

template<typename ReturnType>
class ThreadFunctorBase : public QFutureInterface<ReturnType>, public QRunnable {
 public:
  ThreadFunctorBase() {}

  QFuture<ReturnType> Start(QThreadPool* thread_pool) {
    this->setRunnable(this);
    this->reportStarted();
    Q_ASSERT(thread_pool);
    QFuture<ReturnType> future = this->future();
    thread_pool->start(this, 0 /* priority: currently we do not support
                                  changing the priority. Might be added later
                                  if needed */);
    return future;
  }

  virtual void run() = 0;

  void End() {
    this->reportResult(result_);
    this->reportFinished();
  }

 protected:
  ReturnType result_;
};

template<typename ReturnType, typename Arg>
class ThreadFunctor1 : public ThreadFunctorBase<ReturnType> {
public:
  ThreadFunctor1(std::tr1::function<ReturnType (Arg)> function,
                const Arg& arg)
    : function_(function),
      arg_(arg)
  { }

  void run() {
    this->result_ = function_(arg_);
    ThreadFunctorBase<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType (Arg)> function_;
  Arg arg_;
};

template<typename ReturnType, typename Arg1, typename Arg2>
class ThreadFunctor2 : public ThreadFunctorBase<ReturnType> {
public:
  ThreadFunctor2(std::tr1::function<ReturnType (Arg1, Arg2)> function,
                const Arg1& arg1, const Arg2& arg2)
    : function_(function),
      arg1_(arg1),
      arg2_(arg2)
  { }

  void run() {
    this->result_ = function_(arg1_, arg2_);
    ThreadFunctorBase<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType (Arg1, Arg2)> function_;
  Arg1 arg1_;
  Arg2 arg2_;
};

namespace ConcurrentRun {

  template<typename ReturnType, typename Arg>
  QFuture<ReturnType> Run(
      QThreadPool* threadpool,
      std::tr1::function<ReturnType (Arg)> function,
      const Arg& arg) {

    return (new ThreadFunctor1<ReturnType, Arg>(function, arg))->Start(threadpool);
  }

  template<typename ReturnType, typename Arg1, typename Arg2>
  QFuture<ReturnType> Run(
      QThreadPool* threadpool,
      std::tr1::function<ReturnType (Arg1, Arg2)> function,
      const Arg1& arg1, const Arg2& arg2) {

    return (new ThreadFunctor2<ReturnType, Arg1, Arg2>(function, arg1, arg2))->Start(threadpool);
  }
}

#endif // CONCURRENTRUN_H
