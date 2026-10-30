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

#include <gst/gst.h>

#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QVariantMap>
#include <QtEndian>
#include <cmath>
#include <thread>

#include "airplay/airplaycrypto.h"
#include "airplay/airplaydevice.h"
#include "airplay/airplaydiscovery.h"
#include "airplay/airplayplist.h"
#include "airplay/airplaysession.h"
#include "airplay/airplaysink.h"
#include "airplay/airplaysrp.h"
#include "airplay/airplaystream.h"
#include "airplay/airplaytlv8.h"
#include "fakeairplayreceiver.h"
#include "gtest/gtest.h"
#include "test_utils.h"

using namespace airplay;

// For gtest's failure messages.
std::ostream& operator<<(std::ostream& stream, const QByteArray& bytes) {
  return stream << bytes.toHex().constData();
}

std::ostream& operator<<(std::ostream& stream, const QSet<QString>& set) {
  return stream << QStringList(set.begin(), set.end()).join(", ");
}

namespace {

// RFC 8439 section 2.8.2.
TEST(AirPlayTest, ChaCha20Poly1305) {
  QByteArray key;
  for (int i = 0x80; i < 0xa0; ++i) key.append(char(i));
  const QByteArray nonce = QByteArray::fromHex("070000004041424344454647");
  const QByteArray aad = QByteArray::fromHex("50515253c0c1c2c3c4c5c6c7");
  const QByteArray plain(
      "Ladies and Gentlemen of the class of '99: If I could offer you only "
      "one tip for the future, sunscreen would be it.");
  const QByteArray expected = QByteArray::fromHex(
      "d31a8d34648e60db7b86afbc53ef7ec2a4aded51296e08fea9e2b5a736ee62d63dbea4"
      "5e8ca9671282fafb69da92728b1a71de0a9e060b2905d6a5b67ecd3b3692ddbd7f2d77"
      "8b8c9803aee328091b58fab324e4fad675945585808b4831d7bc3ff4def08e4b7a9de5"
      "76d26586cec64b6116"
      "1ae10b594f09e26a7e902ecbd0600691");

  const QByteArray sealed = Seal(key, nonce, aad, plain);
  EXPECT_EQ(sealed.toHex(), expected.toHex());

  QByteArray opened;
  EXPECT_TRUE(Open(key, nonce, aad, sealed, &opened));
  EXPECT_EQ(opened, plain);

  QByteArray tampered = sealed;
  tampered[0] = char(tampered[0] ^ 1);
  EXPECT_FALSE(Open(key, nonce, aad, tampered, &opened));
}

// Checked against Python's cryptography package.
TEST(AirPlayTest, Hkdf) {
  QByteArray secret;
  for (int i = 0; i < 64; ++i) secret.append(char(i));
  EXPECT_EQ(HkdfSha512("Control-Salt", "Control-Write-Encryption-Key", secret)
                .toHex(),
            QByteArray("5a6cb19bcbe7d4df2dd8279f39562f7fae2dbf73eb5a4f98849c24"
                       "5c82b2fe96"));
}

TEST(AirPlayTest, Nonce) {
  EXPECT_EQ(Nonce(0x0102).toHex(), QByteArray("000000000201000000000000"));
}

TEST(AirPlayTest, FrameCipherRoundTrip) {
  const QByteArray a(32, 'a');
  const QByteArray b(32, 'b');
  FrameCipher sender(a, b);
  FrameCipher receiver(b, a);
  // Over 1024 bytes, so it's split into two frames.
  const QByteArray message(1500, 'x');
  QByteArray wire = sender.Encrypt(message);
  wire += sender.Encrypt("more");
  EXPECT_EQ(wire.size(), 1500 + 4 + 2 * 3 + 16 * 3);

  // Arriving in pieces.
  QByteArray plain;
  EXPECT_TRUE(receiver.Decrypt(wire.left(100), &plain));
  EXPECT_TRUE(plain.isEmpty());
  EXPECT_TRUE(receiver.Decrypt(wire.mid(100), &plain));
  EXPECT_EQ(plain, message + "more");
}

// The same inputs give the same results as the Rust proof of concept,
// which was checked against srptools. The salt's leading zero byte is
// dropped before hashing, as srptools does.
TEST(AirPlayTest, Srp) {
  const QByteArray a(QByteArray::fromHex(QByteArray(64, '1')));
  const QByteArray salt =
      QByteArray::fromHex("00a1b2c3d4e5f60718293a4b5c6d7e8f");
  const QByteArray b_hash = Sha512("b").toHex();
  const QByteArray b = QByteArray::fromHex(
      "7f" + (b_hash + b_hash + b_hash + b_hash + b_hash + b_hash).mid(2));

  const SrpClient::Result r = SrpClient(a).Process("3939", salt, b);
  EXPECT_EQ(
      r.public_key.toHex(),
      QByteArray(
          "c95341372ca4d0b469f87c0ae37bda635713223b577ef5e8d64a959a54e53957"
          "22fea50e71bbd44306055760924a64885805ae13545fcfc7e65f4e7b75c765a1"
          "12d267f79bffb8353d96ae81cfa64ca7eb687a5dba4cf223b21e1362489ca3e0"
          "56254b25f610c00643490ea19944f6d3d0872bdcc5fc338bebc8936f30b69592"
          "4e117f364a4cfc3898eeaa1a4bb98957df8eb046eba3cf42558138ca616a1dc7"
          "991426292dd258693bbd4c11816e3064e4a7c536f7255ae61735db25e3cf0349"
          "eb8b3e9848d0aa572b0f809d8983f1b280581aba96104422fcf088b698f8d899"
          "115a5653cc473e87f5e45552a5a126c8b7fc273bd2cd12bfb15ff3501a572b17"
          "608f16c33f6e971081f3af1be37a0a96748f60d204e935f6a5d2a71303aea434"
          "b45bcb9ab6b4444208d3fdf7a00c07a3ba2a09b56b578170c78ba1573ca26924"
          "06ba8d461e43bb3d160f0024a388d7234236537e29c3604f666c69d7244eae29"
          "a0e8b9fa5ee490890e5892973fa535707fbe8f06e90dcbb99c7b39b5eb2b45f"
          "b"));
  EXPECT_EQ(
      r.proof.toHex(),
      QByteArray(
          "dcffaf5aa2754af8645619e632a9cc034c7f8d5c6095120893a6bff857f89887"
          "971aa93102c75d77e80553b98eb7351fc98bcf01a840d52a1806d83607b39eb"
          "1"));
  EXPECT_EQ(
      r.key.toHex(),
      QByteArray(
          "eb3b50c2e3514a00ec9c72d3bf32d99f2145de13f86a74cdf0b7a49a38477051"
          "4011c96ee111133bf8885a879360fbee39c58c0d74d6cefb0a52277419451b9"
          "0"));
}

TEST(AirPlayTest, Tlv8) {
  const QByteArray big(300, 'k');
  const QByteArray wire = tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 3)},
                                       {tlv8::kPublicKey, big},
                                       {tlv8::kProof, QByteArray()}});
  EXPECT_EQ(wire.size(), 3 + 2 + 255 + 2 + 45 + 2);
  const tlv8::Items items = tlv8::Read(wire);
  EXPECT_EQ(items.size(), 3);
  EXPECT_EQ(tlv8::Get(items, tlv8::kPublicKey), big);
  EXPECT_EQ(tlv8::Get(items, tlv8::kSeqNo), QByteArray(1, 3));
  EXPECT_TRUE(tlv8::Get(items, tlv8::kSalt).isNull());
}

