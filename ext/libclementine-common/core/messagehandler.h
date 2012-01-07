/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// Note: this file is licensed under the Apache License instead of GPL because
// it is used by the Spotify blob which links against libspotify and is not GPL
// compatible.


#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QBuffer>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QSemaphore>
#include <QThread>

class QAbstractSocket;
class QIODevice;
class QLocalSocket;

#define QStringFromStdString(x) \
  QString::fromUtf8(x.data(), x.size())
#define DataCommaSizeFromQString(x) \
  x.toUtf8().constData(), x.toUtf8().length()


// Base QObject for a reply future class that is returned immediately for
// requests that will occur in the background.  Similar to QNetworkReply.
// Use MessageReply instead.
class _MessageReplyBase : public QObject {
  Q_OBJECT

public:
  _MessageReplyBase(int id, QObject* parent = 0);

  int id() const { return id_; }
  bool is_finished() const { return finished_; }
  bool is_successful() const { return success_; }

  // Waits for the reply to finish by waiting on a semaphore.  Never call this
  // from the MessageHandler's thread or it will block forever.
  // Returns true if the call was successful.
  bool WaitForFinished();

  void Abort();

signals:
  void Finished(bool success);

protected:
  int id_;
  bool finished_;
  bool success_;

  QSemaphore semaphore_;
};


// A reply future class that is returned immediately for requests that will
// occur in the background.  Similar to QNetworkReply.
template <typename MessageType>
class MessageReply : public _MessageReplyBase {
public:
  MessageReply(int id, QObject* parent = 0);

  const MessageType& message() const { return message_; }

  void SetReply(const MessageType& message);

private:
  MessageType message_;
};


// Reads and writes uint32 length encoded protobufs to a socket.
// This base QObject is separate from AbstractMessageHandler because moc can't
// handle templated classes.  Use AbstractMessageHandler instead.
class _MessageHandlerBase : public QObject {
  Q_OBJECT

public:
  // device can be NULL, in which case you must call SetDevice before writing
  // any messages.
  _MessageHandlerBase(QIODevice* device, QObject* parent);

  void SetDevice(QIODevice* device);

protected slots:
  void WriteMessage(const QByteArray& data);
  void DeviceReadyRead();
  virtual void SocketClosed() {}

protected:
  virtual bool RawMessageArrived(const QByteArray& data) = 0;

protected:
  typedef bool (QAbstractSocket::*FlushAbstractSocket)();
  typedef bool (QLocalSocket::*FlushLocalSocket)();

  QIODevice* device_;
  FlushAbstractSocket flush_abstract_socket_;
  FlushLocalSocket flush_local_socket_;

  bool reading_protobuf_;
  quint32 expected_length_;
  QBuffer buffer_;
};


// Reads and writes uint32 length encoded MessageType messages to a socket.
// You should subclass this and implement the MessageArrived(MessageType)
// method.
template <typename MessageType>
class AbstractMessageHandler : public _MessageHandlerBase {
public:
  AbstractMessageHandler(QIODevice* device, QObject* parent);

  typedef MessageReply<MessageType> ReplyType;

  // Serialises the message and writes it to the socket.  This version MUST be
  // called from the thread in which the AbstractMessageHandler was created.
  void SendMessage(const MessageType& message);

  // Serialises the message and writes it to the socket.  This version may be
  // called from any thread.
  void SendMessageAsync(const MessageType& message);

  // Creates a new reply future for the request with the next sequential ID,
  // and sets the request's ID to the ID of the reply.  When a reply arrives
  // for this request the reply is triggered automatically and MessageArrived
  // is NOT called.  Can be called from any thread.
  ReplyType* NewReply(MessageType* message);

  // Same as NewReply, except the message is sent as well.  Can be called from
  // any thread.
  ReplyType* SendMessageWithReply(MessageType* message);

  // Sets the "id" field of reply to the same as the request, and sends the
  // reply on the socket.  Used on the worker side.
  void SendReply(const MessageType& request, MessageType* reply);

protected:
  // Called when a message is received from the socket.
  virtual void MessageArrived(const MessageType& message) {}

  // _MessageHandlerBase
  bool RawMessageArrived(const QByteArray& data);
  void SocketClosed();

private:
  QMutex mutex_;
  int next_id_;
  QMap<int, ReplyType*> pending_replies_;
};


template<typename MessageType>
AbstractMessageHandler<MessageType>::AbstractMessageHandler(
    QIODevice* device, QObject* parent)
  : _MessageHandlerBase(device, parent),
    next_id_(1)
{
}

template<typename MessageType>
void AbstractMessageHandler<MessageType>::SendMessage(const MessageType& message) {
  Q_ASSERT(QThread::currentThread() == thread());

  std::string data = message.SerializeAsString();
  WriteMessage(QByteArray(data.data(), data.size()));
}

template<typename MessageType>
void AbstractMessageHandler<MessageType>::SendMessageAsync(const MessageType& message) {
  std::string data = message.SerializeAsString();
  metaObject()->invokeMethod(this, "WriteMessage", Qt::QueuedConnection,
                             Q_ARG(QByteArray, QByteArray(data.data(), data.size())));
}

template<typename MessageType>
void AbstractMessageHandler<MessageType>::SendReply(const MessageType& request,
                                                    MessageType* reply) {
  reply->set_id(request.id());
  SendMessage(*reply);
}

template<typename MessageType>
bool AbstractMessageHandler<MessageType>::RawMessageArrived(const QByteArray& data) {
  MessageType message;
  if (!message.ParseFromArray(data.constData(), data.size())) {
    return false;
  }

  ReplyType* reply = NULL;
  {
    QMutexLocker l(&mutex_);
    reply = pending_replies_.take(message.id());
  }

  if (reply) {
    // This is a reply to a message that we created earlier.
    reply->SetReply(message);
  } else {
    MessageArrived(message);
  }

  return true;
}

template<typename MessageType>
typename AbstractMessageHandler<MessageType>::ReplyType*
AbstractMessageHandler<MessageType>::NewReply(
    MessageType* message) {
  ReplyType* reply = NULL;

  {
    QMutexLocker l(&mutex_);

    const int id = next_id_ ++;
    reply = new ReplyType(id);
    pending_replies_[id] = reply;
  }

  message->set_id(reply->id());
  return reply;
}

template<typename MessageType>
typename AbstractMessageHandler<MessageType>::ReplyType*
AbstractMessageHandler<MessageType>::SendMessageWithReply(
    MessageType* message) {
  ReplyType* reply = NewReply(message);

  SendMessageAsync(*message);

  return reply;
}

template<typename MessageType>
void AbstractMessageHandler<MessageType>::SocketClosed() {
  QMutexLocker l(&mutex_);

  foreach (ReplyType* reply, pending_replies_) {
    reply->Abort();
  }
  pending_replies_.clear();
}

template<typename MessageType>
MessageReply<MessageType>::MessageReply(int id, QObject* parent)
  : _MessageReplyBase(id, parent)
{
}

template<typename MessageType>
void MessageReply<MessageType>::SetReply(const MessageType& message) {
  Q_ASSERT(!finished_);

  message_.MergeFrom(message);
  finished_ = true;
  success_ = true;

  emit Finished(success_);
  semaphore_.release();
}

#endif // MESSAGEHANDLER_H
