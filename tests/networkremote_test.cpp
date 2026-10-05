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
#include <algorithm>

#include "core/song.h"
#include "gtest/gtest.h"
#include "networkremote/authattemptlimiter.h"
#include "networkremote/outgoingdatacreator.h"
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

class AuthAttemptLimiterTest : public ::testing::Test {
 protected:
  AuthAttemptLimiterTest()
      : now_(0), limiter_([this]() { return now_; }), guesser_("203.0.113.7") {}

  void FailFreely(const QHostAddress& address) {
    for (int i = 0; i < AuthAttemptLimiter::kFreeFailures; ++i) {
      ASSERT_EQ(0, limiter_.LockedOutFor(address));
      EXPECT_EQ(0, limiter_.RecordFailure(address));
    }
  }

  qint64 now_;
  AuthAttemptLimiter limiter_;
  const QHostAddress guesser_;
};

TEST_F(AuthAttemptLimiterTest, AFewWrongCodesAreFree) {
  FailFreely(guesser_);
  EXPECT_EQ(0, limiter_.LockedOutFor(guesser_));
}

TEST_F(AuthAttemptLimiterTest, LockoutDoublesUpToTheMaximum) {
  FailFreely(guesser_);

  qint64 expected = AuthAttemptLimiter::kFirstLockoutMsec;
  for (int i = 0; i < 20; ++i) {
    ASSERT_EQ(0, limiter_.LockedOutFor(guesser_));
    EXPECT_EQ(expected, limiter_.RecordFailure(guesser_));

    now_ += expected - 1;
    EXPECT_EQ(1, limiter_.LockedOutFor(guesser_));
    now_ += 1;

    expected = std::min(expected * 2, AuthAttemptLimiter::kMaxLockoutMsec);
  }
}

TEST_F(AuthAttemptLimiterTest, GuessingAllTheCodesTakesYears) {
  // As fast as the limit allows.
  qint64 guesses = 0;
  while (now_ < 365LL * 24 * 60 * 60 * 1000) {
    now_ += limiter_.RecordFailure(guesser_);
    ++guesses;
  }
  // Half of the 100000 codes, the average needed, would take over five years.
  EXPECT_LT(guesses, 10000);
}

TEST_F(AuthAttemptLimiterTest, OtherAddressesAreUnaffected) {
  FailFreely(guesser_);
  limiter_.RecordFailure(guesser_);
  EXPECT_LT(0, limiter_.LockedOutFor(guesser_));
  EXPECT_EQ(0, limiter_.LockedOutFor(QHostAddress("203.0.113.8")));
}

TEST_F(AuthAttemptLimiterTest, ARightCodeForgetsTheAddress) {
  FailFreely(guesser_);
  limiter_.RecordSuccess(guesser_);
  EXPECT_EQ(0, limiter_.address_count());
  FailFreely(guesser_);
}

TEST_F(AuthAttemptLimiterTest, ADayWithoutWrongCodesForgetsTheAddress) {
  FailFreely(guesser_);
  limiter_.RecordFailure(guesser_);

  now_ += AuthAttemptLimiter::kForgetAfterMsec - 1;
  EXPECT_EQ(AuthAttemptLimiter::kFirstLockoutMsec * 2,
            limiter_.RecordFailure(guesser_));

  now_ += AuthAttemptLimiter::kForgetAfterMsec;
  FailFreely(guesser_);
}

TEST_F(AuthAttemptLimiterTest, IPv4IsTheSameOnADualStackSocket) {
  FailFreely(QHostAddress("::ffff:203.0.113.7"));
  limiter_.RecordFailure(guesser_);
  EXPECT_LT(0, limiter_.LockedOutFor(QHostAddress("::ffff:203.0.113.7")));
}

TEST_F(AuthAttemptLimiterTest, IPv6IsCountedBySlash64) {
  FailFreely(QHostAddress("2001:db8:1:2::1"));
  limiter_.RecordFailure(QHostAddress("2001:db8:1:2:aaaa::9"));
  EXPECT_LT(0, limiter_.LockedOutFor(QHostAddress("2001:db8:1:2:ffff::1")));
  EXPECT_EQ(0, limiter_.LockedOutFor(QHostAddress("2001:db8:1:3::1")));
}

TEST_F(AuthAttemptLimiterTest, KeepsABoundedNumberOfAddresses) {
  for (int i = 0; i < AuthAttemptLimiter::kMaxAddresses * 2; ++i) {
    now_ += 1;
    limiter_.RecordFailure(QHostAddress(quint32(0x0a000000 + i)));
  }
  EXPECT_EQ(AuthAttemptLimiter::kMaxAddresses, limiter_.address_count());

  // The latest is still counted: its sixth wrong code locks it out.
  const QHostAddress latest(
      quint32(0x0a000000 + AuthAttemptLimiter::kMaxAddresses * 2 - 1));
  for (int i = 1; i < AuthAttemptLimiter::kFreeFailures; ++i) {
    limiter_.RecordFailure(latest);
  }
  EXPECT_EQ(AuthAttemptLimiter::kFirstLockoutMsec,
            limiter_.RecordFailure(latest));
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

// A song that isn't valid (one whose file can't be read, say) is sent as the
// playlist shows it, with its own index rather than the first song's, 0.
TEST(NetworkRemoteTest, SongThatIsntValidIsSentAsThePlaylistShowsIt) {
  Song song;
  song.Init("Clair de lune", "Claude Debussy", "Suite bergamasque", 300);
  song.set_url(QUrl::fromLocalFile("/music/clair.ogg"));
  song.set_valid(false);

  cpb::remote::SongMetadata pb_song;
  OutgoingDataCreator::CreateSong(song, QImage(), 3, &pb_song);
  EXPECT_EQ(3, pb_song.index());
  EXPECT_TRUE(pb_song.has_id());
  EXPECT_EQ("Clair de lune", pb_song.title());
  EXPECT_EQ("Claude Debussy", pb_song.artist());
}

// The remotes take a song without an id for no song, as when none is playing.
TEST(NetworkRemoteTest, EmptySongHasNoFieldsButItsIndex) {
  cpb::remote::SongMetadata pb_song;
  OutgoingDataCreator::CreateSong(Song(), QImage(), 3, &pb_song);
  EXPECT_EQ(3, pb_song.index());
  EXPECT_FALSE(pb_song.has_id());
  EXPECT_FALSE(pb_song.has_title());
}