TEST(AirPlayTest, BinaryPlistRoundTrip) {
  QVariantMap stream;
  stream["audioFormat"] = 0x800;
  stream["shk"] = QByteArray(32, '\x01');
  stream["isMedia"] = true;
  stream["latencyMax"] = 88200;
  stream["big"] = qint64(0x123456789LL);
  stream["negative"] = -1;
  stream["ratio"] = 0.5;
  stream["name"] = QString("Clementine on a very long name indeed");
  stream["unicode"] = QString::fromUtf8("Caf\xc3\xa9");
  QVariantMap root;
  root["streams"] = QVariantList{stream, QVariantList{}};

  const QByteArray data = bplist::Write(root);
  EXPECT_TRUE(data.startsWith("bplist00"));
  QVariant read;
  EXPECT_TRUE(bplist::Read(data, &read));
  const QVariantMap s = read.toMap().value("streams").toList().value(0).toMap();
  EXPECT_EQ(s.value("audioFormat").toLongLong(), 0x800);
  EXPECT_EQ(s.value("shk").toByteArray(), QByteArray(32, '\x01'));
  EXPECT_EQ(s.value("isMedia").toBool(), true);
  EXPECT_EQ(s.value("latencyMax").toLongLong(), 88200);
  EXPECT_EQ(s.value("big").toLongLong(), 0x123456789LL);
  EXPECT_EQ(s.value("negative").toLongLong(), -1);
  EXPECT_EQ(s.value("ratio").toDouble(), 0.5);
  EXPECT_EQ(s.value("name").toString(),
            QString("Clementine on a very long name indeed"));
  EXPECT_EQ(s.value("unicode").toString(), QString::fromUtf8("Caf\xc3\xa9"));
}

// Written by Python's plistlib, which uses 1-byte refs and offsets.
TEST(AirPlayTest, BinaryPlistFromPython) {
  const QByteArray data = QByteArray::fromHex(
      "62706c6973743030d3010203040506596576656e74506f7274546e616d655a74"
      "696d696e67506f727411ddad597a61726e69776f6f701000080f191e292c3600"
      "00000000000101000000000000000700000000000000000000000000000038");
  QVariant read;
  EXPECT_TRUE(bplist::Read(data, &read));
  const QVariantMap map = read.toMap();
  EXPECT_EQ(map.value("eventPort").toLongLong(), 56749);
  EXPECT_EQ(map.value("timingPort").toLongLong(), 0);
  EXPECT_EQ(map.value("name").toString(), QString("zarniwoop"));
}

TEST(AirPlayTest, BinaryPlistRejectsGarbage) {
  QVariant read;
  EXPECT_FALSE(bplist::Read("bplist00", &read));
  EXPECT_FALSE(bplist::Read(QByteArray(64, '\xff'), &read));
  QByteArray truncated = bplist::Write(QVariantMap{{"a", 1}});
  truncated[truncated.size() - 1] = char(0xff);  // Table offset past the end
  EXPECT_FALSE(bplist::Read(truncated, &read));
}

// zarniwoop's _airplay._tcp TXT record, with "Require password" on.
const QList<QByteArray> kMacTxt = {"acl=0",
                                   "deviceid=E6:43:60:C0:42:4C",
                                   "features=0x4A7FCFD5,0x38174FDE",
                                   "fex=1c9/St5PFzg2IYxA",
                                   "flags=0x284",
                                   "model=Mac14,2",
                                   "protovers=1.1",
                                   "srcvers=980.77.5"};

