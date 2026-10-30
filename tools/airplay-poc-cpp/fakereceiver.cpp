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

#include "fakereceiver.h"

#include <QCryptographicHash>
#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QtEndian>

#include "bplist.h"
#include "bytes.h"
#include "discovery.h"
#include "mini-gmp.h"
#include "tlv8.h"

namespace airplay {

namespace {

const char kN[] =
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA6"
    "3B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F2411"
    "7C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F"
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08"
    "CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9"
    "DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AAAC42DAD33170D"
    "04507A33A85521ABDF1CBA64ECFB850458DBEF0A8AEA71575D060C7DB3970F85A6E1E4C7"
    "ABF5AE8CDB0933D71E8C94E04A25619DCEE3D2261AD2EE6BF12FFA06D98A0864D8760273"
    "3EC86A64521F2B18177B200CBBE117577A615D6C770988C0BAD946E208E24FA074E5AB31"
    "43DB5BFCE0FD108E4B82D120A93AD2CAFFFFFFFFFFFFFFFF";
const char kUsername[] = "Pair-Setup";
const char kTransientPin[] = "3939";
// 352 stereo frames.
const int kSamplesPerPacket = 352 * 2;

// An mpz_t that frees itself, for the server side of SRP.
class BigInt {
 public:
  BigInt() { mpz_init(v_); }
  explicit BigInt(const QByteArray& big_endian) {
    mpz_init(v_);
    mpz_import(v_, big_endian.size(), 1, 1, 1, 0, big_endian.constData());
  }
  ~BigInt() { mpz_clear(v_); }
  BigInt(const BigInt&) = delete;
  BigInt& operator=(const BigInt&) = delete;

  mpz_ptr get() { return v_; }
  mpz_srcptr get() const { return v_; }

  QByteArray Bytes() const {
    QByteArray out((mpz_sizeinbase(v_, 2) + 7) / 8, '\0');
    size_t written = 0;
    mpz_export(out.data(), &written, 1, 1, 1, 0, v_);
    return out.left(qMax<size_t>(written, 1));
  }

  QByteArray Padded(int size) const {
    const QByteArray b = Bytes();
    return QByteArray(qMax(0, size - int(b.size())), '\0') + b;
  }

