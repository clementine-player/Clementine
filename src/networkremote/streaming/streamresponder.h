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

#ifndef NETWORKREMOTE_STREAMING_STREAMRESPONDER_H_
#define NETWORKREMOTE_STREAMING_STREAMRESPONDER_H_

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>

class QTcpSocket;
class QTimer;

// What DirectResponder and PipelineResponder have in common: they own one
// client socket, belong to the MediaHttpServer (so they go when it does),
// delete themselves when the socket closes, and give up on a client that
// stops reading.
class StreamResponder : public QObject {
  Q_OBJECT

 public:
  enum Kind { Direct, Pipeline };

  // A client that hasn't taken any data for this long while some is waiting
  // is disconnected.
  static const int kIdleTimeoutMsec;

  // Takes ownership of |socket|.
  StreamResponder(QTcpSocket* socket, const QByteArray& token, Kind kind,
                  QObject* parent);
  ~StreamResponder();

  const QByteArray& token() const { return token_; }
  Kind kind() const { return kind_; }
  // False once the client has gone and this is about to be deleted, so it no
  // longer counts against the limits.
  bool active() const { return active_; }

 protected:
  QTcpSocket* socket_;

 private slots:
  void Disconnected();
  void Progress();
  void CheckIdle();

 private:
  QByteArray token_;
  Kind kind_;
  bool active_;
  QElapsedTimer last_progress_;
  QTimer* idle_timer_;
};

#endif  // NETWORKREMOTE_STREAMING_STREAMRESPONDER_H_