TEST(AirPlayDeviceTest, ParsesAMacsTxt) {
  AirPlayDevice device;
  ASSERT_TRUE(device.ParseTxt(kMacTxt));
  EXPECT_EQ("E6:43:60:C0:42:4C", device.id);
  EXPECT_EQ("Mac14,2", device.model);
  EXPECT_EQ(Q_UINT64_C(0x38174fde4a7fcfd5), device.features);
  EXPECT_EQ(0x284u, device.status_flags);
  EXPECT_TRUE(device.password_required());
  EXPECT_TRUE(device.supports_airplay2());
}

TEST(AirPlayDeviceTest, NoPasswordWithoutTheFlag) {
  QList<QByteArray> txt = kMacTxt;
  txt.replace(4, "flags=0x84");
  AirPlayDevice device;
  ASSERT_TRUE(device.ParseTxt(txt));
  EXPECT_FALSE(device.password_required());
}

TEST(AirPlayDeviceTest, NeedsADeviceId) {
  AirPlayDevice device;
  EXPECT_FALSE(device.ParseTxt({"model=Mac14,2", "flags=0x4"}));
}

TEST(AirPlayDeviceTest, SplitsWireFormatTxt) {
  const QByteArray rdata = QByteArray(
      "\x05"
      "acl=0"
      "\x0b"
      "flags=0x284",
      18);
  EXPECT_EQ(QByteArray("acl=0|flags=0x284"),
            AirPlayDevice::SplitTxt(rdata).join('|'));
}

AirPlayDevice Receiver(const QByteArray& id, const QString& name,
                       const QString& address) {
  AirPlayDevice device;
  EXPECT_TRUE(device.ParseTxt({"deviceid=" + id, "model=Mac14,2"}));
  device.name = name;
  device.address = QHostAddress(address);
  device.port = 7000;
  return device;
}

class FakeAirPlayDiscovery : public AirPlayDiscovery {
 public:
  FakeAirPlayDiscovery() : AirPlayDiscovery(kAirPlayServiceType) {
    connect(this, &AirPlayDiscovery::DeviceFound,
            [this](const AirPlayDevice& device) { found_ << device.name; });
    connect(this, &AirPlayDiscovery::DeviceLost,
            [this](const QString& id) { lost_ << id; });
  }

  void Start() override {}

  void Resolve(const QString& service, const AirPlayDevice& device) {
    ServiceAdded(service);
    ServiceResolved(service, device);
  }
  using AirPlayDiscovery::ServiceRemoved;

  QStringList found_;
  QStringList lost_;
};

// A receiver is advertised on each interface and address family.
TEST(AirPlayDiscoveryTest, MergesAdvertisementsOfOneReceiver) {
  FakeAirPlayDiscovery discovery;
  const AirPlayDevice mac = Receiver("E6:43", "zarniwoop", "192.168.86.213");
  discovery.Resolve("eth0/ipv4", mac);
  discovery.Resolve("eth0/ipv6", mac);
  EXPECT_EQ(QStringList({"zarniwoop"}), discovery.found_);

  discovery.ServiceRemoved("eth0/ipv4");
  EXPECT_TRUE(discovery.lost_.isEmpty());
  discovery.ServiceRemoved("eth0/ipv6");
  EXPECT_EQ(QStringList({"E6:43"}), discovery.lost_);
}

TEST(AirPlayDiscoveryTest, ReportsARenamedReceiver) {
  FakeAirPlayDiscovery discovery;
  discovery.Resolve("eth0/ipv4",
                    Receiver("E6:43", "zarniwoop", "192.168.86.213"));
  discovery.Resolve("eth0/ipv4", Receiver("E6:43", "Study", "192.168.86.213"));
  EXPECT_EQ(QStringList({"zarniwoop", "Study"}), discovery.found_);
}

// Options that don't keep a test waiting.
AirPlaySession::Options FastOptions() {
  AirPlaySession::Options options;
  options.name = "Clementine on test";
  options.connect_timeout_msec = 1000;
  options.request_timeout_msec = 1000;
  options.approval_timeout_msec = 2000;
  return options;
}

struct Result {
  bool paired = false;
  bool set_up = false;
  QString error;
  // Why the receiver's user didn't allow it.
  QString not_allowed;
  // Whether it wants a password, and whether the one given was wrong.
  bool password_needed = false;
  bool password_wrong = false;
  qint64 msec = 0;
};

// Opens |session| to |device| and waits until it's set up or failed.
Result Open(AirPlaySession* session, const AirPlayDevice& device) {
  QSignalSpy paired(session, &AirPlaySession::Paired);
  QSignalSpy set_up(session, &AirPlaySession::SetUp);
  QSignalSpy failed(session, &AirPlaySession::Failed);
  QSignalSpy not_allowed(session, &AirPlaySession::NotAllowed);
  QSignalSpy password(session, &AirPlaySession::PasswordNeeded);
  QSignalSpy forgotten(session, &AirPlaySession::PairingForgotten);
  QElapsedTimer timer;
  timer.start();
  session->Open(device);
  while (set_up.isEmpty() && failed.isEmpty() && not_allowed.isEmpty() &&
         password.isEmpty() && forgotten.isEmpty() && timer.elapsed() < 10000) {
    set_up.wait(50);
  }
  Result result;
  result.paired = !paired.isEmpty();
  result.set_up = !set_up.isEmpty();
  if (!failed.isEmpty()) result.error = failed.first().first().toString();
  if (!not_allowed.isEmpty()) {
    result.not_allowed = not_allowed.first().first().toString();
  }
  result.password_needed = !password.isEmpty();
  if (result.password_needed) {
    result.password_wrong = password.first().first().toBool();
  }
  result.msec = timer.elapsed();
  return result;
}