 private:
  mpz_t v_;
};

QByteArray RandomBytes(int size) {
  QByteArray out(size, '\0');
  QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(out.data()),
                                        size / sizeof(quint32));
  return out;
}

QByteArray Md5Hex(const QByteArray& data) {
  return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

QByteArray ReasonPhrase(int code) {
  switch (code) {
    case 200:
      return "OK";
    case 401:
      return "Unauthorized";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    default:
      return "Not Implemented";
  }
}

// key="value", key2=value2 to a map.
QMap<QByteArray, QByteArray> ParseParameters(const QByteArray& list) {
  QMap<QByteArray, QByteArray> out;
  for (const QByteArray& part : list.split(',')) {
    const int eq = part.indexOf('=');
    if (eq < 0) continue;
    QByteArray value = part.mid(eq + 1).trimmed();
    if (value.startsWith('"') && value.endsWith('"')) {
      value = value.mid(1, value.size() - 2);
    }
    out[part.left(eq).trimmed()] = value;
  }
  return out;
}

class BitReader {
 public:
  explicit BitReader(const QByteArray& data) : data_(data) {}

  quint32 Get(int bits) {
    quint32 value = 0;
    for (int i = 0; i < bits; ++i, ++pos_) {
      const int byte = pos_ / 8;
      const int bit =
          byte < data_.size() ? (quint8(data_[byte]) >> (7 - pos_ % 8)) & 1 : 0;
      value = (value << 1) | bit;
    }
    return value;
  }

 private:
  const QByteArray data_;
  int pos_ = 0;
};

}  // namespace

QByteArray FakeReceiver::Request::Header(const QByteArray& name) const {
  for (const auto& h : headers) {
    if (h.first.compare(name, Qt::CaseInsensitive) == 0) return h.second;
  }
  return QByteArray();
}

FakeReceiver::FakeReceiver(const Options& options, QObject* parent)
    : QObject(parent),
      options_(options),
      server_(new QTcpServer(this)),
      event_server_(new QTcpServer(this)),
      timing_(new QUdpSocket(this)),
      control_(new QUdpSocket(this)),
      audio_socket_(new QUdpSocket(this)) {
  connect(server_, SIGNAL(newConnection()), SLOT(NewConnection()));
  connect(event_server_, SIGNAL(newConnection()), SLOT(NewEventConnection()));
  connect(timing_, SIGNAL(readyRead()), SLOT(TimingReadyRead()));
  connect(control_, SIGNAL(readyRead()), SLOT(ControlReadyRead()));
  connect(audio_socket_, SIGNAL(readyRead()), SLOT(AudioReadyRead()));
}

FakeReceiver::~FakeReceiver() {}

bool FakeReceiver::Listen() {
  return server_->listen(address()) && event_server_->listen(address()) &&
         timing_->bind(address()) && control_->bind(address()) &&
         audio_socket_->bind(address());
}

quint16 FakeReceiver::port() const { return server_->serverPort(); }

void FakeReceiver::SetPassword(const QString& password) {
  password_ = password;
  allowed_names_.clear();
}

QMap<QString, QString> FakeReceiver::txt() const {
  QMap<QString, QString> txt;
  quint32 flags = options_.status_flags;
  if (options_.legacy_digest) {
    txt["pw"] = password_.isEmpty() ? "false" : "true";
  } else {
    if (!password_.isEmpty()) flags |= kStatusPasswordRequired;
    txt["features"] = "0x4A7FCFD5,0x38174FDE";
  }
  txt["flags"] = "0x" + QString::number(flags, 16);
  txt["model"] = "Mac14,2";
  return txt;
}

void FakeReceiver::Allow() {
  if (!prompting_) return;
  prompting_ = false;
  allowed_names_ << QString::fromUtf8(prompted_name_);
  ProcessRequests();
}

void FakeReceiver::NewConnection() {
  // One sender at a time: a new connection replaces the last.
  if (client_) client_->deleteLater();
  client_ = server_->nextPendingConnection();
  buffer_.clear();
  cipher_.reset();
  paired_ = false;
  held_.reset();
  prompting_ = false;
  waiting_for_time_ = false;
  sender_timing_port_ = 0;
  audio_key_.clear();
  audio_nonces_.clear();
  connect(client_, SIGNAL(readyRead()), SLOT(ReadyRead()));
}

void FakeReceiver::ReadyRead() {
  const QByteArray data = client_->readAll();
  if (cipher_) {
    if (!cipher_->Decrypt(data, &buffer_)) {
      errors_ << "a request didn't decrypt";
      client_->abort();
      return;
    }
  } else {
    buffer_ += data;
  }
  ProcessRequests();
}

void FakeReceiver::ProcessRequests() {
  while (client_) {
    if (!held_) {
      Request request;
      if (!ParseRequest(&request)) return;
      // RTSP requests are logged by method: their URIs name the sender's
      // address and a random session.
      requests_ << QString::fromUtf8(request.uri.startsWith("rtsp://")
                                         ? request.method
                                         : request.method + " " + request.uri);
      held_ = request;
    }
    if (!Handle(*held_)) return;
    held_.reset();
  }
}

bool FakeReceiver::ParseRequest(Request* request) {
  const int header_end = buffer_.indexOf("\r\n\r\n");
  if (header_end < 0) return false;
  const QList<QByteArray> lines = buffer_.left(header_end).split('\n');
  const QList<QByteArray> first = lines.value(0).trimmed().split(' ');
  Request r;
  r.method = first.value(0);
  r.uri = first.value(1);
  for (int i = 1; i < lines.size(); ++i) {
    const int colon = lines[i].indexOf(':');
    if (colon < 0) continue;
    r.headers.append(
        {lines[i].left(colon).trimmed(), lines[i].mid(colon + 1).trimmed()});
  }
  const int length = r.Header("Content-Length").toInt();
  if (buffer_.size() < header_end + 4 + length) return false;
  r.body = buffer_.mid(header_end + 4, length);
  buffer_.remove(0, header_end + 4 + length);
  *request = r;
  return true;
}

void FakeReceiver::Respond(const Request& request, int code,
                           const QByteArray& body, const Headers& headers) {
  QByteArray message = "RTSP/1.0 " + QByteArray::number(code) + " " +
                       ReasonPhrase(code) + "\r\n";
  message += "CSeq: " + request.Header("CSeq") + "\r\n";
  message += "Server: AirTunes/980.77.5\r\n";
  for (const auto& h : headers) message += h.first + ": " + h.second + "\r\n";
  message += "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n";
  message += body;
  client_->write(cipher_ ? cipher_->Encrypt(message) : message);
}

bool FakeReceiver::Handle(const Request& request) {
  if (options_.legacy_digest && !password_.isEmpty() && !CheckDigest(request)) {
    return true;
  }

  const QByteArray& method = request.method;
  if (method == "GET" && request.uri == "/info") {
    QVariantMap info;
    info["name"] = "fake";
    info["model"] = "Mac14,2";
    info["sourceVersion"] = "980.77.5";
    const QString flags = txt().value("flags");
    info["statusFlags"] = flags.mid(2).toUInt(nullptr, 16);
    Respond(request, 200, bplist::Write(info));
  } else if (method == "POST" && request.uri == "/pair-pin-start") {
    if (options_.legacy_digest) {
      Respond(request, 404);
    } else if (!CheckAllowed(request)) {
      return false;
    } else {
      Respond(request, 200);
    }
  } else if (method == "POST" && request.uri == "/pair-setup") {
    if (options_.legacy_digest) {
      Respond(request, 404);
    } else {
      PairSetup(request);
    }
  } else if (method == "ANNOUNCE") {
    // A Mac with a password wants AirPlay 1 senders to pair first.
    if (!options_.legacy_digest && !password_.isEmpty() && !paired_) {
      Respond(request, 403);
    } else if (!paired_ && !CheckAllowed(request)) {
      return false;
    } else {
      Respond(request, 200);
    }
  } else if (method == "SETUP") {
    return Setup(request);
  } else if (method == "RECORD") {
    Respond(request, 200);
    SendEvent();
  } else if (method == "FLUSH") {
    Respond(request, 200, {}, {{"RTP-Info", "rtptime=0"}});
  } else if (method == "SET_PARAMETER") {
    if (!request.body.startsWith("volume: ")) {
      errors_ << "SET_PARAMETER without a volume";
    }
    Respond(request, 200);
  } else if (method == "POST" && request.uri == "/feedback") {
    Respond(request, 200);
  } else if (method == "TEARDOWN") {
    Respond(request, 200);
    torn_down_ = true;
    emit TornDown();
  } else {
    Respond(request, 501);
  }
  return true;
}

bool FakeReceiver::CheckDigest(const Request& request) {
  const QByteArray authorization = request.Header("Authorization");
  if (authorization.startsWith("Digest ") && !digest_nonce_.isEmpty()) {
    const QMap<QByteArray, QByteArray> p =
        ParseParameters(authorization.mid(7));
    const QByteArray ha1 = Md5Hex(p.value("username") + ":" + p.value("realm") +
                                  ":" + password_.toUtf8());
    const QByteArray ha2 = Md5Hex(request.method + ":" + p.value("uri"));
    const QByteArray expected = Md5Hex(ha1 + ":" + digest_nonce_ + ":" + ha2);
    if (p.value("nonce") == digest_nonce_ &&
        p.value("response").compare(expected, Qt::CaseInsensitive) == 0) {
      return true;
    }
  }
  digest_nonce_ = RandomBytes(16).toHex();
  Respond(request, 401, {},
          {{"WWW-Authenticate",
            "Digest realm=\"raop\", nonce=\"" + digest_nonce_ + "\""}});
  return false;
}

bool FakeReceiver::CheckAllowed(const Request& request) {
  const QByteArray name = request.Header("X-Apple-Client-Name");
  if (!options_.ask_to_allow ||
      allowed_names_.contains(QString::fromUtf8(name))) {
    return true;
  }
  if (!prompting_) {
    prompting_ = true;
    prompted_name_ = name;
    ++prompts_;
    emit Prompted(QString::fromUtf8(name));
  }
  return false;
}

void FakeReceiver::PairSetup(const Request& request) {
  const tlv8::Items items = tlv8::Read(request.body);
  const QByteArray seq_item = tlv8::Get(items, tlv8::kSeqNo);
  const int seq = seq_item.isEmpty() ? 0 : quint8(seq_item[0]);

  BigInt n;
  mpz_set_str(n.get(), kN, 16);
  BigInt g;
  mpz_set_ui(g.get(), 5);
  const int n_size = n.Bytes().size();
  const QByteArray pin =
      password_.isEmpty() ? kTransientPin : password_.toUtf8();

  if (seq == 1) {
    srp_salt_ = RandomBytes(16);
    srp_salt_[0] = char(srp_salt_[0] | 0x80);  // No leading zero to drop
    srp_b_ = RandomBytes(32);
  }
  // v = g ^ H(s, H(I:P)), k = H(N, PAD(g))
  BigInt x(Sha512(srp_salt_ + Sha512(QByteArray(kUsername) + ":" + pin)));
  BigInt v;
  mpz_powm(v.get(), g.get(), x.get(), n.get());
  const BigInt k(Sha512(n.Bytes() + g.Padded(n_size)));
  const BigInt b(srp_b_);

  if (seq == 1) {
    // B = k * v + g ^ b
    BigInt gb;
    mpz_powm(gb.get(), g.get(), b.get(), n.get());
    BigInt b_public;
    mpz_mul(b_public.get(), k.get(), v.get());
    mpz_add(b_public.get(), b_public.get(), gb.get());
    mpz_mod(b_public.get(), b_public.get(), n.get());
    srp_b_public_ = b_public.Bytes();
    Respond(request, 200,
            tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 2)},
                         {tlv8::kSalt, srp_salt_},
                         {tlv8::kPublicKey, srp_b_public_}}));
    return;
  }

  // S = (A * v ^ u) ^ b, with u = H(PAD(A), PAD(B))
  const BigInt a(tlv8::Get(items, tlv8::kPublicKey));
  const BigInt b_public(srp_b_public_);
  const BigInt u(Sha512(a.Padded(n_size) + b_public.Padded(n_size)));
  BigInt s;
  mpz_powm(s.get(), v.get(), u.get(), n.get());
  mpz_mul(s.get(), s.get(), a.get());
  mpz_mod(s.get(), s.get(), n.get());
  mpz_powm(s.get(), s.get(), b.get(), n.get());
  const QByteArray key = Sha512(s.Bytes());

  const BigInt hn(Sha512(n.Bytes()));
  const BigInt hg(Sha512(g.Bytes()));
  BigInt hxor;
  mpz_xor(hxor.get(), hn.get(), hg.get());
  const QByteArray m1 = Sha512(hxor.Bytes() + Sha512(kUsername) + srp_salt_ +
                               a.Bytes() + b_public.Bytes() + key);
  if (m1 != tlv8::Get(items, tlv8::kProof)) {
    // As a Mac answers a wrong PIN: HTTP 200, with an authentication error.
    Respond(request, 200,
            tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 4)},
                         {tlv8::kError, QByteArray(1, 2)}}));
    return;
  }
  Respond(request, 200,
          tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 4)},
                       {tlv8::kProof, Sha512(a.Bytes() + m1 + key)}}));
  shared_key_ = key;
  cipher_.reset(new FrameCipher(
      HkdfSha512("Control-Salt", "Control-Read-Encryption-Key", key),
      HkdfSha512("Control-Salt", "Control-Write-Encryption-Key", key)));
  paired_ = true;
}

