#ifndef SPOTIFYMESSAGEUTILS_H
#define SPOTIFYMESSAGEUTILS_H

namespace protobuf {
  class SpotifyMessage;
}

class QIODevice;

#define QStringFromStdString(x) \
  QString::fromUtf8(x.data(), x.size())
#define DataCommaSizeFromQString(x) \
  x.toUtf8().constData(), x.toUtf8().length()


class SpotifyMessageUtils {
public:
  virtual ~SpotifyMessageUtils() {}

protected:
  SpotifyMessageUtils() {}

  bool ReadMessage(QIODevice* device, protobuf::SpotifyMessage* message);
  void SendMessage(QIODevice* device, const protobuf::SpotifyMessage& message);
};

#endif // SPOTIFYMESSAGEUTILS_H
