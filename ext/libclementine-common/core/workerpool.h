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

#ifndef WORKERPOOL_H
#define WORKERPOOL_H

#include <QAtomicInt>
#include <QCoreApplication>
#include <QFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QThread>

#include "core/closure.h"
#include "core/logging.h"

// Base class containing signals and slots - required because moc doesn't do
// templated objects.
class _WorkerPoolBase : public QObject {
  Q_OBJECT

 public:
  _WorkerPoolBase(QObject* parent = nullptr);

signals:
  // Emitted when a worker failed to start.  This usually happens when the
  // worker wasn't found, or couldn't be executed.
  void WorkerFailedToStart();

 protected slots:
  virtual void DoStart() {}
  virtual void NewConnection() {}
  virtual void ProcessError(QProcess::ProcessError) {}
  virtual void SendQueuedMessages() {}
};

// Manages a pool of one or more external processes.  A local socket server is
// started for each process, and the address is passed to the process as
// argv[1].  The process is expected to connect back to the socket server, and
// when it does a HandlerType is created for it.
// Instances of HandlerType are created in the WorkerPool's thread.
template <typename HandlerType>
class WorkerPool : public _WorkerPoolBase {
 public:
  WorkerPool(QObject* parent = nullptr);
  ~WorkerPool();

  typedef typename HandlerType::MessageType MessageType;
  typedef typename HandlerType::ReplyType ReplyType;

  // Sets the name of the worker executable.  This is looked for first in the
  // current directory, and then in $PATH.  You must call this before calling
  // Start().
  void SetExecutableName(const QString& executable_name);

  // Sets the number of worker process to use.  Defaults to
  // 1 <= (processors / 2) <= 2.
  void SetWorkerCount(int count);

  // Sets the prefix to use for the local server (on unix this is a named pipe
  // in /tmp).  Defaults to QApplication::applicationName().  A random number
  // is appended to this name when creating each server.
  void SetLocalServerName(const QString& local_server_name);

  // Starts all workers.
  void Start();

  // Fills in the message's "id" field and creates a reply future.  The message
  // is queued and the WorkerPool's thread will send it to the next available
  // worker.  Can be called from any thread.
  ReplyType* SendMessageWithReply(MessageType* message);

 protected:
  // These are all reimplemented slots, they are called on the WorkerPool's
  // thread.
  void DoStart();
  void NewConnection();
  void ProcessError(QProcess::ProcessError error);
  void SendQueuedMessages();

 private:
  struct Worker {
    Worker()
        : local_server_(NULL),
          local_socket_(NULL),
          process_(NULL),
          handler_(NULL) {}

    QLocalServer* local_server_;
    QLocalSocket* local_socket_;
    QProcess* process_;
    HandlerType* handler_;
  };

  // Must only ever be called on my thread.
  void StartOneWorker(Worker* worker);

  template <typename T>
  Worker* FindWorker(T Worker::*member, T value) {
    for (typename QList<Worker>::iterator it = workers_.begin();
         it != workers_.end(); ++it) {
      if ((*it).*member == value) {
        return &(*it);
      }
    }
    return NULL;
  }

  template <typename T>
  void DeleteQObjectPointerLater(T** p) {
    if (*p) {
      (*p)->deleteLater();
      *p = NULL;
    }
  }

  // Creates a new reply future for the request with the next sequential ID,
  // and sets the request's ID to the ID of the reply.  Can be called from any
  // thread
  ReplyType* NewReply(MessageType* message);

  // Returns the next handler, or NULL if there isn't one.  Must be called from
  // my thread.
  HandlerType* NextHandler() const;

 private:
  QString local_server_name_;
  QString executable_name_;
  QString executable_path_;

  int worker_count_;
  mutable int next_worker_;
  QList<Worker> workers_;

  QAtomicInt next_id_;

  QMutex message_queue_mutex_;
  QQueue<ReplyType*> message_queue_;
};