Result Pair(const AirPlayDevice& device,
            const AirPlaySession::Options& options) {
  AirPlaySession session(options);
  return Open(&session, device);
}

// Opens |session| to |device| and waits until its stream is ready.
bool OpenReady(AirPlaySession* session, const AirPlayDevice& device) {
  QSignalSpy ready(session, &AirPlaySession::Ready);
  QSignalSpy failed(session, &AirPlaySession::Failed);
  session->Open(device);
  QElapsedTimer timer;
  timer.start();
  while (ready.isEmpty() && failed.isEmpty() && timer.elapsed() < 10000) {
    ready.wait(50);
  }
  return !ready.isEmpty();
}

// |frames| of a 440 Hz tone, as S16LE stereo.
QByteArray Tone(int frames) {
  QByteArray pcm;
  for (int i = 0; i < frames; ++i) {
    const qint16 s = qint16(8000 * std::sin(2 * M_PI * 440 * i / 44100.0));
    pcm += airplay::Bytes(qToLittleEndian(s));
    pcm += airplay::Bytes(qToLittleEndian(s));
  }
  return pcm;
}

// Runs the event loop until |done| or a second has passed.
void WaitFor(std::function<bool()> done) {
  QElapsedTimer timer;
  timer.start();
  while (!done() && timer.elapsed() < 1000) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
  }
}

TEST(AirPlaySessionTest, Pairs) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  const Result result = Pair(receiver.device(), FastOptions());
  EXPECT_TRUE(result.set_up) << result.error;
  EXPECT_TRUE(receiver.paired());
  // pair-pin-start would make it show a PIN, which transient pairing doesn't
  // use.
  EXPECT_EQ(QStringList(
                {"GET /info", "POST /pair-setup", "POST /pair-setup", "SETUP"}),
            receiver.requests().mid(0, 4));
  EXPECT_EQ(QStringList(), receiver.errors());
  EXPECT_GE(receiver.timing_replies(), 1);
  EXPECT_EQ(QSet<QString>({"Clementine on test"}), receiver.client_names());
}

TEST(AirPlaySessionTest, WaitsForTheReceiversUserToAllow) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_on_prompt([&receiver](const QString&) {
    QTimer::singleShot(200, &receiver, [&receiver]() { receiver.Allow(); });
  });

  const Result result = Pair(receiver.device(), FastOptions());
  EXPECT_TRUE(result.set_up) << result.error;
  EXPECT_EQ(1, receiver.prompts());
  EXPECT_GE(result.msec, 200);
}

// A Mac asks once per sender name.
TEST(AirPlaySessionTest, IsAskedAboutOncePerName) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_on_prompt([&receiver](const QString&) {
    QTimer::singleShot(0, &receiver, [&receiver]() { receiver.Allow(); });
  });

  AirPlaySession::Options options = FastOptions();
  EXPECT_TRUE(Pair(receiver.device(), options).set_up);
  EXPECT_TRUE(Pair(receiver.device(), options).set_up);
  EXPECT_EQ(1, receiver.prompts());

  options.name = "Clementine elsewhere";
  EXPECT_TRUE(Pair(receiver.device(), options).set_up);
  EXPECT_EQ(2, receiver.prompts());
}

// The receiver's user can take longer than other responses may.
TEST(AirPlaySessionTest, ApprovalOutlastsTheRequestTimeout) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_on_prompt([&receiver](const QString&) {
    QTimer::singleShot(300, &receiver, [&receiver]() { receiver.Allow(); });
  });

  AirPlaySession::Options options = FastOptions();
  options.request_timeout_msec = 100;
  options.approval_timeout_msec = 1000;
  const Result result = Pair(receiver.device(), options);
  EXPECT_TRUE(result.set_up) << result.error;
}

// A receiver whose flags say it has no password may still want one.
TEST(AirPlaySessionTest, SaysAPairingRefusalMayMeanAPassword) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);
  receiver.SetPassword("foobar");
  receiver.set_report_password(false);

  // So the refusal asks for one.
  const Result result = Pair(receiver.device(), FastOptions());
  EXPECT_FALSE(result.paired);
  EXPECT_TRUE(result.password_needed);
  EXPECT_FALSE(result.password_wrong);
}

TEST(AirPlaySessionTest, SaysItsWaitingForTheReceiversUser) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_on_prompt([&receiver](const QString&) {
    QTimer::singleShot(1200, &receiver, [&receiver]() { receiver.Allow(); });
  });

  AirPlaySession session(FastOptions());
  QSignalSpy waiting(&session, &AirPlaySession::WaitingForApproval);
  EXPECT_TRUE(Open(&session, receiver.device()).set_up);
  EXPECT_EQ(1, waiting.count());
}

// A Mac remembers which senders were allowed by this and the name.
TEST(AirPlaySessionTest, SendsItsSenderId) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession::Options options = FastOptions();
  options.sender_id = "02:11:22:33:44:55";
  EXPECT_TRUE(Pair(receiver.device(), options).set_up);
  EXPECT_EQ("02:11:22:33:44:55", receiver.sender_mac_address());
}

TEST(AirPlaySessionTest, TearsDown) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(FastOptions());
  ASSERT_TRUE(Open(&session, receiver.device()).set_up);
  session.Close();
  WaitFor([&receiver]() { return receiver.torn_down(); });
  EXPECT_TRUE(receiver.torn_down()) << receiver.requests().join(",");
}

