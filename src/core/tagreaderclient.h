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

#ifndef TAGREADERCLIENT_H
#define TAGREADERCLIENT_H

#include "messagehandler.h"
#include "song.h"
#include "tagreadermessages.pb.h"

class QProcess;

class TagReaderClient : public QObject {
  Q_OBJECT

public:
  TagReaderClient(QObject* parent = 0);

  typedef AbstractMessageHandler<pb::tagreader::Message> HandlerType;
  typedef typename HandlerType::ReplyType ReplyType;

  void Start();

  ReplyType* ReadFile(const QString& filename);
  ReplyType* SaveFile(const QString& filename, const Song& metadata);
  ReplyType* IsMediaFile(const QString& filename);
  ReplyType* LoadEmbeddedArt(const QString& filename);

private:
  QProcess* process_;
  HandlerType* handler_;
};

typedef TagReaderClient::ReplyType TagReaderReply;

#endif // TAGREADERCLIENT_H
