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

#include <functional>

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
*/


/*
  Base abstract classes ThreadFunctorBase and ThreadFunctor (for void and
  non-void result):
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
};

template <typename ReturnType, typename... Args>
class ThreadFunctor : public ThreadFunctorBase<ReturnType> {
 public:
  ThreadFunctor(std::function<ReturnType (Args...)> function,
                Args... args)
      : function_(std::bind(function, args...)) {
  }

  virtual void run() {
    this->reportResult(function_());
    this->reportFinished();
  }

 private:
  std::function<ReturnType()> function_;
};

// Partial specialisation for void return type.
template <typename... Args>
class ThreadFunctor <void, Args...> : public ThreadFunctorBase<void> {
 public:
  ThreadFunctor(std::function<void (Args...)> function,
                Args... args)
      : function_(std::bind(function, args...)) {
  }

  virtual void run() {
    function_();
    this->reportFinished();
  }

 private:
  std::function<void()> function_;
};


/*
  Run functions
*/
namespace ConcurrentRun {

  // Empty argument form.
  template <typename ReturnType>
  QFuture<ReturnType> Run(
      QThreadPool* threadpool,
      std::function<ReturnType ()> function) {
    return (new ThreadFunctor<ReturnType>(function))->Start(threadpool);
  }

  // Function object with arguments form.
  template <typename ReturnType, typename... Args>
  QFuture<ReturnType> Run(
      QThreadPool* threadpool,
      std::function<ReturnType (Args...)> function,
      const Args&... args) {
    return (new ThreadFunctor<ReturnType, Args...>(
        function, args...))->Start(threadpool);
  }

  // Support passing C function pointers instead of function objects.
  template <typename ReturnType, typename... Args>
  QFuture<ReturnType> Run(
      QThreadPool* threadpool,
      ReturnType (*function) (Args...),
      const Args&... args) {
    return Run(
        threadpool, std::function<ReturnType (Args...)>(function), args...);
  }
}

#endif // CONCURRENTRUN_H
