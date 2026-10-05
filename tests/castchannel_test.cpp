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

#include "chromecast/castchannel.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslSocket>
#include <QTcpServer>
#include <functional>

#include "gtest/gtest.h"
#include "test_utils.h"

namespace {

// A self-signed certificate, like a Cast device's.
const char kCertificate[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIBpDCCAUmgAwIBAgIUddVuDVfghWPfUg8QTjhyURsvnngwCgYIKoZIzj0EAwIw\n"
    "JjEkMCIGA1UEAwwbQ2xlbWVudGluZSB0ZXN0IENhc3QgZGV2aWNlMCAXDTI2MTAw\n"
    "NTEzMDA0NVoYDzIxMjYwOTExMTMwMDQ1WjAmMSQwIgYDVQQDDBtDbGVtZW50aW5l\n"
    "IHRlc3QgQ2FzdCBkZXZpY2UwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARU6R2t\n"
    "VFPHNzhoHDk191wS8gum0rQ4ZnyEfXKlofFlx+wseukGtVHjZ87xqD2My8TpXgz8\n"
    "cgEgf1r4IznQhHJuo1MwUTAdBgNVHQ4EFgQUU7re92nLkUNCYp79noCFW/uWODUw\n"
    "HwYDVR0jBBgwFoAUU7re92nLkUNCYp79noCFW/uWODUwDwYDVR0TAQH/BAUwAwEB\n"
    "/zAKBggqhkjOPQQDAgNJADBGAiEAqRJN3Di/lO5dblmnYIiAQd1F+1PTcbwCsHTM\n"
    "GObIM6wCIQCy9KQV3blCBnGaXf93CmQc3MhO/lImBiD+p4a58L8DuA==\n"
    "-----END CERTIFICATE-----\n";
const char kPrivateKey[] =
    "-----BEGIN PRIVATE KEY-----\n"
    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQg/Q/mXJbv7drZCd4N\n"
    "F3S8ThdCCF110mCHkmShF13QPNehRANCAARU6R2tVFPHNzhoHDk191wS8gum0rQ4\n"
    "ZnyEfXKlofFlx+wseukGtVHjZ87xqD2My8TpXgz8cgEgf1r4IznQhHJu\n"
    "-----END PRIVATE KEY-----\n";

const char kMediaNamespace[] = "urn:x-cast:com.google.cast.media";

// Runs the event loop until |done| or 5 seconds pass.
bool WaitFor(std::function<bool()> done) {
  QElapsedTimer timer;
  timer.start();
  while (!done()) {
    if (timer.elapsed() > 5000) return false;
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
  }
  return true;
}

CastChannel::Message Msg(const QString& source, const QString& destination,
                         const QString& name_space,
                         const QJsonObject& payload) {
  return {
      source, destination, name_space,
      QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact))};
}

QString Type(const CastChannel::Message& message) {
  return QJsonDocument::fromJson(message.payload.toUtf8())
      .object()["type"]
      .toString();
}

// Plays a Cast device: accepts one TLS connection and records what it gets.
class FakeDevice : public QTcpServer {
 public:
  FakeDevice() { EXPECT_TRUE(listen(QHostAddress::LocalHost)); }

  void Send(const CastChannel::Message& message) {
    socket_->write(CastChannel::Encode(message));
  }

  // Waits until |count| messages have arrived.
  bool WaitForMessages(int count) {
    return WaitFor([this, count]() { return received_.size() >= count; });
  }

  QSslSocket* socket_ = nullptr;
  QList<CastChannel::Message> received_;

 protected:
  void incomingConnection(qintptr descriptor) override {
    socket_ = new QSslSocket(this);
    socket_->setSocketDescriptor(descriptor);
    socket_->setLocalCertificate(QSslCertificate(kCertificate));
    socket_->setPrivateKey(QSslKey(kPrivateKey, QSsl::Ec));
    connect(socket_, &QSslSocket::readyRead, [this]() {
      buffer_.append(socket_->readAll());
      EXPECT_TRUE(CastChannel::Decode(&buffer_, &received_));
    });
    socket_->startServerEncryption();
  }

 private:
  QByteArray buffer_;
};

class CastChannelTest : public ::testing::Test {
 protected:
  void Open() {
    QSignalSpy opened(&channel_, &CastChannel::Opened);
    channel_.Open(QHostAddress::LocalHost, device_.serverPort());
    ASSERT_TRUE(WaitFor([&opened]() { return opened.count() == 1; }));
    // The virtual connection to the device.
    ASSERT_TRUE(device_.WaitForMessages(1));
  }

  FakeDevice device_;
  CastChannel channel_;
};

