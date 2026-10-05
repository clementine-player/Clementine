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

#include "chromecast/castmediaplayer.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkInterface>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <functional>

#include "chromecast/castchannel.h"
#include "gtest/gtest.h"
#include "test_utils.h"

namespace {

const char kTransportId[] = "web-7";

struct Sent {
  QString name_space;
  QString destination;
  QJsonObject payload;
};

// Records what's sent, and delivers what the test says the device sent.
class FakeChannel : public CastChannel {
 public:
  bool is_open() const override { return true; }
  void Send(const QString& name_space, const QString& destination,
            const QJsonObject& payload) override {
    sent_ << Sent{name_space, destination, payload};
  }

  void Receive(const QString& name_space, const QString& source,
               const QJsonObject& payload) {
    emit MessageReceived(name_space, source, payload);
  }

  // The last message of |type|, or an empty one.
  QJsonObject Last(const QString& type) const {
    for (int i = sent_.size() - 1; i >= 0; --i) {
      if (sent_[i].payload["type"].toString() == type) return sent_[i].payload;
    }
    return QJsonObject();
  }

  QList<Sent> sent_;
};

QJsonObject AppStatus(const QString& session_id) {
  return {
      {"type", "RECEIVER_STATUS"},
      {"status",
       QJsonObject{{"applications",
                    QJsonArray{QJsonObject{{"appId", "CC1AD845"},
                                           {"sessionId", session_id},
                                           {"transportId", kTransportId}}}},
                   {"volume", QJsonObject{{"level", 0.4}, {"muted", false}}}}}};
}

QJsonObject NoAppStatus() {
  return {{"type", "RECEIVER_STATUS"},
          {"status", QJsonObject{{"applications", QJsonArray{}}}}};
}

QJsonObject MediaStatus(int request_id, int media_session_id,
                        const QString& state, double time = 0,
                        const QString& idle_reason = QString()) {
  QJsonObject status{{"mediaSessionId", media_session_id},
                     {"playerState", state},
                     {"currentTime", time},
                     {"playbackRate", 1},
                     {"media", QJsonObject{{"duration", 200.5}}}};
  if (!idle_reason.isEmpty()) status["idleReason"] = idle_reason;
  return {{"type", "MEDIA_STATUS"},
          {"requestId", request_id},
          {"status", QJsonArray{status}}};
}

class CastMediaPlayerTest : public ::testing::Test {
 protected:
  CastMediaPlayerTest() : player_(&channel_) {}

  void Launch() {
    QSignalSpy ready(&player_, &CastMediaPlayer::Ready);
    player_.Launch();
    channel_.Receive(CastMediaPlayer::kReceiverNamespace, "receiver-0",
                     AppStatus("session-1"));
    ASSERT_EQ(1, ready.count());
    ASSERT_TRUE(player_.is_ready());
  }

  // Loads a track and has the device start playing it as media session |id|.
  void LoadAndPlay(int id) {
    player_.Load(Info(), 0, true);
    const int request_id = channel_.Last("LOAD")["requestId"].toInt();
    channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                     MediaStatus(request_id, id, "PLAYING"));
    ASSERT_EQ(CastMediaPlayer::Playing, player_.state());
  }

  static CastMediaInfo Info() {
    CastMediaInfo info;
    info.url = QUrl("http://192.168.1.2:5500/s/token/1");
    info.content_type = "audio/flac";
    info.title = "Title";
    info.artist = "Artist";
    info.album = "Album";
    info.image_url = QUrl("http://192.168.1.2:5500/a/token/1");
    info.duration_ms = 200500;
    return info;
  }

  FakeChannel channel_;
  CastMediaPlayer player_;
};