bool FakeReceiver::Setup(const Request& request) {
  QVariant body;
  bplist::Read(request.body, &body);
  const QVariantMap map = body.toMap();
  const bool stream = map.contains("streams");
  const bool airplay1 = !request.Header("Transport").isEmpty();

  if (!stream && !airplay1) {
    // AirPlay 2's first SETUP: where to ask for the time, and the event
    // channel. A Mac asks for the time straight away.
    sender_timing_port_ = map.value("timingPort").toUInt();
    SendTimingRequest();
    Respond(
        request, 200,
        bplist::Write(QVariantMap{{"eventPort", event_server_->serverPort()}}));
    return true;
  }

  if (airplay1) {
    const QMap<QByteArray, QByteArray> transport = [&request]() {
      QMap<QByteArray, QByteArray> out;
      for (const QByteArray& part : request.Header("Transport").split(';')) {
        const int eq = part.indexOf('=');
        if (eq > 0) out[part.left(eq)] = part.mid(eq + 1);
      }
      return out;
    }();
    sender_timing_port_ = transport.value("timing_port").toUShort();
  }

  // The SETUP that starts the stream isn't answered until the sender has
  // answered a request for the time, which stalled a sender that only
  // listened once streaming.
  if (!waiting_for_time_) {
    waiting_for_time_ = true;
    timing_replies_before_setup_ = timing_replies_;
    SendTimingRequest();
    return false;
  }
  if (timing_replies_ <= timing_replies_before_setup_) return false;
  waiting_for_time_ = false;
  ++setups_after_timing_;

  if (airplay1) {
    Respond(
        request, 200, {},
        {{"Transport", QString("RTP/AVP/UDP;unicast;mode=record;server_port=%1;"
                               "control_port=%2;timing_port=0")
                           .arg(audio_socket_->localPort())
                           .arg(control_->localPort())
                           .toUtf8()},
         {"Session", "1"}});
    return true;
  }

  audio_key_ =
      map.value("streams").toList().value(0).toMap().value("shk").toByteArray();
  if (audio_key_.size() != 32) errors_ << "shk isn't 32 bytes";
  for (const char* info :
       {"Events-Write-Encryption-Key", "Events-Read-Encryption-Key"}) {
    if (audio_key_ == HkdfSha512("Events-Salt", info, shared_key_)) {
      errors_ << QString("shk is the event channel's %1").arg(info);
    }
  }
  QVariantMap reply_stream{{"type", 0x60},
                           {"dataPort", audio_socket_->localPort()},
                           {"controlPort", control_->localPort()}};
  Respond(request, 200,
          bplist::Write(QVariantMap{{"streams", QVariantList{reply_stream}}}));
  return true;
}