template <typename HandlerType>
WorkerPool<HandlerType>::WorkerPool(QObject* parent)
    : _WorkerPoolBase(parent), next_worker_(0), next_id_(0) {
  worker_count_ = qBound(1, QThread::idealThreadCount() / 2, 2);
  local_server_name_ = qApp->applicationName().toLower();

  if (local_server_name_.isEmpty()) local_server_name_ = "workerpool";
}

template <typename HandlerType>
WorkerPool<HandlerType>::~WorkerPool() {
  for (const Worker& worker : workers_) {
    if (worker.local_socket_ && worker.process_) {
      disconnect(worker.process_, SIGNAL(error(QProcess::ProcessError)), this,
                 SLOT(ProcessError(QProcess::ProcessError)));

      // The worker is connected.  Close his socket and wait for him to exit.
      qLog(Debug) << "Closing worker socket";
      worker.local_socket_->close();
      worker.process_->waitForFinished(500);
    }

    if (worker.process_ && worker.process_->state() == QProcess::Running) {
      // The worker is still running - kill it.
      qLog(Debug) << "Killing worker process";
      worker.process_->terminate();
      if (!worker.process_->waitForFinished(500)) {
        worker.process_->kill();
      }
    }
  }

  for (ReplyType* reply : message_queue_) {
    reply->Abort();
  }
}

template <typename HandlerType>
void WorkerPool<HandlerType>::SetWorkerCount(int count) {
  Q_ASSERT(workers_.isEmpty());
  worker_count_ = count;
}

template <typename HandlerType>
void WorkerPool<HandlerType>::SetLocalServerName(
    const QString& local_server_name) {
  Q_ASSERT(workers_.isEmpty());
  local_server_name_ = local_server_name;
}

template <typename HandlerType>
void WorkerPool<HandlerType>::SetExecutableName(
    const QString& executable_name) {
  Q_ASSERT(workers_.isEmpty());
  executable_name_ = executable_name;
}

template <typename HandlerType>
void WorkerPool<HandlerType>::Start() {
  metaObject()->invokeMethod(this, "DoStart");
}

template <typename HandlerType>
void WorkerPool<HandlerType>::DoStart() {
  Q_ASSERT(workers_.isEmpty());
  Q_ASSERT(!executable_name_.isEmpty());
  Q_ASSERT(QThread::currentThread() == thread());

  // Find the executable if we can, default to searching $PATH
  executable_path_ = executable_name_;

  QStringList search_path;
  search_path << qApp->applicationDirPath();
#if defined(Q_OS_MAC) && defined(USE_BUNDLE)
  search_path << qApp->applicationDirPath() + "/" + USE_BUNDLE_DIR;
#endif

  for (const QString& path_prefix : search_path) {
    const QString executable_path = path_prefix + "/" + executable_name_;
    if (QFile::exists(executable_path)) {
      executable_path_ = executable_path;
      break;
    }
  }

  // Start all the workers
  for (int i = 0; i < worker_count_; ++i) {
    Worker worker;
    StartOneWorker(&worker);

    workers_ << worker;
  }
}

template <typename HandlerType>
void WorkerPool<HandlerType>::StartOneWorker(Worker* worker) {
  Q_ASSERT(QThread::currentThread() == thread());

  DeleteQObjectPointerLater(&worker->local_server_);
  DeleteQObjectPointerLater(&worker->local_socket_);
  DeleteQObjectPointerLater(&worker->process_);
  DeleteQObjectPointerLater(&worker->handler_);

  worker->local_server_ = new QLocalServer(this);
  worker->process_ = new QProcess(this);

  connect(worker->local_server_, SIGNAL(newConnection()),
          SLOT(NewConnection()));
  connect(worker->process_, SIGNAL(error(QProcess::ProcessError)),
          SLOT(ProcessError(QProcess::ProcessError)));

  // Create a server, find an unused name and start listening
  forever {
    const int unique_number = qrand() ^ ((int)(quint64(this) & 0xFFFFFFFF));
    const QString name =
        QString("%1_%2").arg(local_server_name_).arg(unique_number);

    if (worker->local_server_->listen(name)) {
      break;
    }
  }

  qLog(Debug) << "Starting worker" << worker << executable_path_
              << worker->local_server_->fullServerName();

  // Start the process
  worker->process_->setProcessChannelMode(QProcess::ForwardedChannels);
  worker->process_->start(executable_path_,
                          QStringList()
                              << worker->local_server_->fullServerName());
}