TEST_F(CastMediaPlayerTest, LaunchesTheDefaultMediaReceiver) {
  player_.Launch();
  ASSERT_EQ(1, channel_.sent_.size());
  EXPECT_EQ(CastMediaPlayer::kReceiverNamespace, channel_.sent_[0].name_space);
  EXPECT_EQ("receiver-0", channel_.sent_[0].destination);
  EXPECT_EQ("LAUNCH", channel_.sent_[0].payload["type"].toString());
  EXPECT_EQ("CC1AD845", channel_.sent_[0].payload["appId"].toString());
  EXPECT_FALSE(player_.is_ready());

  // A status from before the app started changes nothing.
  QSignalSpy ready(&player_, &CastMediaPlayer::Ready);
  channel_.Receive(CastMediaPlayer::kReceiverNamespace, "receiver-0",
                   NoAppStatus());
  EXPECT_EQ(0, ready.count());

  QSignalSpy volume(&player_, &CastMediaPlayer::VolumeChanged);
  channel_.Receive(CastMediaPlayer::kReceiverNamespace, "receiver-0",
                   AppStatus("session-1"));
  EXPECT_EQ(1, ready.count());
  EXPECT_TRUE(player_.is_ready());
  EXPECT_EQ(1, volume.count());
  EXPECT_DOUBLE_EQ(0.4, player_.volume());

  // It asks the app what it's playing.
  EXPECT_EQ(kTransportId, channel_.sent_.last().destination);
  EXPECT_EQ("GET_STATUS", channel_.sent_.last().payload["type"].toString());
}

TEST_F(CastMediaPlayerTest, LoadsWithMetadata) {
  Launch();
  player_.Load(Info(), 30000, true);

  const Sent& load = channel_.sent_.last();
  EXPECT_EQ(CastMediaPlayer::kMediaNamespace, load.name_space);
  EXPECT_EQ(kTransportId, load.destination);
  EXPECT_EQ("LOAD", load.payload["type"].toString());
  EXPECT_TRUE(load.payload["autoplay"].toBool());
  EXPECT_DOUBLE_EQ(30.0, load.payload["currentTime"].toDouble());

  const QJsonObject media = load.payload["media"].toObject();
  EXPECT_EQ("http://192.168.1.2:5500/s/token/1", media["contentId"].toString());
  EXPECT_EQ("audio/flac", media["contentType"].toString());
  EXPECT_EQ("BUFFERED", media["streamType"].toString());
  EXPECT_DOUBLE_EQ(200.5, media["duration"].toDouble());

  const QJsonObject metadata = media["metadata"].toObject();
  EXPECT_EQ(3, metadata["metadataType"].toInt());
  EXPECT_EQ("Title", metadata["title"].toString());
  EXPECT_EQ("Artist", metadata["artist"].toString());
  EXPECT_EQ("Album", metadata["albumName"].toString());
  EXPECT_EQ("http://192.168.1.2:5500/a/token/1",
            metadata["images"].toArray()[0].toObject()["url"].toString());

  EXPECT_EQ(CastMediaPlayer::Buffering, player_.state());
  EXPECT_EQ(30000, player_.position_ms());
}

TEST_F(CastMediaPlayerTest, DoesntLoadBeforeLaunching) {
  player_.Load(Info(), 0, true);
  EXPECT_TRUE(channel_.sent_.isEmpty());
  EXPECT_EQ(CastMediaPlayer::Idle, player_.state());
}

TEST_F(CastMediaPlayerTest, CommandsNameTheMediaSession) {
  Launch();
  LoadAndPlay(5);
  EXPECT_EQ(200500, player_.duration_ms());

  player_.Pause();
  EXPECT_EQ("PAUSE", channel_.sent_.last().payload["type"].toString());
  EXPECT_EQ(5, channel_.sent_.last().payload["mediaSessionId"].toInt());

  player_.Seek(42000);
  EXPECT_EQ("SEEK", channel_.sent_.last().payload["type"].toString());
  EXPECT_DOUBLE_EQ(42.0,
                   channel_.sent_.last().payload["currentTime"].toDouble());

  player_.Play();
  EXPECT_EQ("PLAY", channel_.sent_.last().payload["type"].toString());

  // Every request has an id of its own.
  QSet<int> ids;
  for (const Sent& sent : channel_.sent_) {
    ids << sent.payload["requestId"].toInt();
  }
  EXPECT_EQ(channel_.sent_.size(), ids.size());
}