void FakeReceiver::SendTimingRequest() {
  if (!sender_timing_port_) return;
  QByteArray packet;
  packet.append(char(0x80));
  packet.append(char(0xD2));
  packet += Bytes(qToBigEndian(quint16(7)));
  packet += QByteArray(20, '\0');
  packet += Bytes(qToBigEndian(quint64(timing_requests_ + 1)));
  timing_->writeDatagram(packet, address(), sender_timing_port_);
  ++timing_requests_;
}

void FakeReceiver::TimingReadyRead() {
  while (timing_->hasPendingDatagrams()) {
    const QByteArray data = timing_->receiveDatagram().data();
    if (data.size() >= 32 && (quint8(data[1]) & 0x7F) == 0x53) {
      ++timing_replies_;
    }
  }
  ProcessRequests();
}

void FakeReceiver::ControlReadyRead() {
  while (control_->hasPendingDatagrams()) {
    const QByteArray data = control_->receiveDatagram().data();
    if (data.size() >= 2 && (quint8(data[1]) & 0x7F) == 0x54) ++sync_packets_;
  }
}

void FakeReceiver::AudioReadyRead() {
  while (audio_socket_->hasPendingDatagrams()) {
    const QByteArray packet = audio_socket_->receiveDatagram().data();
    if (packet.size() < 12) continue;
    ++audio_packets_;

    if (audio_key_.isEmpty()) {
      // AirPlay 1: an uncompressed ALAC frame.
      BitReader reader(packet.mid(12));
      reader.Get(3 + 4 + 12);
      if (reader.Get(4) != 0b0001) {
        errors_ << "an ALAC frame isn't uncompressed";
        continue;
      }
      for (int i = 0; i < kSamplesPerPacket; ++i) {
        audio_ += Bytes(qToLittleEndian(quint16(reader.Get(16))));
      }
      continue;
    }

    // AirPlay 2: the payload, its tag, then the last 8 bytes of its nonce.
    const QByteArray nonce = QByteArray(4, '\0') + packet.right(8);
    if (audio_nonces_.contains(nonce)) {
      errors_ << "an audio nonce was used twice";
    }
    audio_nonces_ << nonce;
    QByteArray pcm;
    if (!Open(audio_key_, nonce, packet.mid(4, 8),
              packet.mid(12, packet.size() - 12 - 8), &pcm)) {
      errors_ << "an audio packet didn't decrypt";
      continue;
    }
    // L16 is big endian.
    for (int i = 0; i + 1 < pcm.size(); i += 2) {
      audio_ +=
          Bytes(qToLittleEndian(qFromBigEndian<quint16>(pcm.constData() + i)));
    }
  }
}