TEST(AirPlaySessionTest, SaysWhenTheReceiversUserDoesntAllowIt) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_on_prompt([&receiver](const QString&) {
    QTimer::singleShot(1200, &receiver, [&receiver]() { receiver.Deny(); });
  });

  // Not a failure of the receiver: the user may allow it yet.
  AirPlaySession session(FastOptions());
  const Result result = Open(&session, receiver.device());
  EXPECT_FALSE(result.set_up);
  EXPECT_TRUE(result.error.isEmpty());
  EXPECT_EQ("Fake didn't allow Clementine on test to play there",
            result.not_allowed);
  EXPECT_FALSE(session.is_open());

  // Trying again asks again.
  receiver.set_on_prompt([&receiver](const QString&) {
    QTimer::singleShot(0, &receiver, [&receiver]() { receiver.Allow(); });
  });
  EXPECT_TRUE(Open(&session, receiver.device()).set_up);
  EXPECT_EQ(2, receiver.prompts());
}

TEST(AirPlaySessionTest, UnansweredPromptTimesOut) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());

  AirPlaySession::Options options = FastOptions();
  options.approval_timeout_msec = 300;
  const Result result = Pair(receiver.device(), options);
  EXPECT_FALSE(result.set_up);
  EXPECT_EQ("The AirPlay receiver didn't respond in 0.3 s", result.error);
  EXPECT_EQ(1, receiver.prompts());
}

// Passwords aren't supported yet, and a receiver that wants one says so
// before pairing.
TEST(AirPlaySessionTest, AsksForAPasswordBeforePairing) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.SetPassword("foobar");

  const Result result = Pair(receiver.device(), FastOptions());
  EXPECT_TRUE(result.password_needed);
  EXPECT_FALSE(result.password_wrong);
  EXPECT_EQ(QStringList({"GET /info"}), receiver.requests());
}

// The password is transient pairing's PIN.
TEST(AirPlaySessionTest, PairsWithAPassword) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);
  receiver.SetPassword("foobar");

  AirPlaySession::Options options = FastOptions();
  options.password = "foobar";
  EXPECT_TRUE(Pair(receiver.device(), options).set_up);
}

TEST(AirPlaySessionTest, SaysWhenThePasswordIsWrong) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);
  receiver.SetPassword("foobar");

  AirPlaySession::Options options = FastOptions();
  options.password = "wrong";
  const Result result = Pair(receiver.device(), options);
  EXPECT_FALSE(result.paired);
  EXPECT_TRUE(result.password_needed);
  EXPECT_TRUE(result.password_wrong);
}

// TEST-NET-1 (RFC 5737) never answers.
TEST(AirPlaySessionTest, UnansweredConnectTimesOut) {
  AirPlayDevice device;
  device.id = "AA:BB:CC:00:11:22";
  device.name = "Nowhere";
  device.address = QHostAddress("192.0.2.1");
  device.port = 7000;

  AirPlaySession::Options options = FastOptions();
  options.connect_timeout_msec = 200;
  const Result result = Pair(device, options);
  EXPECT_FALSE(result.paired);
  if (result.error.contains("unreachable", Qt::CaseInsensitive)) {
    std::cout << "Skipped: this machine has no route to TEST-NET-1\n";
    return;
  }
  EXPECT_EQ(
      "Couldn't connect to the AirPlay receiver at 192.0.2.1:7000 in 0.2 s. "
      "Is it awake, with AirPlay on?",
      result.error);
}

// The receiver asks for the time while setting up, and holds the SETUP
// that starts the stream until it's answered.
TEST(AirPlaySessionTest, AnswersTheTimeWhileSettingUp) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(FastOptions());
  EXPECT_TRUE(OpenReady(&session, receiver.device()));
  EXPECT_EQ(1, receiver.setups_after_timing());
  EXPECT_GE(receiver.timing_replies(), 2);
}

TEST(AirPlaySessionTest, StreamsAudio) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(FastOptions());
  ASSERT_TRUE(OpenReady(&session, receiver.device()));

  // 12 whole packets of 352 frames, and some left over for the next write.
  const QByteArray tone = Tone(4410);
  session.stream()->Write(tone.constData(), tone.size());
  WaitFor([&receiver]() { return receiver.audio_packets() >= 12; });
  EXPECT_EQ(12, receiver.audio_packets());
  EXPECT_EQ(tone.left(12 * 352 * 4), receiver.audio());
  EXPECT_GE(receiver.sync_packets(), 1);
  EXPECT_EQ(1, receiver.event_replies());
  EXPECT_EQ(QStringList(), receiver.errors());
}

// GStreamer's streaming thread writes the audio.
TEST(AirPlaySessionTest, StreamsAudioWrittenFromAnotherThread) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(FastOptions());
  ASSERT_TRUE(OpenReady(&session, receiver.device()));

  const QByteArray tone = Tone(352 * 4);
  AirPlayStream* stream = session.stream();
  std::thread writer(
      [stream, &tone]() { stream->Write(tone.constData(), tone.size()); });
  writer.join();
  WaitFor([&receiver]() { return receiver.audio_packets() >= 4; });
  EXPECT_EQ(tone, receiver.audio());
  EXPECT_EQ(QStringList(), receiver.errors());
}

