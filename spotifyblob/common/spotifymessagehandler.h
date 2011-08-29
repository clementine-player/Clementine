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


#ifndef SPOTIFYMESSAGEHANDLER_H
#define SPOTIFYMESSAGEHANDLER_H

#include <QBuffer>
#include <QObject>

namespace spotify_pb {
  class SpotifyMessage;
}


#define QStringFromStdString(x) \
  QString::fromUtf8(x.data(), x.size())
#define DataCommaSizeFromQString(x) \
  x.toUtf8().constData(), x.toUtf8().length()


class SpotifyMessageHandler : public QObject {
  Q_OBJECT

public:
  SpotifyMessageHandler(QIODevice* device, QObject* parent);

  void SendMessage(const spotify_pb::SpotifyMessage& message);

signals:
  void MessageArrived(const spotify_pb::SpotifyMessage& message);

private slots:
  void DeviceReadyRead();

private:
  QIODevice* device_;

  bool reading_protobuf_;
  quint32 expected_length_;
  QBuffer buffer_;
};

#endif // SPOTIFYMESSAGEHANDLER_H