void FakeReceiver::NewEventConnection() {
  if (event_socket_) event_socket_->deleteLater();
  event_socket_ = event_server_->nextPendingConnection();
  event_buffer_.clear();
  // The sender reads with the Events-Write key, and writes with Events-Read.
  event_cipher_.reset(new FrameCipher(
      HkdfSha512("Events-Salt", "Events-Write-Encryption-Key", shared_key_),
      HkdfSha512("Events-Salt", "Events-Read-Encryption-Key", shared_key_)));
  connect(event_socket_, SIGNAL(readyRead()), SLOT(EventReadyRead()));
}

// A Mac sends its senders requests on the event channel; they answer 200 OK.
void FakeReceiver::SendEvent() {
  if (!event_socket_ || !event_cipher_) return;
  event_socket_->write(event_cipher_->Encrypt(
      "POST /command RTSP/1.0\r\nCSeq: 1\r\nContent-Length: 0\r\n\r\n"));
}

void FakeReceiver::EventReadyRead() {
  if (!event_cipher_->Decrypt(event_socket_->readAll(), &event_buffer_)) {
    errors_ << "an event reply didn't decrypt";
    return;
  }
  while (event_buffer_.contains("\r\n\r\n")) {
    const int end = event_buffer_.indexOf("\r\n\r\n");
    if (event_buffer_.startsWith("RTSP/1.0 200")) ++event_replies_;
    event_buffer_.remove(0, end + 4);
  }
}

}  // namespace airplay