TEST(AirPlaySessionTest, SetsTheVolumeFlushesAndTearsDown) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(FastOptions());
  // Set before it's ready, which is sent once it is.
  session.SetVolume(50);
  ASSERT_TRUE(OpenReady(&session, receiver.device()));
  WaitFor([&receiver]() { return !receiver.volume().isEmpty(); });
  // Half the amplitude.
  EXPECT_EQ("-6.020600", receiver.volume());

  session.Flush();
  WaitFor([&receiver]() { return receiver.flushes() == 1; });
  EXPECT_EQ(1, receiver.flushes());

  session.Close();
  WaitFor([&receiver]() { return receiver.torn_down(); });
  EXPECT_TRUE(receiver.torn_down());
}

class CapturingTarget : public AirPlaySinkTarget {
 public:
  void Write(const char* pcm, int size) override {
    QMutexLocker l(&mutex_);
    data_.append(pcm, size);
  }
  void Flushed() override {}

  int size() {
    QMutexLocker l(&mutex_);
    return data_.size();
  }

 private:
  QMutex mutex_;
  QByteArray data_;
};

TEST(AirPlaySinkTest, HandsWhatItRendersToItsTarget) {
  gst_init(nullptr, nullptr);
  AirPlaySink::RegisterElement();
  std::shared_ptr<CapturingTarget> target(new CapturingTarget);
  AirPlaySink::SetTarget("test-device", target);

  GError* error = nullptr;
  GstElement* pipeline = gst_parse_launch(
      "audiotestsrc num-buffers=4 samplesperbuffer=441 ! audioconvert ! "
      "audioresample ! clementineairplaysink device=test-device",
      &error);
  ASSERT_TRUE(pipeline) << (error ? error->message : "");
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  GstBus* bus = gst_element_get_bus(pipeline);
  GstMessage* message = gst_bus_timed_pop_filtered(
      bus, 5 * GST_SECOND, GstMessageType(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
  ASSERT_TRUE(message);
  EXPECT_EQ(GST_MESSAGE_EOS, GST_MESSAGE_TYPE(message));
  gst_message_unref(message);
  gst_object_unref(bus);
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
  AirPlaySink::RemoveTarget("test-device");

  // Four buffers of 441 S16 stereo frames.
  EXPECT_EQ(4 * 441 * 4, target->size());
}

// While paused, nothing is written, so the stream falls behind the time.
// Resuming flushes, and what comes after must not be too late to play.
TEST(AirPlaySessionTest, PlaysOnInTimeAfterAPause) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(FastOptions());
  ASSERT_TRUE(OpenReady(&session, receiver.device()));
  const QByteArray tone = Tone(352 * 4);
  session.stream()->Write(tone.constData(), tone.size());
  WaitFor([&receiver]() { return receiver.audio_packets() >= 4; });

  // Longer than the receiver's 1.5 s latency.
  QTest::qWait(1800);
  session.Flush();
  session.stream()->Write(tone.constData(), tone.size());
  WaitFor([&receiver]() { return receiver.audio_packets() >= 8; });
  EXPECT_EQ(8, receiver.audio_packets());
  EXPECT_EQ(0, receiver.late_packets());
  EXPECT_EQ(0, receiver.dropped_packets());
  EXPECT_EQ(tone + tone, receiver.audio());
}

// Between tracks nothing is written for a moment, without a FLUSH.
TEST(AirPlaySessionTest, PlaysOnInTimeAfterAGap) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(FastOptions());
  ASSERT_TRUE(OpenReady(&session, receiver.device()));
  const QByteArray tone = Tone(352 * 4);
  session.stream()->Write(tone.constData(), tone.size());
  WaitFor([&receiver]() { return receiver.audio_packets() >= 4; });

  QTest::qWait(1800);
  session.stream()->Write(tone.constData(), tone.size());
  WaitFor([&receiver]() { return receiver.audio_packets() >= 8; });
  EXPECT_EQ(0, receiver.late_packets());
  EXPECT_EQ(tone + tone, receiver.audio());
}

// RFC 8032 section 7.1, test 1.
TEST(AirPlayTest, Ed25519) {
  const QByteArray seed = QByteArray::fromHex(
      "9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60");
  const QByteArray public_key = Ed25519PublicKey(seed);
  EXPECT_EQ(
      QByteArray(
          "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a"),
      public_key.toHex());
  const QByteArray signature = Ed25519Sign(seed, QByteArray());
  EXPECT_EQ(
      QByteArray(
          "e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8"
          "821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b"),
      signature.toHex());
  EXPECT_TRUE(Ed25519Verify(public_key, signature, QByteArray()));
  EXPECT_FALSE(Ed25519Verify(public_key, signature, "x"));
}

// RFC 7748 section 6.1.
TEST(AirPlayTest, X25519) {
  const QByteArray alice = QByteArray::fromHex(
      "77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a");
  const QByteArray bob = QByteArray::fromHex(
      "5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb");
  EXPECT_EQ(
      QByteArray(
          "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a"),
      X25519PublicKey(alice).toHex());
  EXPECT_EQ(
      QByteArray(
          "4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742"),
      X25519Shared(alice, X25519PublicKey(bob)).toHex());
}

AirPlaySession::Options PersistentOptions() {
  AirPlaySession::Options options = FastOptions();
  options.sender_id = "02:11:22:33:44:55";
  options.identity_seed = QByteArray(32, '\x07');
  return options;
}

