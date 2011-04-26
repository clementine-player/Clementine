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


#include "spotifymessages.pb.h"
#include "spotifymessageutils.h"
#include "core/logging.h"

#include <boost/scoped_array.hpp>

bool SpotifyMessageUtils::ReadMessage(QIODevice* device,
                                      protobuf::SpotifyMessage* message) {
  QDataStream s(device);

  quint32 length = 0;
  s >> length;

  if (length == 0) {
    qLog(Error) << "Zero length protobuf message body";
    return false;
  }

  boost::scoped_array<char> data(new char[length]);
  s.readRawData(data.get(), length);

  if (!message->ParseFromArray(data.get(), length)) {
    qLog(Error) << "Malformed protobuf message";
    return false;
  }

  qLog(Debug) << message->DebugString().c_str();
  return true;
}

void SpotifyMessageUtils::SendMessage(QIODevice* device,
                                      const protobuf::SpotifyMessage& message) {
  qLog(Debug) << message.DebugString().c_str();

  std::string data(message.SerializeAsString());

  QDataStream s(device);
  s << quint32(data.length());
  s.writeRawData(data.data(), data.length());
}
