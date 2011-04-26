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
