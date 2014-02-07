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

#include "core/logging.h"
#include "core/messagereply.h"

class QAbstractSocket;
class QIODevice;
class QLocalSocket;

#define QStringFromStdString(x) QString::fromUtf8(x.data(), x.size())
#define DataCommaSizeFromQString(x) x.toUtf8().constData(), x.toUtf8().length()

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

  // After this is true, messages cannot be sent to the handler any more.
  bool is_device_closed() const { return is_device_closed_; }

 protected slots:
  void WriteMessage(const QByteArray& data);
  void DeviceReadyRead();
  virtual void DeviceClosed();

 protected:
  virtual bool RawMessageArrived(const QByteArray& data) = 0;
  virtual void AbortAll() = 0;

 protected:
  typedef bool (QAbstractSocket::*FlushAbstractSocket)();
  typedef bool (QLocalSocket::*FlushLocalSocket)();

  QIODevice* device_;
  FlushAbstractSocket flush_abstract_socket_;
  FlushLocalSocket flush_local_socket_;

  bool reading_protobuf_;
  quint32 expected_length_;
  QBuffer buffer_;

  bool is_device_closed_;
};

// Reads and writes uint32 length encoded MessageType messages to a socket.
// You should subclass this and implement the MessageArrived(MessageType)
// method.
template <typename MT>
class AbstractMessageHandler : public _MessageHandlerBase {
 public:
  AbstractMessageHandler(QIODevice* device, QObject* parent);
  ~AbstractMessageHandler() { AbortAll(); }

  typedef MT MessageType;
  typedef MessageReply<MT> ReplyType;

  // Serialises the message and writes it to the socket.  This version MUST be
  // called from the thread in which the AbstractMessageHandler was created.
  void SendMessage(const MessageType& message);

  // Serialises the message and writes it to the socket.  This version may be
  // called from any thread.
  void SendMessageAsync(const MessageType& message);

  // Sends the request message inside and takes ownership of the MessageReply.
  // The MessageReply's Finished() signal will be emitted when a reply arrives
  // with the same ID.  Must be called from my thread.
  void SendRequest(ReplyType* reply);

  // Sets the "id" field of reply to the same as the request, and sends the
  // reply on the socket.  Used on the worker side.
  void SendReply(const MessageType& request, MessageType* reply);

 protected:
  // Called when a message is received from the socket.
  virtual void MessageArrived(const MessageType& message) {}

  // _MessageHandlerBase
  bool RawMessageArrived(const QByteArray& data);
  void AbortAll();

 private:
  QMap<int, ReplyType*> pending_replies_;
};

template <typename MT>
AbstractMessageHandler<MT>::AbstractMessageHandler(QIODevice* device,
                                                   QObject* parent)
    : _MessageHandlerBase(device, parent) {}

template <typename MT>
void AbstractMessageHandler<MT>::SendMessage(const MessageType& message) {
  Q_ASSERT(QThread::currentThread() == thread());

  std::string data = message.SerializeAsString();
  WriteMessage(QByteArray(data.data(), data.size()));
}

template <typename MT>
void AbstractMessageHandler<MT>::SendMessageAsync(const MessageType& message) {
  std::string data = message.SerializeAsString();
  metaObject()->invokeMethod(
      this, "WriteMessage", Qt::QueuedConnection,
      Q_ARG(QByteArray, QByteArray(data.data(), data.size())));
}

template <typename MT>
void AbstractMessageHandler<MT>::SendRequest(ReplyType* reply) {
  pending_replies_[reply->id()] = reply;
  SendMessage(reply->request_message());
}

template <typename MT>
void AbstractMessageHandler<MT>::SendReply(const MessageType& request,
                                           MessageType* reply) {
  reply->set_id(request.id());
  SendMessage(*reply);
}

template <typename MT>
bool AbstractMessageHandler<MT>::RawMessageArrived(const QByteArray& data) {
  MessageType message;
  if (!message.ParseFromArray(data.constData(), data.size())) {
    return false;
  }

  ReplyType* reply = pending_replies_.take(message.id());

  if (reply) {
    // This is a reply to a message that we created earlier.
    reply->SetReply(message);
  } else {
    MessageArrived(message);
  }

  return true;
}

template <typename MT>
void AbstractMessageHandler<MT>::AbortAll() {
  foreach(ReplyType * reply, pending_replies_) { reply->Abort(); }
  pending_replies_.clear();
}

#endif  // MESSAGEHANDLER_H
