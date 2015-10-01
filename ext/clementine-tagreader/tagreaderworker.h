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

#ifndef TAGREADERWORKER_H
#define TAGREADERWORKER_H

#include "config.h"
#include "tagreader.h"
#include "tagreadermessages.pb.h"
#include "core/messagehandler.h"

class TagReaderWorker : public AbstractMessageHandler<pb::tagreader::Message> {
 public:
  TagReaderWorker(QIODevice* socket, QObject* parent = NULL);

 protected:
  void MessageArrived(const pb::tagreader::Message& message);
  void DeviceClosed();

 private:
  void ReportNetworkStatistics(
      pb::tagreader::NetworkStatisticsResponse* response) const;

  TagReader tag_reader_;
};

#endif  // TAGREADERWORKER_H
