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

// The sender against a fake receiver that behaves as a Mac was seen to.

#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <QtTest>

#include "audiosource.h"
#include "discovery.h"
#include "fakereceiver.h"
#include "session.h"

using namespace airplay;

namespace {

// A tenth of a second of tone.
const qint64 kToneFrames = kSampleRate / 10;

struct Result {
  bool finished = false;
  QString error;
  qint64 msec = 0;
};

// Options that don't keep a test waiting.
Session::Options FastOptions() {
  Session::Options options;
  options.name = "Clementine on test";
  options.connect_timeout_msec = 1000;
  options.request_timeout_msec = 1000;
  options.approval_timeout_msec = 2000;
  options.play_out_frames = 0;
  return options;
}

// Plays the tone to |host| and waits for the session to end.
Result PlayTo(const QHostAddress& host, quint16 port,
              const Session::Options& options) {
  Result result;
  QEventLoop loop;
  Session session(options, std::make_unique<ToneSource>(kToneFrames));
  QObject::connect(&session, &Session::Finished, &loop, [&]() {
    result.finished = true;
    loop.quit();
  });
  QObject::connect(&session, &Session::Failed, &loop,
                   [&](const QString& error) {
                     result.error = error;
                     loop.quit();
                   });
  // A backstop, so a hang fails the test rather than blocking it.
  QTimer::singleShot(10000, &loop, [&]() {
    result.error = "the test timed out";
    loop.quit();
  });
  QElapsedTimer timer;
  timer.start();
  session.Start(host, port);
  loop.exec();
  result.msec = timer.elapsed();
  return result;
}

Result Play(const FakeReceiver& receiver, const Session::Options& options) {
  return PlayTo(receiver.address(), receiver.port(), options);
}

}  // namespace

class ReceiverTest : public QObject {
  Q_OBJECT

 private slots:
  void AirPlay2PlaysTheTone() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    connect(&receiver, &FakeReceiver::Prompted, &receiver,
            &FakeReceiver::Allow);

    const Result result = Play(receiver, FastOptions());
    QVERIFY2(result.finished, qPrintable(result.error));
    QVERIFY(receiver.torn_down());
    QCOMPARE(receiver.errors(), QStringList());

