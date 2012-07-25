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

#include <boost/type_traits.hpp>
#include <boost/utility.hpp>

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

  Currently, only functions with zero, one, two or three arguments are
  supported, but other might be added easily for X arguments by defining a new
  ThreadFunctorX class (and eventually a second class for handling functions
  which return void: see existing classes for examples), and add new Run()
  function.
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

// Base implemenation for functions having a result to be returned
template<typename ReturnType, class Enabled = void>
class ThreadFunctor : public ThreadFunctorBase<ReturnType> {
 public:
  ThreadFunctor() {}

  virtual void run() = 0;

  void End() {
    this->reportResult(result_);
    this->reportFinished();
  }

 protected:
  ReturnType result_;
};

// Base implementation for functions with void result
template<typename ReturnType>
class ThreadFunctor<ReturnType, typename boost::enable_if<boost::is_void<ReturnType> >::type>
  : public ThreadFunctorBase<ReturnType> {
 public:
  ThreadFunctor() {}

  virtual void run() = 0;

  void End() {
    this->reportFinished();
  }
};

/*
  ThreadFunctor with no arguments:
*/
// Non-void result
template<typename ReturnType, class Enabled = void>
class ThreadFunctor0 : public ThreadFunctor<ReturnType> {
public:
  ThreadFunctor0(std::tr1::function<ReturnType ()> function)
    : function_(function)
  { }

  void run() {
    this->result_ = function_();
    ThreadFunctor<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType ()> function_;
};

// Void result
template<typename ReturnType>
class ThreadFunctor0<ReturnType, typename boost::enable_if<boost::is_void<ReturnType> >::type>
  : public ThreadFunctor<ReturnType> {
public:
  ThreadFunctor0(std::tr1::function<ReturnType ()> function)
    : function_(function)
  { }

  void run() {
    function_();
    ThreadFunctor<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType ()> function_;
};

/*
  ThreadFunctor with one argument:
*/
// Non-void result
template<typename ReturnType, typename Arg, class Enabled = void>
class ThreadFunctor1 : public ThreadFunctor<ReturnType> {
public:
  ThreadFunctor1(std::tr1::function<ReturnType (Arg)> function,
                const Arg& arg)
    : function_(function),
      arg_(arg)
  { }

  void run() {
    this->result_ = function_(arg_);
    ThreadFunctor<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType (Arg)> function_;
  Arg arg_;
};

// Void result
template<typename ReturnType, typename Arg>
class ThreadFunctor1<ReturnType, Arg, typename boost::enable_if<boost::is_void<ReturnType> >::type>
  : public ThreadFunctor<ReturnType> {
public:
  ThreadFunctor1(std::tr1::function<ReturnType (Arg)> function,
                 const Arg& arg)
    : function_(function),
      arg_(arg)
  { }

  void run() {
    function_(arg_);
    ThreadFunctor<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType (Arg)> function_;
  Arg arg_;
};

/*
  ThreadFunctor with two arguments:
*/
// Non-void result
template<typename ReturnType, typename Arg1, typename Arg2, class Enabled = void>
class ThreadFunctor2 : public ThreadFunctor<ReturnType> {
public:
  ThreadFunctor2(std::tr1::function<ReturnType (Arg1, Arg2)> function,
                const Arg1& arg1, const Arg2& arg2)
    : function_(function),
      arg1_(arg1),
      arg2_(arg2)
  { }

  void run() {
    this->result_ = function_(arg1_, arg2_);
    ThreadFunctor<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType (Arg1, Arg2)> function_;
  Arg1 arg1_;
  Arg2 arg2_;
};

// Void result
template<typename ReturnType, typename Arg1, typename Arg2>
class ThreadFunctor2<ReturnType, Arg1, Arg2, typename boost::enable_if<boost::is_void<ReturnType> >::type>
  : public ThreadFunctor<ReturnType> {
public:
  ThreadFunctor2(std::tr1::function<ReturnType (Arg1, Arg2)> function,
                const Arg1& arg1, const Arg2& arg2)
    : function_(function),
      arg1_(arg1),
      arg2_(arg2)
  { }

  void run() {
    function_(arg1_, arg2_);
    ThreadFunctor<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType (Arg1, Arg2)> function_;
  Arg1 arg1_;
  Arg2 arg2_;
};

/*
  ThreadFunctor with three arguments:
*/
// Non-void result
template<typename ReturnType, typename Arg1, typename Arg2, typename Arg3, class Enabled = void>
class ThreadFunctor3 : public ThreadFunctor<ReturnType> {
public:
  ThreadFunctor3(std::tr1::function<ReturnType (Arg1, Arg2, Arg3)> function,
                const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
    : function_(function),
      arg1_(arg1),
      arg2_(arg2),
      arg3_(arg3)
  { }

  void run() {
    this->result_ = function_(arg1_, arg2_, arg3_);
    ThreadFunctor<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType (Arg1, Arg2, Arg3)> function_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
};

// Void result
template<typename ReturnType, typename Arg1, typename Arg2, typename Arg3>
class ThreadFunctor3<ReturnType, Arg1, Arg2, Arg3, typename boost::enable_if<boost::is_void<ReturnType> >::type>
  : public ThreadFunctor<ReturnType> {
public:
  ThreadFunctor3(std::tr1::function<ReturnType (Arg1, Arg2, Arg3)> function,
                const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
    : function_(function),
      arg1_(arg1),
      arg2_(arg2),
      arg3_(arg3)
  { }

  void run() {
    function_(arg1_, arg2_, arg3_);
    ThreadFunctor<ReturnType>::End();
  }

private:
  std::tr1::function<ReturnType (Arg1, Arg2, Arg3)> function_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
};


/*
  Run functions
*/
namespace ConcurrentRun {

  template<typename ReturnType>
  QFuture<ReturnType> Run(
      QThreadPool* threadpool,
      std::tr1::function<ReturnType ()> function) {

    return (new ThreadFunctor0<ReturnType>(function))->Start(threadpool);
  }

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

  template<typename ReturnType, typename Arg1, typename Arg2, typename Arg3>
  QFuture<ReturnType> Run(
      QThreadPool* threadpool,
      std::tr1::function<ReturnType (Arg1, Arg2, Arg3)> function,
      const Arg1& arg1, const Arg2& arg2, const Arg3& arg3) {

    return (new ThreadFunctor3<ReturnType, Arg1, Arg2, Arg3>(function, arg1, arg2, arg3))->Start(threadpool);
  }
}

#endif // CONCURRENTRUN_H