TEST(CastChannelCodingTest, RoundTrips) {
  QByteArray data = CastChannel::Encode(
      Msg("sender-0", "receiver-0", kMediaNamespace, {{"type", "PLAY"}}));
  data += CastChannel::Encode(
      Msg("receiver-0", "sender-0", kMediaNamespace, {{"type", "PAUSE"}}));

  QList<CastChannel::Message> messages;
  ASSERT_TRUE(CastChannel::Decode(&data, &messages));
  EXPECT_TRUE(data.isEmpty());
  ASSERT_EQ(2, messages.size());
  EXPECT_EQ("sender-0", messages[0].source_id);
  EXPECT_EQ("receiver-0", messages[0].destination_id);
  EXPECT_EQ(kMediaNamespace, messages[0].name_space);
  EXPECT_EQ("PLAY", Type(messages[0]));
  EXPECT_EQ("PAUSE", Type(messages[1]));
}

TEST(CastChannelCodingTest, WaitsForTheWholeMessage) {
  const QByteArray data = CastChannel::Encode(
      Msg("sender-0", "receiver-0", kMediaNamespace, {{"type", "PLAY"}}));

  QByteArray buffer = data.left(3);
  QList<CastChannel::Message> messages;
  ASSERT_TRUE(CastChannel::Decode(&buffer, &messages));
  EXPECT_TRUE(messages.isEmpty());

  buffer = data.left(data.size() - 1);
  ASSERT_TRUE(CastChannel::Decode(&buffer, &messages));
  EXPECT_TRUE(messages.isEmpty());
  EXPECT_EQ(data.size() - 1, buffer.size());

  buffer += data.right(1);
  ASSERT_TRUE(CastChannel::Decode(&buffer, &messages));
  EXPECT_EQ(1, messages.size());
  EXPECT_TRUE(buffer.isEmpty());
}

TEST(CastChannelCodingTest, RejectsBadData) {
  QList<CastChannel::Message> messages;

  // Larger than a Cast device would send.
  QByteArray too_large("\x00\x01\x00\x01", 4);
  EXPECT_FALSE(CastChannel::Decode(&too_large, &messages));

  QByteArray garbage("\x00\x00\x00\x03\xff\xff\xff", 7);
  EXPECT_FALSE(CastChannel::Decode(&garbage, &messages));
  EXPECT_TRUE(messages.isEmpty());
}

TEST_F(CastChannelTest, ConnectsToTheDevice) {
  Open();
  EXPECT_TRUE(channel_.is_open());
  EXPECT_EQ(CastChannel::kConnectionNamespace, device_.received_[0].name_space);
  EXPECT_EQ("receiver-0", device_.received_[0].destination_id);
  EXPECT_EQ("CONNECT", Type(device_.received_[0]));
}

TEST_F(CastChannelTest, AnswersPings) {
  Open();
  device_.Send(Msg("receiver-0", "sender-0", CastChannel::kHeartbeatNamespace,
                   {{"type", "PING"}}));
  ASSERT_TRUE(device_.WaitForMessages(2));
  EXPECT_EQ(CastChannel::kHeartbeatNamespace, device_.received_[1].name_space);
  EXPECT_EQ("PONG", Type(device_.received_[1]));
}

TEST_F(CastChannelTest, ConnectsToAnAppBeforeTalkingToIt) {
  Open();
  channel_.Send(kMediaNamespace, "app-1", {{"type", "GET_STATUS"}});
  channel_.Send(kMediaNamespace, "app-1", {{"type", "PLAY"}});
  ASSERT_TRUE(device_.WaitForMessages(4));
  EXPECT_EQ("app-1", device_.received_[1].destination_id);
  EXPECT_EQ("CONNECT", Type(device_.received_[1]));
  EXPECT_EQ("GET_STATUS", Type(device_.received_[2]));
  EXPECT_EQ("PLAY", Type(device_.received_[3]));

  // Once the app has gone, talking to it needs a new connection.
  device_.Send(Msg("app-1", "sender-0", CastChannel::kConnectionNamespace,
                   {{"type", "CLOSE"}}));
  QSignalSpy received(&channel_, &CastChannel::MessageReceived);
  ASSERT_TRUE(WaitFor([&received]() { return received.count() == 1; }));
  channel_.Send(kMediaNamespace, "app-1", {{"type", "PLAY"}});
  ASSERT_TRUE(device_.WaitForMessages(6));
  EXPECT_EQ("CONNECT", Type(device_.received_[4]));
  EXPECT_EQ("PLAY", Type(device_.received_[5]));
  EXPECT_TRUE(channel_.is_open());
}