    // It's the tone, decrypted with the key the sender chose.
    QVERIFY(receiver.audio().startsWith(Tone(kToneFrames)));
    // The receiver asked for the time before it answered the stream's SETUP,
    // and the sender answered the event channel.
    QCOMPARE(receiver.setups_after_timing(), 1);
    QVERIFY(receiver.sync_packets() >= 1);
    QCOMPARE(receiver.event_replies(), 1);
    QCOMPARE(receiver.requests(),
             QStringList({"GET /info", "POST /pair-pin-start",
                          "POST /pair-setup", "POST /pair-setup", "SETUP",
                          "SETUP", "RECORD", "FLUSH", "TEARDOWN"}));
  }

  void AirPlay1PlaysTheTone() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    connect(&receiver, &FakeReceiver::Prompted, &receiver,
            &FakeReceiver::Allow);

    Session::Options options = FastOptions();
    options.version = Version::kAirPlay1;
    const Result result = Play(receiver, options);
    QVERIFY2(result.finished, qPrintable(result.error));
    QCOMPARE(receiver.errors(), QStringList());
    QVERIFY(receiver.audio().startsWith(Tone(kToneFrames)));
    QCOMPARE(receiver.setups_after_timing(), 1);
    // AirPlay 1 asks to be allowed at ANNOUNCE.
    QCOMPARE(receiver.prompts(), 1);
  }

  void VolumeIsSent() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    connect(&receiver, &FakeReceiver::Prompted, &receiver,
            &FakeReceiver::Allow);
    Session::Options options = FastOptions();
    options.volume = 50;
    QVERIFY(Play(receiver, options).finished);
    QVERIFY(receiver.requests().contains("SET_PARAMETER"));
    QCOMPARE(receiver.errors(), QStringList());
  }

  void PasswordIsDetectedFromTxt() {
    FakeReceiver mac(FakeReceiver::Options{});
    Device device;
    device.txt = mac.txt();
    QVERIFY(!device.password_required());
    mac.SetPassword("foobar");
    device.txt = mac.txt();
    QCOMPARE(device.txt.value("flags"), QString("0x284"));
    QVERIFY(device.password_required());

    FakeReceiver::Options legacy_options;
    legacy_options.legacy_digest = true;
    FakeReceiver legacy(legacy_options);
    legacy.SetPassword("foobar");
    device.txt = legacy.txt();
    QVERIFY(device.password_required());
  }

  void NoPasswordFailsBeforePairing() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    receiver.SetPassword("foobar");

    const Result result = Play(receiver, FastOptions());
    QVERIFY(!result.finished);
    QVERIFY2(result.error.contains("requires a password"),
             qPrintable(result.error));
    QCOMPARE(receiver.requests(), QStringList({"GET /info"}));
  }

  void PasswordPlays() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    receiver.SetPassword("foobar");
    connect(&receiver, &FakeReceiver::Prompted, &receiver,
            &FakeReceiver::Allow);

    const Result result = Play(receiver, WithPassword("foobar"));
    QVERIFY2(result.finished, qPrintable(result.error));
    QVERIFY(receiver.audio().startsWith(Tone(kToneFrames)));
    QCOMPARE(receiver.errors(), QStringList());
  }

  void WrongPasswordIsReported() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    receiver.SetPassword("foobar");
    connect(&receiver, &FakeReceiver::Prompted, &receiver,
            &FakeReceiver::Allow);

    const Result result = Play(receiver, WithPassword("wrong"));
    QVERIFY(!result.finished);
    QVERIFY2(result.error.contains("wrong password"), qPrintable(result.error));
    QVERIFY(!receiver.requests().contains("SETUP"));
  }

  void AirPlay1WithPasswordPairsFirst() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    receiver.SetPassword("foobar");
    connect(&receiver, &FakeReceiver::Prompted, &receiver,
            &FakeReceiver::Allow);

    Session::Options options = WithPassword("foobar");
    options.version = Version::kAirPlay1;
    const Result result = Play(receiver, options);
    QVERIFY2(result.finished, qPrintable(result.error));
    QVERIFY(receiver.audio().startsWith(Tone(kToneFrames)));
    // Refused, paired, then accepted on the encrypted connection.
    const QStringList requests = receiver.requests();
    QCOMPARE(requests.mid(1, 5),
             QStringList({"ANNOUNCE", "POST /pair-pin-start",
                          "POST /pair-setup", "POST /pair-setup", "ANNOUNCE"}));
  }

  void LegacyDigestPlays() {
    FakeReceiver::Options receiver_options;
    receiver_options.legacy_digest = true;
    FakeReceiver receiver(receiver_options);
    QVERIFY(receiver.Listen());
    receiver.SetPassword("foobar");
    connect(&receiver, &FakeReceiver::Prompted, &receiver,
            &FakeReceiver::Allow);

    Session::Options options = WithPassword("foobar");
    options.version = Version::kAirPlay1;
    const Result result = Play(receiver, options);
    QVERIFY2(result.finished, qPrintable(result.error));
    QVERIFY(receiver.audio().startsWith(Tone(kToneFrames)));
    // GET /info is challenged, then everything carries a digest.
    QCOMPARE(receiver.requests().mid(0, 2),
             QStringList({"GET /info", "GET /info"}));
  }

  void LegacyDigestWrongPasswordIsReported() {
    FakeReceiver::Options receiver_options;
    receiver_options.legacy_digest = true;
    FakeReceiver receiver(receiver_options);
    QVERIFY(receiver.Listen());
    receiver.SetPassword("foobar");

    Session::Options options = WithPassword("wrong");
    options.version = Version::kAirPlay1;
    const Result result = Play(receiver, options);
    QVERIFY(!result.finished);
    QVERIFY2(result.error.contains("wrong password"), qPrintable(result.error));
  }

  // A Mac asks once per sender name, not per device id.
  void PromptsOncePerName() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    connect(&receiver, &FakeReceiver::Prompted, &receiver,
            &FakeReceiver::Allow);

    Session::Options options = FastOptions();
    options.device_id = "C1E0C1E0C1E00001";
    QVERIFY(Play(receiver, options).finished);
    QCOMPARE(receiver.prompts(), 1);

    QVERIFY(Play(receiver, options).finished);
    QCOMPARE(receiver.prompts(), 1);

    options.device_id = "C1E0C1E0C1E00002";
    QVERIFY(Play(receiver, options).finished);
    QCOMPARE(receiver.prompts(), 1);

    options.name = "Clementine test";
    QVERIFY(Play(receiver, options).finished);
    QCOMPARE(receiver.prompts(), 2);
  }

  void SettingAPasswordForgetsAllowedNames() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    connect(&receiver, &FakeReceiver::Prompted, &receiver,
            &FakeReceiver::Allow);
    QVERIFY(Play(receiver, FastOptions()).finished);
    QCOMPARE(receiver.prompts(), 1);

    receiver.SetPassword("foobar");
    QVERIFY(Play(receiver, WithPassword("foobar")).finished);
    QCOMPARE(receiver.prompts(), 2);
  }

  // The prompt can take longer than other requests may.
  void ApprovalOutlastsTheRequestTimeout() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());
    connect(&receiver, &FakeReceiver::Prompted, &receiver, [&receiver]() {
      QTimer::singleShot(300, &receiver, &FakeReceiver::Allow);
    });

    Session::Options options = FastOptions();
    options.request_timeout_msec = 100;
    options.approval_timeout_msec = 1000;
    const Result result = Play(receiver, options);
    QVERIFY2(result.finished, qPrintable(result.error));
    QVERIFY(result.msec >= 300);
  }

  void UnansweredPromptTimesOut() {
    FakeReceiver receiver(FakeReceiver::Options{});
    QVERIFY(receiver.Listen());

    Session::Options options = FastOptions();
    options.approval_timeout_msec = 300;
    const Result result = Play(receiver, options);
    QVERIFY(!result.finished);
    QVERIFY2(result.error.contains("no response from the receiver in 0.3 s"),
             qPrintable(result.error));
    QCOMPARE(receiver.prompts(), 1);
  }

  // TEST-NET-1 (RFC 5737) never answers.
  void UnansweredConnectTimesOut() {
    Session::Options options = FastOptions();
    options.connect_timeout_msec = 200;
    const Result result = PlayTo(QHostAddress("192.0.2.1"), 7000, options);
    QVERIFY(!result.finished);
    if (result.error.contains("unreachable", Qt::CaseInsensitive)) {
      QSKIP("This machine has no route to TEST-NET-1");
    }
    QVERIFY2(result.error.contains("couldn't connect to 192.0.2.1:7000 in "
                                   "0.2 s"),
             qPrintable(result.error));
  }

 private:
  static QByteArray Tone(qint64 frames) {
    ToneSource source(frames);
    QByteArray tone(frames * kFrameSize, '\0');
    tone.resize(source.Read(tone.data(), tone.size()));
    return tone;
  }

  static Session::Options WithPassword(const QString& password) {
    Session::Options options = FastOptions();
    options.pin = password;
    options.has_password = true;
    return options;
  }
};

QTEST_GUILESS_MAIN(ReceiverTest)
#include "receivertests.moc"
