/* This file is part of Clementine.

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

#include "networkremote/networkremote.h"

#include <QDataStream>
#include <QHostAddress>
#include <QStringList>

#include "gtest/gtest.h"
#include "test_utils.h"

using Listening = NetworkRemote::Listening;

TEST(NetworkRemoteTest, AllAddressesListensOnTheWildcards) {
  const QList<QHostAddress> addresses =
      NetworkRemote::ListenAddresses(true, QStringList() << "192.168.1.5");

  // The chosen list is ignored when listening on everything.
  ASSERT_EQ(2, addresses.size());
  EXPECT_EQ(QHostAddress(QHostAddress::Any), addresses[0]);
  EXPECT_EQ(QHostAddress(QHostAddress::AnyIPv6), addresses[1]);
}

TEST(NetworkRemoteTest, ChosenAddressesInOrder) {
  const QList<QHostAddress> addresses = NetworkRemote::ListenAddresses(
      false, QStringList() << "100.65.55.75" << "192.168.86.178"
                           << "fd7a:115c:a1e0::f535:374b");

  ASSERT_EQ(3, addresses.size());
  EXPECT_EQ(QHostAddress("100.65.55.75"), addresses[0]);
  EXPECT_EQ(QHostAddress("192.168.86.178"), addresses[1]);
  EXPECT_EQ(QHostAddress("fd7a:115c:a1e0::f535:374b"), addresses[2]);
}

TEST(NetworkRemoteTest, SkipsAddressesThatDontParse) {
  const QList<QHostAddress> addresses = NetworkRemote::ListenAddresses(
      false, QStringList() << "1000.65.55.75" << "10.0.0.1" << "");

  ASSERT_EQ(1, addresses.size());
  EXPECT_EQ(QHostAddress("10.0.0.1"), addresses[0]);
}

TEST(NetworkRemoteTest, NothingChosenListensNowhere) {
  EXPECT_TRUE(NetworkRemote::ListenAddresses(false, QStringList()).isEmpty());
}

TEST(NetworkRemoteTest, LocalhostIsPrivate) {
  EXPECT_TRUE(NetworkRemote::IpIsPrivate(QHostAddress("127.0.0.1")));
  EXPECT_TRUE(NetworkRemote::IpIsPrivate(QHostAddress("::1")));
  // How an IPv4 client appears on a dual-stack socket.
  EXPECT_TRUE(NetworkRemote::IpIsPrivate(QHostAddress("::ffff:127.0.0.1")));
}

TEST(NetworkRemoteTest, PrivateAndPublicAddresses) {
  EXPECT_TRUE(NetworkRemote::IpIsPrivate(QHostAddress("192.168.1.10")));
  EXPECT_TRUE(NetworkRemote::IpIsPrivate(QHostAddress("::ffff:10.0.0.2")));
  EXPECT_FALSE(NetworkRemote::IpIsPrivate(QHostAddress("8.8.8.8")));
  EXPECT_FALSE(NetworkRemote::IpIsPrivate(QHostAddress("::ffff:8.8.8.8")));
}

TEST(NetworkRemoteTest, LinkLocalIsPrivate) {
  EXPECT_TRUE(NetworkRemote::IpIsPrivate(QHostAddress("169.254.0.7")));
  EXPECT_TRUE(NetworkRemote::IpIsPrivate(QHostAddress("::ffff:169.254.0.7")));
  EXPECT_TRUE(NetworkRemote::IpIsPrivate(QHostAddress("fe80::1")));
  EXPECT_TRUE(NetworkRemote::IpIsPrivate(QHostAddress("fd7a:115c:a1e0::1")));
}

TEST(NetworkRemoteTest, TailnetClientIsLocalOnAChosenTailnetAddress) {
  const QHostAddress local("100.65.55.75");
  EXPECT_TRUE(NetworkRemote::IsLocalClient(QHostAddress("100.101.2.3"), local,
                                           Listening::OnChosenAddress));
  // Not when listening on everything.
  EXPECT_FALSE(NetworkRemote::IsLocalClient(QHostAddress("100.101.2.3"), local,
                                            Listening::OnAllAddresses));
  // The internet stays out.
  EXPECT_FALSE(NetworkRemote::IsLocalClient(QHostAddress("8.8.8.8"), local,
                                            Listening::OnChosenAddress));
}

TEST(NetworkRemoteTest, TailnetClientIsntLocalOnALanAddress) {
  EXPECT_FALSE(NetworkRemote::IsLocalClient(QHostAddress("100.101.2.3"),
                                            QHostAddress("192.168.86.178"),
                                            Listening::OnChosenAddress));
}

TEST(NetworkRemoteTest, InternetIsntLocalOnAChosenLanAddress) {
  // A machine in a router's DMZ gets the internet on its LAN address.
  EXPECT_FALSE(NetworkRemote::IsLocalClient(QHostAddress("8.8.8.8"),
                                            QHostAddress("192.168.86.178"),
                                            Listening::OnChosenAddress));
  EXPECT_TRUE(NetworkRemote::IsLocalClient(QHostAddress("192.168.86.20"),
                                           QHostAddress("192.168.86.178"),
                                           Listening::OnChosenAddress));
}

TEST(NetworkRemoteTest, LocalClientsCanReachNonPublicAddresses) {
  EXPECT_TRUE(
      NetworkRemote::LocalClientsCanReach(QHostAddress("100.65.55.75")));
  EXPECT_TRUE(
      NetworkRemote::LocalClientsCanReach(QHostAddress("192.168.86.178")));
  EXPECT_TRUE(NetworkRemote::LocalClientsCanReach(
      QHostAddress("fd7a:115c:a1e0::f535:374b")));
  EXPECT_FALSE(
      NetworkRemote::LocalClientsCanReach(QHostAddress("203.0.113.5")));
}

TEST(NetworkRemoteTest, DisconnectMessageIsFramedLikeTheRemote) {
  const QByteArray framed =
      NetworkRemote::DisconnectMessage(cpb::remote::Not_Local_Network);

  QDataStream s(framed);
  qint32 length = 0;
  s >> length;
  ASSERT_EQ(framed.size() - 4, length);

  cpb::remote::Message msg;
  ASSERT_TRUE(msg.ParseFromArray(framed.constData() + 4, length));
  EXPECT_EQ(cpb::remote::DISCONNECT, msg.type());
  EXPECT_EQ(cpb::remote::Not_Local_Network,
            msg.response_disconnect().reason_disconnect());
  EXPECT_TRUE(msg.has_version());
}
