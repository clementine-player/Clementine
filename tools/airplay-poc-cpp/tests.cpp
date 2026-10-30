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

#include <QtTest>

#include "bplist.h"
#include "crypto.h"
#include "logging.h"
#include "srp.h"
#include "tlv8.h"

Q_LOGGING_CATEGORY(airplay_log, "airplay", QtInfoMsg)

using namespace airplay;

class AirPlayTest : public QObject {
  Q_OBJECT

 private slots:
  // RFC 8439 section 2.8.2.
  void ChaCha20Poly1305() {
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
    QCOMPARE(sealed.toHex(), expected.toHex());

    QByteArray opened;
    QVERIFY(Open(key, nonce, aad, sealed, &opened));
    QCOMPARE(opened, plain);

    QByteArray tampered = sealed;
    tampered[0] = char(tampered[0] ^ 1);
    QVERIFY(!Open(key, nonce, aad, tampered, &opened));
  }

  // Checked against Python's cryptography package.
  void Hkdf() {
    QByteArray secret;
    for (int i = 0; i < 64; ++i) secret.append(char(i));
    QCOMPARE(HkdfSha512("Control-Salt", "Control-Write-Encryption-Key", secret)
                 .toHex(),
             QByteArray("5a6cb19bcbe7d4df2dd8279f39562f7fae2dbf73eb5a4f98849c24"
                        "5c82b2fe96"));
  }

  void Nonce() {
    QCOMPARE(airplay::Nonce(0x0102).toHex(),
             QByteArray("000000000201000000000000"));
  }

  void FrameCipherRoundTrip() {
    const QByteArray a(32, 'a');
    const QByteArray b(32, 'b');
    FrameCipher sender(a, b);
    FrameCipher receiver(b, a);
    // Over 1024 bytes, so it's split into two frames.
    const QByteArray message(1500, 'x');
    QByteArray wire = sender.Encrypt(message);
    wire += sender.Encrypt("more");
    QCOMPARE(wire.size(), 1500 + 4 + 2 * 3 + 16 * 3);

    // Arriving in pieces.
    QByteArray plain;
    QVERIFY(receiver.Decrypt(wire.left(100), &plain));
    QVERIFY(plain.isEmpty());
    QVERIFY(receiver.Decrypt(wire.mid(100), &plain));
    QCOMPARE(plain, message + "more");
  }

  // The same inputs give the same results as the Rust proof of concept,
  // which was checked against srptools. The salt's leading zero byte is
  // dropped before hashing, as srptools does.
  void Srp() {
    const QByteArray a(QByteArray::fromHex(QByteArray(64, '1')));
    const QByteArray salt =
        QByteArray::fromHex("00a1b2c3d4e5f60718293a4b5c6d7e8f");
    const QByteArray b_hash = Sha512("b").toHex();
    const QByteArray b = QByteArray::fromHex(
        "7f" + (b_hash + b_hash + b_hash + b_hash + b_hash + b_hash).mid(2));

    const SrpClient::Result r = SrpClient(a).Process("3939", salt, b);
    QCOMPARE(
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
    QCOMPARE(
        r.proof.toHex(),
        QByteArray(
            "dcffaf5aa2754af8645619e632a9cc034c7f8d5c6095120893a6bff857f89887"
            "971aa93102c75d77e80553b98eb7351fc98bcf01a840d52a1806d83607b39eb"
            "1"));
    QCOMPARE(
        r.key.toHex(),
        QByteArray(
            "eb3b50c2e3514a00ec9c72d3bf32d99f2145de13f86a74cdf0b7a49a38477051"
            "4011c96ee111133bf8885a879360fbee39c58c0d74d6cefb0a52277419451b9"
            "0"));
  }

  void Tlv8() {
    const QByteArray big(300, 'k');
    const QByteArray wire = tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 3)},
                                         {tlv8::kPublicKey, big},
                                         {tlv8::kProof, QByteArray()}});
    QCOMPARE(wire.size(), 3 + 2 + 255 + 2 + 45 + 2);
    const tlv8::Items items = tlv8::Read(wire);
    QCOMPARE(items.size(), 3);
    QCOMPARE(tlv8::Get(items, tlv8::kPublicKey), big);
    QCOMPARE(tlv8::Get(items, tlv8::kSeqNo), QByteArray(1, 3));
    QVERIFY(tlv8::Get(items, tlv8::kSalt).isNull());
  }

  void BinaryPlistRoundTrip() {
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
    QVERIFY(data.startsWith("bplist00"));
    QVariant read;
    QVERIFY(bplist::Read(data, &read));
    const QVariantMap s =
        read.toMap().value("streams").toList().value(0).toMap();
    QCOMPARE(s.value("audioFormat").toLongLong(), 0x800);
    QCOMPARE(s.value("shk").toByteArray(), QByteArray(32, '\x01'));
    QCOMPARE(s.value("isMedia").toBool(), true);
    QCOMPARE(s.value("latencyMax").toLongLong(), 88200);
    QCOMPARE(s.value("big").toLongLong(), 0x123456789LL);
    QCOMPARE(s.value("negative").toLongLong(), -1);
    QCOMPARE(s.value("ratio").toDouble(), 0.5);
    QCOMPARE(s.value("name").toString(),
             QString("Clementine on a very long name indeed"));
    QCOMPARE(s.value("unicode").toString(), QString::fromUtf8("Caf\xc3\xa9"));
  }

  // Written by Python's plistlib, which uses 1-byte refs and offsets.
  void BinaryPlistFromPython() {
    const QByteArray data = QByteArray::fromHex(
        "62706c6973743030d3010203040506596576656e74506f7274546e616d655a74"
        "696d696e67506f727411ddad597a61726e69776f6f701000080f191e292c3600"
        "00000000000101000000000000000700000000000000000000000000000038");
    QVariant read;
    QVERIFY(bplist::Read(data, &read));
    const QVariantMap map = read.toMap();
    QCOMPARE(map.value("eventPort").toLongLong(), 56749);
    QCOMPARE(map.value("timingPort").toLongLong(), 0);
    QCOMPARE(map.value("name").toString(), QString("zarniwoop"));
  }

  void BinaryPlistRejectsGarbage() {
    QVariant read;
    QVERIFY(!bplist::Read("bplist00", &read));
    QVERIFY(!bplist::Read(QByteArray(64, '\xff'), &read));
    QByteArray truncated = bplist::Write(QVariantMap{{"a", 1}});
    truncated[truncated.size() - 1] = char(0xff);  // Table offset past the end
    QVERIFY(!bplist::Read(truncated, &read));
  }
};

QTEST_GUILESS_MAIN(AirPlayTest)
#include "tests.moc"
