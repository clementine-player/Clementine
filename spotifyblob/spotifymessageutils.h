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