// Pairing with long-term keys the first time, then pair-verify.
TEST(AirPlaySessionTest, PairsPersistentlyThenVerifies) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession::Options options = PersistentOptions();
  QByteArray receiver_id;
  QByteArray receiver_key;
  {
    AirPlaySession session(options);
    QObject::connect(&session, &AirPlaySession::PairedPersistently,
                     [&](const QByteArray& id, const QByteArray& key) {
                       receiver_id = id;
                       receiver_key = key;
                     });
    EXPECT_TRUE(Open(&session, receiver.device()).set_up);
  }
  EXPECT_EQ(FakeAirPlayReceiver::PairingId(), receiver_id);
  EXPECT_EQ(FakeAirPlayReceiver::PublicKey(), receiver_key);
  EXPECT_EQ(Ed25519PublicKey(options.identity_seed),
            receiver.senders().value("02:11:22:33:44:55"));

  options.receiver_pairing_id = receiver_id;
  options.receiver_public_key = receiver_key;
  AirPlaySession session(options);
  const int requests = receiver.requests().size();
  EXPECT_TRUE(Open(&session, receiver.device()).set_up);
  const QStringList second = receiver.requests().mid(requests);
  EXPECT_EQ(QStringList({"GET /info", "POST /pair-verify", "POST /pair-verify",
                         "SETUP"}),
            second.mid(0, 4));
  EXPECT_EQ(QStringList(), receiver.errors());
}

TEST(AirPlaySessionTest, SaysWhenTheReceiverForgotThePairing) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession::Options options = PersistentOptions();
  options.receiver_pairing_id = FakeAirPlayReceiver::PairingId();
  options.receiver_public_key = FakeAirPlayReceiver::PublicKey();
  AirPlaySession session(options);
  QSignalSpy forgotten(&session, &AirPlaySession::PairingForgotten);
  EXPECT_FALSE(Open(&session, receiver.device()).set_up);
  EXPECT_EQ(1, forgotten.count());
}

// An Apple TV shows a PIN, which the user types in.
TEST(AirPlaySessionTest, PairsWithAPinTheReceiverShows) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);
  receiver.set_pin("4821");

  AirPlaySession::Options options = PersistentOptions();
  options.show_pin = true;
  AirPlaySession session(options);
  QObject::connect(&session, &AirPlaySession::PinShown,
                   [&session]() { session.SubmitPin("4821"); });
  EXPECT_TRUE(Open(&session, receiver.device()).set_up);
  EXPECT_EQ(QStringList(), receiver.errors());
  EXPECT_TRUE(receiver.senders().contains("02:11:22:33:44:55"));
}

TEST(AirPlaySessionTest, SaysWhenThePinIsWrong) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);
  receiver.set_pin("4821");

  AirPlaySession::Options options = PersistentOptions();
  options.show_pin = true;
  AirPlaySession session(options);
  QObject::connect(&session, &AirPlaySession::PinShown,
                   [&session]() { session.SubmitPin("1111"); });
  const Result result = Open(&session, receiver.device());
  EXPECT_TRUE(result.password_needed);
  EXPECT_TRUE(result.password_wrong);
}

}  // namespace

TEST(AirPlayDeviceTest, ParsesARaopService) {
  AirPlayDevice device;
  ASSERT_TRUE(device.ParseRaopService(
      "E64360C0424C@zarniwoop",
      {"am=AirPort10,115", "ft=0x77,0x1", "sf=0x4", "pw=true", "et=0,4"}));
  EXPECT_EQ("E6:43:60:C0:42:4C", device.id);
  EXPECT_EQ("zarniwoop", device.name);
  EXPECT_EQ("AirPort10,115", device.model);
  EXPECT_EQ(Q_UINT64_C(0x100000077), device.features);
  EXPECT_TRUE(device.password_required());
  EXPECT_TRUE(device.airplay1);
  EXPECT_TRUE(device.plain_audio);
}

TEST(AirPlayDeviceTest, RaopServiceMayWantEncryptedAudio) {
  AirPlayDevice device;
  ASSERT_TRUE(device.ParseRaopService("E64360C0424C@Kitchen", {"et=1"}));
  EXPECT_FALSE(device.plain_audio);
  EXPECT_FALSE(device.password_required());
}

TEST(AirPlayDeviceTest, RaopServiceNeedsAMac) {
  AirPlayDevice device;
  EXPECT_FALSE(device.ParseRaopService("Kitchen", {}));
  EXPECT_FALSE(device.ParseRaopService("E643@Kitchen", {}));
}

// 23 bits of header, 352 stereo frames of 16 bits, and a 3-bit end tag.
TEST(AirPlayStreamTest, UncompressedAlac) {
  QByteArray pcm = Tone(352);
  // The first left sample, which the tone starts at 0.
  pcm[0] = char(0xFF);
  pcm[1] = char(0x7F);
  const QByteArray frame = AirPlayStream::UncompressedAlac(pcm);
  EXPECT_EQ((23 + 352 * 2 * 16 + 3 + 7) / 8, frame.size());
  // Channel pair, nothing shifted or partial, escaped; then 0x7FFF from the
  // 24th bit: 0, then ones.
  EXPECT_EQ(QByteArray::fromHex("200002FF"), frame.left(4));
}

AirPlaySession::Options AirPlay1Options(FakeAirPlayReceiver* receiver) {
  receiver->set_airplay1(true);
  return FastOptions();
}

TEST(AirPlaySessionTest, StreamsAudioWithAirPlay1) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(AirPlay1Options(&receiver));
  ASSERT_TRUE(OpenReady(&session, receiver.device()));
  EXPECT_TRUE(session.airplay1());
  EXPECT_FALSE(receiver.paired());

  const QByteArray tone = Tone(352 * 6);
  session.stream()->Write(tone.constData(), tone.size());
  WaitFor([&receiver]() { return receiver.audio_packets() >= 6; });
  EXPECT_EQ(tone, receiver.audio());
  EXPECT_GE(receiver.sync_packets(), 1);
  EXPECT_GE(receiver.timing_replies(), 1);
  EXPECT_EQ(QStringList({"ANNOUNCE", "SETUP", "RECORD"}), receiver.requests());
  EXPECT_EQ(QStringList(), receiver.errors());
}