TEST_F(CastMediaPlayerTest, NoCommandsWithNothingLoaded) {
  Launch();
  const int sent = channel_.sent_.size();
  player_.Play();
  player_.Pause();
  player_.Seek(1000);
  EXPECT_EQ(sent, channel_.sent_.size());
}

TEST_F(CastMediaPlayerTest, FollowsTheDevicesState) {
  Launch();
  LoadAndPlay(5);
  QSignalSpy states(&player_, &CastMediaPlayer::StateChanged);

  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(0, 5, "PAUSED", 12.5));
  EXPECT_EQ(CastMediaPlayer::Paused, player_.state());
  EXPECT_EQ(12500, player_.position_ms());

  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(0, 5, "BUFFERING", 12.5));
  EXPECT_EQ(CastMediaPlayer::Buffering, player_.state());
  EXPECT_EQ(2, states.count());
}

TEST_F(CastMediaPlayerTest, IgnoresTheOldTracksStatusAfterALoad) {
  Launch();
  LoadAndPlay(5);
  player_.Load(Info(), 0, true);
  const int request_id = channel_.Last("LOAD")["requestId"].toInt();

  // The old track being interrupted, then the new one buffering.
  QSignalSpy ended(&player_, &CastMediaPlayer::TrackEnded);
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(0, 5, "IDLE", 100, "INTERRUPTED"));
  EXPECT_EQ(CastMediaPlayer::Buffering, player_.state());
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(request_id, 6, "PLAYING"));
  EXPECT_EQ(CastMediaPlayer::Playing, player_.state());
  EXPECT_EQ(0, ended.count());

  player_.Pause();
  EXPECT_EQ(6, channel_.sent_.last().payload["mediaSessionId"].toInt());
}

TEST_F(CastMediaPlayerTest, LoadingIsNotTheEnd) {
  Launch();
  player_.Load(Info(), 0, true);
  const int request_id = channel_.Last("LOAD")["requestId"].toInt();

  // What a Nest Mini answers a LOAD with.
  QJsonObject loading = MediaStatus(request_id, 2, "IDLE");
  QJsonArray statuses = loading["status"].toArray();
  QJsonObject status = statuses[0].toObject();
  status["extendedStatus"] = QJsonObject{{"playerState", "LOADING"}};
  statuses[0] = status;
  loading["status"] = statuses;
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId, loading);
  EXPECT_EQ(CastMediaPlayer::Buffering, player_.state());

  // Nor is IDLE without a reason.
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(request_id, 2, "IDLE"));
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(0, 2, "PLAYING"));
  EXPECT_EQ(CastMediaPlayer::Playing, player_.state());
  player_.Pause();
  EXPECT_EQ(2, channel_.sent_.last().payload["mediaSessionId"].toInt());
}

TEST_F(CastMediaPlayerTest, ReportsTheEndOfATrackOnce) {
  Launch();
  LoadAndPlay(5);
  QSignalSpy ended(&player_, &CastMediaPlayer::TrackEnded);
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(0, 5, "IDLE", 200.5, "FINISHED"));
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(0, 5, "IDLE", 200.5, "FINISHED"));
  EXPECT_EQ(1, ended.count());
  EXPECT_EQ(CastMediaPlayer::Idle, player_.state());
}

TEST_F(CastMediaPlayerTest, ReportsErrors) {
  Launch();
  QSignalSpy errors(&player_, &CastMediaPlayer::Error);

  player_.Load(Info(), 0, true);
  const int request_id = channel_.Last("LOAD")["requestId"].toInt();
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   {{"type", "LOAD_FAILED"}, {"requestId", request_id}});
  EXPECT_EQ(1, errors.count());
  EXPECT_EQ(CastMediaPlayer::Idle, player_.state());

  LoadAndPlay(7);
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(0, 7, "IDLE", 3, "ERROR"));
  EXPECT_EQ(2, errors.count());
  EXPECT_EQ(CastMediaPlayer::Idle, player_.state());
}