template <typename HandlerType>
void WorkerPool<HandlerType>::NewConnection() {
  Q_ASSERT(QThread::currentThread() == thread());

  QLocalServer* server = qobject_cast<QLocalServer*>(sender());

  // Find the worker with this server.
  Worker* worker = FindWorker(&Worker::local_server_, server);
  if (!worker) return;

  qLog(Debug) << "Worker" << worker << "connected to"
              << server->fullServerName();

  // Accept the connection.
  worker->local_socket_ = server->nextPendingConnection();

  // We only ever accept one connection per worker, so destroy the server now.
  worker->local_socket_->setParent(this);
  worker->local_server_->deleteLater();
  worker->local_server_ = NULL;

  // Create the handler.
  worker->handler_ = new HandlerType(worker->local_socket_, this);

  SendQueuedMessages();
}

template <typename HandlerType>
void WorkerPool<HandlerType>::ProcessError(QProcess::ProcessError error) {
  Q_ASSERT(QThread::currentThread() == thread());

  QProcess* process = qobject_cast<QProcess*>(sender());

  // Find the worker with this process.
  Worker* worker = FindWorker(&Worker::process_, process);
  if (!worker) return;

  switch (error) {
    case QProcess::FailedToStart:
      // Failed to start errors are bad - it usually means the worker isn't
      // installed.  Don't restart the process, but tell our owner, who will
      // probably want to do something fatal.
      qLog(Error) << "Worker failed to start";
      emit WorkerFailedToStart();
      break;

    default:
      // On any other error we just restart the process.
      qLog(Debug) << "Worker" << worker << "failed with error" << error
                  << "- restarting";
      StartOneWorker(worker);
      break;
  }
}

template <typename HandlerType>
typename WorkerPool<HandlerType>::ReplyType* WorkerPool<HandlerType>::NewReply(
    MessageType* message) {
  const int id = next_id_.fetchAndAddOrdered(1);
  message->set_id(id);

  return new ReplyType(*message);
}

template <typename HandlerType>
typename WorkerPool<HandlerType>::ReplyType*
WorkerPool<HandlerType>::SendMessageWithReply(MessageType* message) {
  ReplyType* reply = NewReply(message);

  // Add the pending reply to the queue
  {
    QMutexLocker l(&message_queue_mutex_);
    message_queue_.enqueue(reply);
  }

  // Wake up the main thread
  metaObject()->invokeMethod(this, "SendQueuedMessages", Qt::QueuedConnection);

  return reply;
}

template <typename HandlerType>
void WorkerPool<HandlerType>::SendQueuedMessages() {
  QMutexLocker l(&message_queue_mutex_);

  while (!message_queue_.isEmpty()) {
    ReplyType* reply = message_queue_.dequeue();

    // Find a worker for this message
    HandlerType* handler = NextHandler();
    if (!handler) {
      // No available handlers - put the message on the front of the queue.
      message_queue_.prepend(reply);
      qLog(Debug) << "No available handlers to process request";
      break;
    }

    handler->SendRequest(reply);
  }
}

template <typename HandlerType>
HandlerType* WorkerPool<HandlerType>::NextHandler() const {
  for (int i = 0; i < workers_.count(); ++i) {
    const int worker_index = (next_worker_ + i) % workers_.count();

    if (workers_[worker_index].handler_ &&
        !workers_[worker_index].handler_->is_device_closed()) {
      next_worker_ = (worker_index + 1) % workers_.count();
      return workers_[worker_index].handler_;
    }
  }

  return NULL;
}

#endif  // WORKERPOOL_H