TEST_F(CastChannelTest, DeliversMessagesForThisSender) {
  Open();
  QSignalSpy received(&channel_, &CastChannel::MessageReceived);
  device_.Send(Msg("app-1", "sender-other", kMediaNamespace,
                   {{"type", "MEDIA_STATUS"}, {"for", "someone else"}}));
  device_.Send(Msg("app-1", "*", kMediaNamespace,
                   {{"type", "MEDIA_STATUS"}, {"for", "everyone"}}));
  device_.Send(Msg("app-1", "sender-0", kMediaNamespace,
                   {{"type", "MEDIA_STATUS"}, {"for", "us"}}));
  ASSERT_TRUE(WaitFor([&received]() { return received.count() == 2; }));
  QCoreApplication::processEvents();
  ASSERT_EQ(2, received.count());

  EXPECT_EQ(kMediaNamespace, received[0][0].toString());
  EXPECT_EQ("app-1", received[0][1].toString());
  EXPECT_EQ("everyone", received[0][2].toJsonObject()["for"].toString());
  EXPECT_EQ("us", received[1][2].toJsonObject()["for"].toString());
}

TEST_F(CastChannelTest, DeviceClosingTheConnection) {
  Open();
  QSignalSpy closed(&channel_, &CastChannel::Closed);
  device_.Send(Msg("receiver-0", "sender-0", CastChannel::kConnectionNamespace,
                   {{"type", "CLOSE"}}));
  ASSERT_TRUE(WaitFor([&closed]() { return closed.count() == 1; }));
  EXPECT_FALSE(channel_.is_open());
}

TEST_F(CastChannelTest, DeviceDisconnecting) {
  Open();
  QSignalSpy closed(&channel_, &CastChannel::Closed);
  device_.socket_->disconnectFromHost();
  ASSERT_TRUE(WaitFor([&closed]() { return closed.count() == 1; }));
  EXPECT_FALSE(channel_.is_open());
}

TEST_F(CastChannelTest, GivesUpOnASilentDevice) {
  channel_.set_heartbeat_interval(50);
  Open();
  QSignalSpy closed(&channel_, &CastChannel::Closed);
  ASSERT_TRUE(WaitFor([&closed]() { return closed.count() == 1; }));
  EXPECT_EQ("The device stopped answering", closed[0][0].toString());

  // It was pinged in the meantime.
  bool pinged = false;
  for (const CastChannel::Message& message : device_.received_) {
    if (Type(message) == "PING") pinged = true;
  }
  EXPECT_TRUE(pinged);
}

TEST_F(CastChannelTest, CloseSaysGoodbyeQuietly) {
  Open();
  QSignalSpy closed(&channel_, &CastChannel::Closed);
  channel_.Close();
  EXPECT_FALSE(channel_.is_open());
  ASSERT_TRUE(device_.WaitForMessages(2));
  EXPECT_EQ("CLOSE", Type(device_.received_[1]));
  QCoreApplication::processEvents();
  EXPECT_EQ(0, closed.count());
}

TEST(CastChannelConnectTest, ReportsAFailedConnection) {
  quint16 port;
  {
    QTcpServer unused;
    ASSERT_TRUE(unused.listen(QHostAddress::LocalHost));
    port = unused.serverPort();
  }
  CastChannel channel;
  QSignalSpy closed(&channel, &CastChannel::Closed);
  channel.Open(QHostAddress::LocalHost, port);
  ASSERT_TRUE(WaitFor([&closed]() { return closed.count() == 1; }));
  EXPECT_FALSE(channel.is_open());
}

// Talks to a real device when CAST_TEST_DEVICE is set to its address, eg.
//   CAST_TEST_DEVICE=192.168.1.20 ./castchannel_test
TEST(CastChannelLiveTest, AsksARealDeviceForItsStatus) {
  const QString address = qEnvironmentVariable("CAST_TEST_DEVICE");
  if (address.isEmpty()) return;

  CastChannel channel;
  channel.set_heartbeat_interval(1000);
  QSignalSpy opened(&channel, &CastChannel::Opened);
  QSignalSpy closed(&channel, &CastChannel::Closed);
  QSignalSpy received(&channel, &CastChannel::MessageReceived);
  channel.Open(QHostAddress(address), 8009);
  ASSERT_TRUE(WaitFor([&opened]() { return opened.count() == 1; }));

  channel.Send("urn:x-cast:com.google.cast.receiver", CastChannel::kReceiverId,
               {{"type", "GET_STATUS"}, {"requestId", 1}});
  QJsonObject status;
  ASSERT_TRUE(WaitFor([&received, &status]() {
    for (const QList<QVariant>& args : received) {
      const QJsonObject payload = args[2].toJsonObject();
      if (payload["type"].toString() == "RECEIVER_STATUS") status = payload;
    }
    return !status.isEmpty();
  }));
  EXPECT_EQ(1, status["requestId"].toInt());
  EXPECT_TRUE(status["status"].toObject().contains("volume"));
  std::cout
      << "Receiver status: "
      << QJsonDocument(status).toJson(QJsonDocument::Compact).toStdString()
      << std::endl;

  // The device answers pings, so the channel outlives several heartbeats.
  QElapsedTimer timer;
  timer.start();
  WaitFor([&timer]() { return timer.elapsed() > 4500; });
  EXPECT_EQ(0, closed.count());
  EXPECT_TRUE(channel.is_open());
}

}  // namespace
