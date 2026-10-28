/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef NETWORKREMOTE_STREAMING_EVENTCALLBACKTABLE_H_
#define NETWORKREMOTE_STREAMING_EVENTCALLBACKTABLE_H_

#include <QByteArray>
#include <QHostAddress>
#include <QMap>
#include <QMutex>

class QObject;

// Where renderers send their events, such as UPnP's NOTIFY: by token, the
// only address allowed to send them, and the object that takes them.
// Written on the main thread and read by MediaHttpServer on the network
// remote's thread, which hands each event to its receiver on the receiver's
// own thread.
class EventCallbackTable {
 public:
  // |receiver| has an invokable EventReceived(QByteArray body). It must be
  // removed before it's deleted.
  void Set(const QByteArray& token, const QHostAddress& peer,
           QObject* receiver);
  void Remove(const QByteArray& token);

  // Finds |token|, and the only address allowed to send to it.
  bool Find(const QByteArray& token, QHostAddress* peer) const;
  // Queues |body| for |token|'s receiver. False if it's gone.
  bool Deliver(const QByteArray& token, const QByteArray& body) const;

 private:
  struct Entry {
    QHostAddress peer;
    QObject* receiver;
  };

  mutable QMutex mutex_;
  QMap<QByteArray, Entry> entries_;
};

#endif  // NETWORKREMOTE_STREAMING_EVENTCALLBACKTABLE_H_