TEST_F(CastMediaPlayerTest, LaunchErrors) {
  QSignalSpy errors(&player_, &CastMediaPlayer::Error);
  player_.Launch();
  channel_.Receive(CastMediaPlayer::kReceiverNamespace, "receiver-0",
                   {{"type", "LAUNCH_ERROR"}, {"reason", "NOT_ALLOWED"}});
  EXPECT_EQ(1, errors.count());
  EXPECT_FALSE(player_.is_ready());
}

TEST_F(CastMediaPlayerTest, NoticesTheAppGoing) {
  Launch();
  LoadAndPlay(5);
  QSignalSpy stopped(&player_, &CastMediaPlayer::AppStopped);

  // A volume-only status says nothing about apps.
  channel_.Receive(
      CastMediaPlayer::kReceiverNamespace, "receiver-0",
      {{"type", "RECEIVER_STATUS"},
       {"status", QJsonObject{{"volume", QJsonObject{{"level", 0.5}}}}}});
  EXPECT_EQ(0, stopped.count());
  EXPECT_DOUBLE_EQ(0.5, player_.volume());

  // Someone else casts to the device.
  channel_.Receive(CastMediaPlayer::kReceiverNamespace, "receiver-0",
                   NoAppStatus());
  EXPECT_EQ(1, stopped.count());
  EXPECT_FALSE(player_.is_ready());
  EXPECT_EQ(CastMediaPlayer::Idle, player_.state());
}

TEST_F(CastMediaPlayerTest, IgnoresOtherSessionsAndApps) {
  Launch();
  LoadAndPlay(5);

  // From something other than our app.
  channel_.Receive(CastMediaPlayer::kMediaNamespace, "web-9",
                   MediaStatus(0, 5, "PAUSED"));
  EXPECT_EQ(CastMediaPlayer::Playing, player_.state());

  // An older media session.
  channel_.Receive(CastMediaPlayer::kMediaNamespace, kTransportId,
                   MediaStatus(0, 4, "PAUSED"));
  EXPECT_EQ(CastMediaPlayer::Playing, player_.state());
}

TEST_F(CastMediaPlayerTest, SetsTheDevicesVolume) {
  Launch();
  player_.SetVolume(1.5);
  EXPECT_EQ("SET_VOLUME", channel_.sent_.last().payload["type"].toString());
  EXPECT_EQ("receiver-0", channel_.sent_.last().destination);
  EXPECT_DOUBLE_EQ(
      1.0,
      channel_.sent_.last().payload["volume"].toObject()["level"].toDouble());

  player_.SetMuted(true);
  EXPECT_TRUE(
      channel_.sent_.last().payload["volume"].toObject()["muted"].toBool());
}

TEST_F(CastMediaPlayerTest, StopsTheApp) {
  Launch();
  QSignalSpy stopped(&player_, &CastMediaPlayer::AppStopped);
  player_.StopApp();
  EXPECT_EQ("STOP", channel_.sent_.last().payload["type"].toString());
  EXPECT_EQ("session-1", channel_.sent_.last().payload["sessionId"].toString());
  EXPECT_EQ(1, stopped.count());
  EXPECT_FALSE(player_.is_ready());
}

// Runs the event loop until |done| or |timeout_ms| pass.
bool WaitFor(std::function<bool()> done, int timeout_ms) {
  QElapsedTimer timer;
  timer.start();
  while (!done()) {
    if (timer.elapsed() > timeout_ms) return false;
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
  }
  return true;
}

// Serves one file to anyone who asks, over plain HTTP.
class FileServer : public QTcpServer {
 public:
  explicit FileServer(const QByteArray& data) : data_(data) {
    EXPECT_TRUE(listen(QHostAddress::AnyIPv4));
  }