TEST(AirPlaySessionTest, SetsTheVolumeFlushesAndTearsDownWithAirPlay1) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(AirPlay1Options(&receiver));
  session.SetVolume(50);
  ASSERT_TRUE(OpenReady(&session, receiver.device()));
  WaitFor([&receiver]() { return !receiver.volume().isEmpty(); });
  EXPECT_EQ("-6.020600", receiver.volume());

  session.Flush();
  WaitFor([&receiver]() { return receiver.flushes() == 1; });
  EXPECT_EQ(1, receiver.flushes());

  session.Close();
  WaitFor([&receiver]() { return receiver.torn_down(); });
  EXPECT_TRUE(receiver.torn_down());
  EXPECT_EQ(QStringList(), receiver.errors());
}

TEST(AirPlaySessionTest, WaitsForTheReceiversUserToAllowAirPlay1) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_on_prompt([&receiver](const QString&) {
    QTimer::singleShot(1200, &receiver, [&receiver]() { receiver.Allow(); });
  });

  AirPlaySession session(AirPlay1Options(&receiver));
  QSignalSpy waiting(&session, &AirPlaySession::WaitingForApproval);
  EXPECT_TRUE(Open(&session, receiver.device()).set_up);
  EXPECT_EQ(1, receiver.prompts());
  EXPECT_EQ(1, waiting.count());
}

TEST(AirPlaySessionTest, SaysWhenTheReceiversUserDoesntAllowAirPlay1) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_on_prompt([&receiver](const QString&) {
    QTimer::singleShot(1200, &receiver, [&receiver]() { receiver.Deny(); });
  });

  AirPlaySession session(AirPlay1Options(&receiver));
  const Result result = Open(&session, receiver.device());
  EXPECT_FALSE(result.set_up);
  EXPECT_EQ("Fake didn't allow Clementine on test to play there",
            result.not_allowed);
}

// An AirPlay 2 receiver refuses AirPlay 1's ANNOUNCE until the sender pairs.
TEST(AirPlaySessionTest, PairsWhenAirPlay1IsRefused) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession session(AirPlay1Options(&receiver));
  receiver.set_airplay1_pairing(true);
  ASSERT_TRUE(OpenReady(&session, receiver.device()));
  EXPECT_TRUE(receiver.paired());
  EXPECT_EQ(QStringList({"ANNOUNCE", "POST /pair-setup", "POST /pair-setup",
                         "ANNOUNCE", "SETUP", "RECORD"}),
            receiver.requests());

  const QByteArray tone = Tone(352 * 2);
  session.stream()->Write(tone.constData(), tone.size());
  WaitFor([&receiver]() { return receiver.audio_packets() >= 2; });
  EXPECT_EQ(tone, receiver.audio());
  EXPECT_EQ(QStringList(), receiver.errors());
}

TEST(AirPlaySessionTest, PairsWithAPasswordWhenAirPlay1IsRefused) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);
  receiver.SetPassword("foobar");

  AirPlaySession::Options options = AirPlay1Options(&receiver);
  receiver.set_airplay1_pairing(true);
  options.password = "wrong";
  Result result = Pair(receiver.device(), options);
  EXPECT_TRUE(result.password_needed);
  EXPECT_TRUE(result.password_wrong);

  options.password = "foobar";
  EXPECT_TRUE(Pair(receiver.device(), options).set_up);
}

// As an AirPort Express asks for one.
TEST(AirPlaySessionTest, AnswersAirPlay1sDigestChallenge) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);
  receiver.SetPassword("foobar");

  AirPlaySession::Options options = AirPlay1Options(&receiver);
  // It said so as it was advertised.
  Result result = Pair(receiver.device(), options);
  EXPECT_TRUE(result.password_needed);
  EXPECT_FALSE(result.password_wrong);
  EXPECT_EQ(QStringList(), receiver.requests());

  options.password = "wrong";
  result = Pair(receiver.device(), options);
  EXPECT_TRUE(result.password_needed);
  EXPECT_TRUE(result.password_wrong);

  options.password = "foobar";
  AirPlaySession session(options);
  ASSERT_TRUE(OpenReady(&session, receiver.device()));
  session.SetVolume(100);
  session.Flush();
  WaitFor([&receiver]() { return receiver.flushes() == 1; });
  EXPECT_EQ("0.000000", receiver.volume());
  EXPECT_EQ(QStringList(), receiver.errors());
}

TEST(AirPlaySessionTest, ForcesAirPlay1) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());
  receiver.set_ask_to_allow(false);

  AirPlaySession::Options options = AirPlay1Options(&receiver);
  options.force_airplay1 = true;
  AirPlayDevice device = receiver.device();
  device.airplay1 = false;
  AirPlaySession session(options);
  ASSERT_TRUE(OpenReady(&session, device));
  EXPECT_TRUE(session.airplay1());
}

TEST(AirPlaySessionTest, SaysWhenAirPlay1AudioMustBeEncrypted) {
  FakeAirPlayReceiver receiver;
  ASSERT_TRUE(receiver.Listen());

  AirPlayDevice device = receiver.device();
  device.airplay1 = true;
  device.plain_audio = false;
  const Result result = Pair(device, FastOptions());
  EXPECT_EQ(
      "Fake only takes encrypted AirPlay 1 audio, which Clementine can't send",
      result.error);
}
