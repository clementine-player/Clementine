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

#ifndef MESSAGEREPLY_H
#define MESSAGEREPLY_H

#include <QObject>
#include <QSemaphore>

#include "core/logging.h"

// Base QObject for a reply future class that is returned immediately for
// requests that will occur in the background.  Similar to QNetworkReply.
// Use MessageReply instead.
class _MessageReplyBase : public QObject {
  Q_OBJECT

 public:
  _MessageReplyBase(QObject* parent = nullptr);

  virtual int id() const = 0;
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
  bool finished_;
  bool success_;

  QSemaphore semaphore_;
};

// A reply future class that is returned immediately for requests that will
// occur in the background.  Similar to QNetworkReply.
template <typename MessageType>
class MessageReply : public _MessageReplyBase {
 public:
  MessageReply(const MessageType& request_message, QObject* parent = nullptr);

  int id() const { return request_message_.id(); }
  const MessageType& request_message() const { return request_message_; }
  const MessageType& message() const { return reply_message_; }

  void SetReply(const MessageType& message);

 private:
  MessageType request_message_;
  MessageType reply_message_;
};

template <typename MessageType>
MessageReply<MessageType>::MessageReply(const MessageType& request_message,
                                        QObject* parent)
    : _MessageReplyBase(parent) {
  request_message_.MergeFrom(request_message);
}

template <typename MessageType>
void MessageReply<MessageType>::SetReply(const MessageType& message) {
  Q_ASSERT(!finished_);

  reply_message_.MergeFrom(message);
  finished_ = true;
  success_ = true;

  qLog(Debug) << "Releasing ID" << id() << "(finished)";
  semaphore_.release();
  emit Finished(success_);

}

#endif  // MESSAGEREPLY_H