  int requests_ = 0;

 protected:
  void incomingConnection(qintptr descriptor) override {
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(descriptor);
    connect(socket, &QTcpSocket::readyRead, [this, socket]() {
      if (!socket->readAll().contains("\r\n\r\n")) return;
      ++requests_;
      socket->write(
          "HTTP/1.1 200 OK\r\nContent-Type: audio/mpeg\r\n"
          "Connection: close\r\nContent-Length: " +
          QByteArray::number(data_.size()) + "\r\n\r\n" + data_);
      socket->disconnectFromHost();
    });
  }

 private:
  QByteArray data_;
};

// This machine's address on the device's network.
QHostAddress LocalAddressFor(const QHostAddress& device) {
  for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
    for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
      if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol &&
          device.isInSubnet(entry.ip(), entry.prefixLength())) {
        return entry.ip();
      }
    }
  }
  return QHostAddress();
}

// Plays a few beeps on a real device when CAST_TEST_DEVICE is set to its
// address, eg.
//   CAST_TEST_DEVICE=192.168.1.20 ./castmediaplayer_test
// The device has to be able to connect back to this machine.
TEST(CastMediaPlayerLiveTest, PlaysOnARealDevice) {
  const QHostAddress device(qEnvironmentVariable("CAST_TEST_DEVICE"));
  if (device.isNull()) return;
  const QHostAddress local = LocalAddressFor(device);
  ASSERT_FALSE(local.isNull());

  // Devices don't report the end of a track as short as one beep, so it's
  // played eight times over; MP3 frames can simply be put end to end.
  // CAST_TEST_MEDIA can name another MP3 to play instead.
  QByteArray data;
  const QString media = qEnvironmentVariable("CAST_TEST_MEDIA");
  if (media.isEmpty()) {
    QFile file(":/testdata/beep.mp3");
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    data = file.readAll().repeated(8);
  } else {
    QFile file(media);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    data = file.readAll();
  }
  FileServer server(data);

  CastChannel channel;
  CastMediaPlayer player(&channel);
  QSignalSpy opened(&channel, &CastChannel::Opened);
  QSignalSpy ready(&player, &CastMediaPlayer::Ready);
  QSignalSpy states(&player, &CastMediaPlayer::StateChanged);
  QSignalSpy ended(&player, &CastMediaPlayer::TrackEnded);
  QSignalSpy errors(&player, &CastMediaPlayer::Error);
  // Printed at the end, to show what a real device says.
  QObject::connect(&channel, &CastChannel::MessageReceived,
                   [](const QString& name_space, const QString& source,
                      const QJsonObject& payload) {
                     std::cout << name_space.toStdString() << " from "
                               << source.toStdString() << ": "
                               << QJsonDocument(payload)
                                      .toJson(QJsonDocument::Compact)
                                      .toStdString()
                               << std::endl;
                   });

  channel.Open(device, 8009);
  ASSERT_TRUE(WaitFor([&]() { return opened.count() == 1; }, 10000));
  player.Launch();
  ASSERT_TRUE(WaitFor([&]() { return ready.count() == 1; }, 20000));

  CastMediaInfo info;
  info.url = QUrl(QString("http://%1:%2/beep.mp3")
                      .arg(local.toString())
                      .arg(server.serverPort()));
  info.content_type = "audio/mpeg";
  info.title = "Clementine test beep";
  player.Load(info, 0, true);
  ASSERT_TRUE(WaitFor(
      [&]() { return ended.count() == 1 || errors.count() > 0; }, 30000));
  EXPECT_EQ(0, errors.count());
  EXPECT_EQ(1, ended.count());
  EXPECT_GE(server.requests_, 1);

  bool played = false;
  for (const QList<QVariant>& args : states) {
    if (args[0].value<CastMediaPlayer::State>() == CastMediaPlayer::Playing) {
      played = true;
    }
  }
  EXPECT_TRUE(played);

  player.StopApp();
  channel.Close();
}

}  // namespace
