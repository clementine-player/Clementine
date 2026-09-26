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

#ifndef NETWORKREMOTE_STREAMING_DIRECTRESPONDER_H_
#define NETWORKREMOTE_STREAMING_DIRECTRESPONDER_H_

#include <QFile>
#include <QObject>

#include "streamitemtable.h"

class QTcpSocket;

// Sends a local file byte for byte, honouring a single Range. Deletes itself
// when the socket closes.
class DirectResponder : public QObject {
  Q_OBJECT

 public:
  // Takes ownership of |socket|.
  DirectResponder(QTcpSocket* socket, const StreamItem& item,
                  const QByteArray& range, bool head_only);
  ~DirectResponder();

 private slots:
  void Pump();

 private:
  QTcpSocket* socket_;
  QFile file_;
  qint64 remaining_;
};

#endif  // NETWORKREMOTE_STREAMING_DIRECTRESPONDER_H_
