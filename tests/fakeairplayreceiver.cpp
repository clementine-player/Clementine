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

#include "fakeairplayreceiver.h"

#include <QCryptographicHash>
#include <QMap>
#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QVariantMap>
#include <QtEndian>

#include "airplay/airplaydmap.h"
#include "airplay/airplayplist.h"
#include "airplay/airplaystream.h"
#include "airplay/airplaytlv8.h"
#include "mini-gmp.h"

namespace {

// RFC 5054's 3072-bit group, as AirPlaySrp uses.
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
// Its long-term Ed25519 seed.
const QByteArray kSeed(32, '\x42');
// What a Mac reported without a password: 0x4 at first, then 0x84.
const quint32 kStatusFlags = 0x84;

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

double NtpSeconds(quint64 ntp) {
  return (ntp >> 32) + (ntp & 0xFFFFFFFF) / 4294967296.0;
}

// Reads bits most significant first, as ALAC has them.
class BitReader {
 public:
  explicit BitReader(const QByteArray& data) : data_(data) {}

  quint32 Read(int bits) {
    quint32 value = 0;
    for (int i = 0; i < bits; ++i, ++pos_) {
      const int byte = pos_ / 8;
      const bool bit =
          byte < data_.size() && (quint8(data_[byte]) >> (7 - pos_ % 8)) & 1;
      value = (value << 1) | (bit ? 1 : 0);
    }
    return value;
  }
  // Whether it read past the end.
  bool overrun() const { return pos_ > data_.size() * 8; }
  // Bits left in the last byte.
  int left() const { return data_.size() * 8 - pos_; }

 private:
  const QByteArray data_;
  int pos_ = 0;
};

QByteArray Md5Hex(const QByteArray& data) {
  return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

const char kRealm[] = "raop";

QByteArray ReasonPhrase(int code) {
  switch (code) {
    case 200:
      return "OK";
    case 400:
      return "Bad Request";
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

}  // namespace

QByteArray FakeAirPlayReceiver::Request::Header(const QByteArray& name) const {
  for (const auto& h : headers) {
    if (h.first.compare(name, Qt::CaseInsensitive) == 0) return h.second;
  }
  return QByteArray();
}

FakeAirPlayReceiver::FakeAirPlayReceiver()
    : server_(new QTcpServer(this)),
      client_(nullptr),
      paired_(false),
      airplay1_(false),
      airplay1_pairing_(false),
      ask_to_allow_(true),
      report_password_(true),
      prompting_(false),
      prompts_(0),
      timing_(new QUdpSocket(this)),
      sender_timing_port_(0),
      timing_requests_(0),
      timing_replies_(0),
      torn_down_(false),
      event_server_(new QTcpServer(this)),
      event_socket_(nullptr),
      control_(new QUdpSocket(this)),
      audio_socket_(new QUdpSocket(this)),
      waiting_for_time_(false),
      timing_replies_before_setup_(0),
      setups_after_timing_(0),
      audio_packets_(0),
      sync_packets_(0),
      event_replies_(0),
      flushes_(0),
      flushed_once_(false),
      showing_pin_(false),
      flush_seq_(-1),
      have_sync_(false),
      sync_rtptime_(0),
      sync_ntp_(0),
      late_packets_(0),
      dropped_packets_(0),
      last_seq_(-1),
      last_rtptime_(0) {
  connect(event_server_, &QTcpServer::newConnection, this,
          [this]() { NewEventConnection(); });
  connect(control_, &QUdpSocket::readyRead, this,
          [this]() { ControlReadyRead(); });
  connect(audio_socket_, &QUdpSocket::readyRead, this,
          [this]() { AudioReadyRead(); });
  connect(server_, &QTcpServer::newConnection, this,
          [this]() { NewConnection(); });
  connect(timing_, &QUdpSocket::readyRead, this,
          [this]() { TimingReadyRead(); });
}

FakeAirPlayReceiver::~FakeAirPlayReceiver() {}

bool FakeAirPlayReceiver::Listen() {
  const QHostAddress local = QHostAddress::LocalHost;
  return server_->listen(local) && timing_->bind(local) &&
         event_server_->listen(local) && control_->bind(local) &&
         audio_socket_->bind(local);
}

AirPlayDevice FakeAirPlayReceiver::device() const {
  AirPlayDevice device;
  device.id = "AA:BB:CC:00:11:22";
  device.name = "Fake";
  device.model = "Mac14,2";
  device.features = 0x38174fde4a7fcfd5;
  device.status_flags = status_flags();
  if (airplay1_) {
    // As ParseRaopService gives it, with "pw=true" for a password.
    device.model = "AirPort10,115";
    device.features = 0x77;
    device.airplay1 = true;
  }
  device.address = QHostAddress::LocalHost;
  device.port = server_->serverPort();
  return device;
}

quint32 FakeAirPlayReceiver::status_flags() const {
  return kStatusFlags | (password_.isEmpty() || !report_password_
                             ? 0
                             : AirPlayDevice::PasswordRequired);
}

void FakeAirPlayReceiver::SetPassword(const QString& password) {
  password_ = password;
  allowed_names_.clear();
}

void FakeAirPlayReceiver::Allow() {
  if (!prompting_) return;
  prompting_ = false;
  allowed_names_ << prompted_name_;
  ProcessRequests();
}

void FakeAirPlayReceiver::Deny() {
  if (!prompting_ || !held_) return;
  prompting_ = false;
  Respond(*held_, 400);
  held_.reset();
  ProcessRequests();
}

void FakeAirPlayReceiver::NewConnection() {
  // One sender at a time: a new connection replaces the last.
  if (client_) client_->deleteLater();
  client_ = server_->nextPendingConnection();
  buffer_.clear();
  held_.reset();
  cipher_.reset();
  paired_ = false;
  prompting_ = false;
  sender_timing_port_ = 0;
  showing_pin_ = false;
  srp_key_.clear();
  flush_seq_ = -1;
  have_sync_ = false;
  last_seq_ = -1;
  waiting_for_time_ = false;
  digest_nonce_ = RandomBytes(16).toHex();
  audio_key_.clear();
  audio_nonces_.clear();
  flushed_once_ = false;
  connect(client_, &QTcpSocket::readyRead, this, [this]() { ReadyRead(); });
}

void FakeAirPlayReceiver::ReadyRead() {
  const QByteArray data = client_->readAll();
  if (cipher_) {
    if (!cipher_->Decrypt(data, &buffer_)) {
      client_->abort();
      return;
    }
  } else {
    buffer_ += data;
  }
  ProcessRequests();
}

void FakeAirPlayReceiver::ProcessRequests() {
  while (client_) {
    if (!held_) {
      Request request;
      if (!ParseRequest(&request)) return;
      // RTSP URIs name the sender's address and a random session.
      requests_ << QString::fromUtf8(request.uri.startsWith("rtsp://")
                                         ? request.method
                                         : request.method + " " + request.uri);
      client_names_ << QString::fromUtf8(request.Header("X-Apple-Client-Name"));
      held_ = request;
    }
    if (!Handle(*held_)) return;
    held_.reset();
  }
}

bool FakeAirPlayReceiver::ParseRequest(Request* request) {
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

void FakeAirPlayReceiver::Respond(const Request& request, int code,
                                  const QByteArray& body,
                                  const Headers& headers) {
  QByteArray message = "RTSP/1.0 " + QByteArray::number(code) + " " +
                       ReasonPhrase(code) + "\r\n";
  message += "CSeq: " + request.Header("CSeq") + "\r\n";
  for (const auto& h : headers) message += h.first + ": " + h.second + "\r\n";
  message += "Server: AirTunes/980.77.5\r\n";
  message += "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n";
  message += body;
  client_->write(cipher_ ? cipher_->Encrypt(message) : message);
}

bool FakeAirPlayReceiver::Handle(const Request& request) {
  if (airplay1_) {
    const std::optional<bool> handled = HandleAirPlay1(request);
    if (handled) return *handled;
  }
  if (request.method == "GET" && request.uri == "/info") {
    QVariantMap info;
    info["name"] = "Fake";
    info["model"] = "Mac14,2";
    info["sourceVersion"] = "980.77.5";
    info["statusFlags"] = status_flags();
    Respond(request, 200, airplay::bplist::Write(info));
  } else if (request.method == "POST" && request.uri == "/pair-pin-start") {
    // Shows a PIN, which transient pairing doesn't use.
    if (pin_.isEmpty()) errors_ << "pair-pin-start";
    showing_pin_ = !pin_.isEmpty();
    Respond(request, 200);
  } else if (request.method == "POST" && request.uri == "/pair-setup") {
    const QByteArray seq = airplay::tlv8::Get(airplay::tlv8::Read(request.body),
                                              airplay::tlv8::kSeqNo);
    if (seq == QByteArray(1, 5)) {
      PairSetupM5(request);
    } else {
      PairSetup(request);
    }
  } else if (request.method == "POST" && request.uri == "/pair-verify") {
    PairVerify(request);
  } else if (request.method == "SETUP" &&
             request.Header("Content-Type") ==
                 "application/x-apple-binary-plist" &&
             request.body.contains("streams")) {
    return SetupStream(request);
  } else if (request.method == "SETUP") {
    // Asks for the time straight away, then for its user to allow the
    // sender.
    if (!sender_timing_port_) {
      QVariant body;
      airplay::bplist::Read(request.body, &body);
      sender_timing_port_ = body.toMap().value("timingPort").toUInt();
      sender_mac_address_ = body.toMap().value("macAddress").toString();
      SendTimingRequest();
    }
    if (!CheckAllowed(request)) return false;
    Respond(request, 200,
            airplay::bplist::Write(
                QVariantMap{{"eventPort", event_server_->serverPort()}}));
  } else if (request.method == "RECORD") {
    Respond(request, 200);
    // A Mac sends its senders requests on the event channel.
    if (event_socket_ && event_cipher_) {
      event_socket_->write(event_cipher_->Encrypt(
          "POST /command RTSP/1.0\r\nCSeq: 1\r\nContent-Length: 0\r\n\r\n"));
    }
  } else if (request.method == "FLUSH") {
    if (flushed_once_) ++flushes_;
    flushed_once_ = true;
    for (const QByteArray& part : request.Header("RTP-Info").split(';')) {
      if (part.startsWith("seq=")) flush_seq_ = part.mid(4).toInt();
    }
    Respond(request, 200);
  } else if (request.method == "SET_PARAMETER") {
    const QByteArray type = request.Header("Content-Type");
    // Metadata and artwork apply from an RTP timestamp.
    const bool timed = request.Header("RTP-Info").startsWith("rtptime=");
    if (request.body.startsWith("volume: ")) {
      volume_ = request.body.mid(8).trimmed();
    } else if (request.body.startsWith("progress: ")) {
      progress_ = request.body.mid(10).trimmed();
    } else if (type == "application/x-dmap-tagged" && timed) {
      namespace dmap = airplay::dmap;
      const dmap::Items item =
          dmap::Read(dmap::Get(dmap::Read(request.body), "mlit"));
      now_playing_ = QStringList();
      for (const char* tag : {"minm", "asar", "asal"}) {
        now_playing_ << QString::fromUtf8(dmap::Get(item, tag));
      }
    } else if (type.startsWith("image/") && timed) {
      artwork_type_ = type;
      artwork_ = request.body;
    } else {
      errors_ << "SET_PARAMETER of something else";
    }
    Respond(request, 200);
  } else if (request.method == "POST" && request.uri == "/feedback") {
    Respond(request, 200);
  } else if (request.method == "TEARDOWN") {
    Respond(request, 200);
    torn_down_ = true;
  } else {
    Respond(request, 501);
  }
  return true;
}

bool FakeAirPlayReceiver::Authorized(const Request& request) const {
  if (password_.isEmpty() || airplay1_pairing_) return true;
  const QByteArray auth = request.Header("Authorization");
  auto param = [&auth](const QByteArray& name) {
    const int start = auth.indexOf(name + "=\"");
    if (start < 0) return QByteArray();
    const int from = start + name.size() + 2;
    return auth.mid(from, auth.indexOf('"', from) - from);
  };
  const QByteArray ha1 =
      Md5Hex(param("username") + ":" + kRealm + ":" + password_.toUtf8());
  const QByteArray ha2 = Md5Hex(request.method + ":" + request.uri);
  return auth.startsWith("Digest ") && param("nonce") == digest_nonce_ &&
         param("uri") == request.uri &&
         param("response") == Md5Hex(ha1 + ":" + digest_nonce_ + ":" + ha2);
}

std::optional<bool> FakeAirPlayReceiver::HandleAirPlay1(
    const Request& request) {
  // Pairing goes as for AirPlay 2.
  if (request.method == "POST" && request.uri != "/feedback") {
    return std::nullopt;
  }
  if (!Authorized(request)) {
    Respond(request, 401, {},
            {{"WWW-Authenticate", QByteArray("Digest realm=\"") + kRealm +
                                      "\", nonce=\"" + digest_nonce_ + "\""}});
    return true;
  }
  if (request.method != "ANNOUNCE" && request.method != "SETUP" &&
      request.method != "TEARDOWN" && request.Header("Session") != "1") {
    errors_ << QString("%1 without the session").arg(QString(request.method));
  }

  if (request.method == "ANNOUNCE") {
    if (airplay1_pairing_ && !paired_) {
      Respond(request, 403);
      return true;
    }
    if (!request.body.contains("a=rtpmap:96 AppleLossless") ||
        request.body.contains("rsaaeskey")) {
      errors_ << "ANNOUNCE isn't of unencrypted ALAC";
    }
    if (!CheckAllowed(request)) return false;
    Respond(request, 200);
  } else if (request.method == "SETUP") {
    for (const QByteArray& part : request.Header("Transport").split(';')) {
      if (part.startsWith("timing_port=")) {
        sender_timing_port_ = part.mid(12).toUInt();
      }
    }
    if (!sender_timing_port_) errors_ << "SETUP without a timing port";
    SendTimingRequest();
    const QByteArray transport =
        "RTP/AVP/UDP;unicast;mode=record;server_port=" +
        QByteArray::number(audio_socket_->localPort()) +
        ";control_port=" + QByteArray::number(control_->localPort()) +
        ";timing_port=" + QByteArray::number(timing_->localPort());
    Respond(request, 200, {},
            {{"Transport", transport}, {"Session", "1;timeout=60"}});
  } else if (request.method == "RECORD") {
    // Where the stream starts.
    for (const QByteArray& part : request.Header("RTP-Info").split(';')) {
      if (part.startsWith("seq=")) flush_seq_ = part.mid(4).toInt();
    }
    if (flush_seq_ < 0) errors_ << "RECORD without RTP-Info";
    flushed_once_ = true;
    Respond(request, 200);
  } else if (request.method == "GET" ||
             (request.method == "POST" && request.uri == "/feedback")) {
    // An AirPort Express has neither.
    errors_ << QString("%1 %2").arg(QString(request.method),
                                    QString(request.uri));
    Respond(request, 404);
  } else {
    // FLUSH, SET_PARAMETER and TEARDOWN go as for AirPlay 2.
    return std::nullopt;
  }
  return true;
}

bool FakeAirPlayReceiver::CheckAllowed(const Request& request) {
  const QString name = QString::fromUtf8(request.Header("X-Apple-Client-Name"));
  if (!ask_to_allow_ || allowed_names_.contains(name)) return true;
  if (!prompting_) {
    prompting_ = true;
    prompted_name_ = name;
    ++prompts_;
    if (on_prompt_) on_prompt_(name);
  }
  return false;
}

void FakeAirPlayReceiver::PairSetup(const Request& request) {
  namespace tlv8 = airplay::tlv8;
  const tlv8::Items items = tlv8::Read(request.body);
  const QByteArray seq_item = tlv8::Get(items, tlv8::kSeqNo);
  const int seq = seq_item.isEmpty() ? 0 : quint8(seq_item[0]);

  BigInt n;
  mpz_set_str(n.get(), kN, 16);
  BigInt g;
  mpz_set_ui(g.get(), 5);
  const int n_size = n.Bytes().size();
  const QByteArray pin = showing_pin_          ? pin_.toUtf8()
                         : password_.isEmpty() ? QByteArray(kTransientPin)
                                               : password_.toUtf8();

  if (seq == 1) {
    srp_salt_ = RandomBytes(16);
    srp_salt_[0] = char(srp_salt_[0] | 0x80);  // No leading zero to drop
    srp_b_ = RandomBytes(32);
  }
  // v = g ^ H(s, H(I:P)), k = H(N, PAD(g))
  const BigInt x(airplay::Sha512(
      srp_salt_ + airplay::Sha512(QByteArray(kUsername) + ":" + pin)));
  BigInt v;
  mpz_powm(v.get(), g.get(), x.get(), n.get());
  const BigInt k(airplay::Sha512(n.Bytes() + g.Padded(n_size)));
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
  const BigInt u(airplay::Sha512(a.Padded(n_size) + b_public.Padded(n_size)));
  BigInt s;
  mpz_powm(s.get(), v.get(), u.get(), n.get());
  mpz_mul(s.get(), s.get(), a.get());
  mpz_mod(s.get(), s.get(), n.get());
  mpz_powm(s.get(), s.get(), b.get(), n.get());
  const QByteArray key = airplay::Sha512(s.Bytes());

  const BigInt hn(airplay::Sha512(n.Bytes()));
  const BigInt hg(airplay::Sha512(g.Bytes()));
  BigInt hxor;
  mpz_xor(hxor.get(), hn.get(), hg.get());
  const QByteArray m1 =
      airplay::Sha512(hxor.Bytes() + airplay::Sha512(kUsername) + srp_salt_ +
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
                       {tlv8::kProof, airplay::Sha512(a.Bytes() + m1 + key)}}));
  // Pairing that's remembered goes on to exchange long-term keys.
  if (request.Header("X-Apple-HKP") == "3") {
    srp_key_ = key;
    return;
  }
  Encrypt(key);
}

void FakeAirPlayReceiver::Encrypt(const QByteArray& shared) {
  shared_key_ = shared;
  cipher_.reset(new airplay::FrameCipher(
      airplay::HkdfSha512("Control-Salt", "Control-Read-Encryption-Key",
                          shared),
      airplay::HkdfSha512("Control-Salt", "Control-Write-Encryption-Key",
                          shared)));
  paired_ = true;
}

void FakeAirPlayReceiver::SendTimingRequest() {
  if (!sender_timing_port_) return;
  QByteArray packet;
  packet.append(char(0x80));
  packet.append(char(0xD2));
  packet += airplay::Bytes(qToBigEndian(quint16(7)));
  packet += QByteArray(20, '\0');
  packet += airplay::Bytes(qToBigEndian(quint64(++timing_requests_)));
  timing_->writeDatagram(packet, QHostAddress::LocalHost, sender_timing_port_);
}

void FakeAirPlayReceiver::TimingReadyRead() {
  while (timing_->hasPendingDatagrams()) {
    const QByteArray data = timing_->receiveDatagram().data();
    if (data.size() >= 32 && (quint8(data[1]) & 0x7F) == 0x53) {
      ++timing_replies_;
    }
  }
  ProcessRequests();
}

bool FakeAirPlayReceiver::SetupStream(const Request& request) {
  QVariant body;
  airplay::bplist::Read(request.body, &body);
  const QVariantMap map = body.toMap();

  // The SETUP that starts the stream isn't answered until the sender has
  // answered a request for the time.
  if (!waiting_for_time_) {
    waiting_for_time_ = true;
    timing_replies_before_setup_ = timing_replies_;
    SendTimingRequest();
    return false;
  }
  if (timing_replies_ <= timing_replies_before_setup_) return false;
  waiting_for_time_ = false;
  ++setups_after_timing_;

  audio_key_ =
      map.value("streams").toList().value(0).toMap().value("shk").toByteArray();
  if (audio_key_.size() != 32) errors_ << "shk isn't 32 bytes";
  for (const char* info :
       {"Events-Write-Encryption-Key", "Events-Read-Encryption-Key"}) {
    if (audio_key_ == airplay::HkdfSha512("Events-Salt", info, shared_key_)) {
      errors_ << QString("shk is the event channel's %1").arg(info);
    }
  }
  const QVariantMap stream{{"type", 0x60},
                           {"dataPort", audio_socket_->localPort()},
                           {"controlPort", control_->localPort()}};
  Respond(
      request, 200,
      airplay::bplist::Write(QVariantMap{{"streams", QVariantList{stream}}}));
  return true;
}

void FakeAirPlayReceiver::ControlReadyRead() {
  while (control_->hasPendingDatagrams()) {
    const QByteArray data = control_->receiveDatagram().data();
    if (data.size() < 20 || (quint8(data[1]) & 0x7F) != 0x54) continue;
    ++sync_packets_;
    // What's playing now, and when that is.
    sync_rtptime_ = qFromBigEndian<quint32>(data.constData() + 4);
    sync_ntp_ = qFromBigEndian<quint64>(data.constData() + 8);
    have_sync_ = true;
  }
}

// The payload and its tag, then the last 8 bytes of its nonce.
void FakeAirPlayReceiver::AudioReadyRead() {
  while (audio_socket_->hasPendingDatagrams()) {
    const QByteArray packet = audio_socket_->receiveDatagram().data();
    if (packet.size() < (airplay1_ ? 12 + 3 : 12 + 16 + 8)) continue;
    ++audio_packets_;
    const quint16 seq = qFromBigEndian<quint16>(packet.constData() + 2);
    // Each packet follows the last, in sequence and in time.
    const quint32 rtptime = qFromBigEndian<quint32>(packet.constData() + 4);
    if (last_seq_ >= 0) {
      if (seq != quint16(last_seq_ + 1)) {
        errors_ << QString("packet %1 followed %2").arg(seq).arg(last_seq_);
      }
      if (qint32(rtptime - last_rtptime_) <= 0) {
        errors_ << QString("packet %1 went back in time").arg(seq);
      }
    }
    last_seq_ = seq;
    last_rtptime_ = rtptime;
    // A packet from before the last FLUSH is dropped, as is one that would
    // play too late, going by the last sync packet, as a Mac drops them.
    if (flush_seq_ >= 0 && qint16(seq - quint16(flush_seq_)) < 0) {
      ++dropped_packets_;
      continue;
    }
    if (have_sync_) {
      const double plays_at =
          NtpSeconds(sync_ntp_) + qint32(rtptime - sync_rtptime_) / 44100.0;
      if (plays_at < NtpSeconds(AirPlayStream::NtpNow())) {
        ++late_packets_;
        continue;
      }
    }
    if (airplay1_) {
      DecodeAlac(packet.mid(12));
      continue;
    }
    const QByteArray nonce = QByteArray(4, '\0') + packet.right(8);
    if (audio_nonces_.contains(nonce)) {
      errors_ << "an audio nonce was used twice";
    }
    audio_nonces_ << nonce;
    QByteArray pcm;
    if (!airplay::Open(audio_key_, nonce, packet.mid(4, 8),
                       packet.mid(12, packet.size() - 12 - 8), &pcm)) {
      errors_ << "an audio packet didn't decrypt";
      continue;
    }
    // L16 is big endian.
    for (int i = 0; i + 1 < pcm.size(); i += 2) {
      audio_ += airplay::Bytes(
          qToLittleEndian(qFromBigEndian<quint16>(pcm.constData() + i)));
    }
  }
}

// Only the uncompressed frames AirPlaySession sends, checking each field.
void FakeAirPlayReceiver::DecodeAlac(const QByteArray& frame) {
  BitReader r(frame);
  const quint32 tag = r.Read(3);
  r.Read(4);  // Element instance tag
  const quint32 unused = r.Read(12);
  const quint32 partial = r.Read(1);
  const quint32 shift = r.Read(2);
  const quint32 escape = r.Read(1);
  if (tag != 1 || unused != 0 || partial != 0 || shift != 0 || escape != 1) {
    errors_ << "an ALAC frame isn't an uncompressed channel pair";
    return;
  }
  QByteArray pcm;
  for (int i = 0; i < AirPlayStream::kFramesPerPacket * 2; ++i) {
    pcm += airplay::Bytes(qToLittleEndian(quint16(r.Read(16))));
  }
  if (r.Read(3) != 7 || r.overrun() || r.left() >= 8) {
    errors_ << "an ALAC frame doesn't end where it should";
    return;
  }
  audio_ += pcm;
}

void FakeAirPlayReceiver::NewEventConnection() {
  if (event_socket_) event_socket_->deleteLater();
  event_socket_ = event_server_->nextPendingConnection();
  event_buffer_.clear();
  // The sender reads with the Events-Write key, and writes with Events-Read.
  event_cipher_.reset(new airplay::FrameCipher(
      airplay::HkdfSha512("Events-Salt", "Events-Write-Encryption-Key",
                          shared_key_),
      airplay::HkdfSha512("Events-Salt", "Events-Read-Encryption-Key",
                          shared_key_)));
  connect(event_socket_, &QTcpSocket::readyRead, this,
          [this]() { EventReadyRead(); });
}

void FakeAirPlayReceiver::EventReadyRead() {
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

QByteArray FakeAirPlayReceiver::PublicKey() {
  return airplay::Ed25519PublicKey(kSeed);
}

// Pair-setup M5 and M6: the sender's long-term key for this receiver's.
void FakeAirPlayReceiver::PairSetupM5(const Request& request) {
  namespace tlv8 = airplay::tlv8;
  const QByteArray key = airplay::HkdfSha512(
      "Pair-Setup-Encrypt-Salt", "Pair-Setup-Encrypt-Info", srp_key_);
  QByteArray plain;
  if (srp_key_.isEmpty() ||
      !airplay::Open(key, QByteArray(4, '\0') + "PS-Msg05", {},
                     tlv8::Get(tlv8::Read(request.body), tlv8::kEncryptedData),
                     &plain)) {
    errors_ << "pair-setup M5 didn't decrypt";
    Respond(request, 200,
            tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 6)},
                         {tlv8::kError, QByteArray(1, 2)}}));
    return;
  }
  const tlv8::Items sub = tlv8::Read(plain);
  const QByteArray id = tlv8::Get(sub, tlv8::kIdentifier);
  const QByteArray public_key = tlv8::Get(sub, tlv8::kPublicKey);
  const QByteArray info =
      airplay::HkdfSha512("Pair-Setup-Controller-Sign-Salt",
                          "Pair-Setup-Controller-Sign-Info", srp_key_) +
      id + public_key;
  if (!airplay::Ed25519Verify(public_key, tlv8::Get(sub, tlv8::kSignature),
                              info)) {
    errors_ << "pair-setup M5's signature didn't verify";
    Respond(request, 200,
            tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 6)},
                         {tlv8::kError, QByteArray(1, 2)}}));
    return;
  }
  senders_[id] = public_key;

  const QByteArray our_info =
      airplay::HkdfSha512("Pair-Setup-Accessory-Sign-Salt",
                          "Pair-Setup-Accessory-Sign-Info", srp_key_) +
      PairingId() + PublicKey();
  const QByteArray reply = tlv8::Write({
      {tlv8::kIdentifier, PairingId()},
      {tlv8::kPublicKey, PublicKey()},
      {tlv8::kSignature, airplay::Ed25519Sign(kSeed, our_info)},
  });
  Respond(request, 200,
          tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 6)},
                       {tlv8::kEncryptedData,
                        airplay::Seal(key, QByteArray(4, '\0') + "PS-Msg06", {},
                                      reply)}}));
  showing_pin_ = false;
}

// Pair-verify M1 to M4.
void FakeAirPlayReceiver::PairVerify(const Request& request) {
  namespace tlv8 = airplay::tlv8;
  const tlv8::Items items = tlv8::Read(request.body);
  if (tlv8::Get(items, tlv8::kSeqNo) == QByteArray(1, 1)) {
    const QByteArray secret = airplay::RandomKey();
    verify_our_public_ = airplay::X25519PublicKey(secret);
    verify_their_public_ = tlv8::Get(items, tlv8::kPublicKey);
    verify_shared_ = airplay::X25519Shared(secret, verify_their_public_);
    const QByteArray key = airplay::HkdfSha512(
        "Pair-Verify-Encrypt-Salt", "Pair-Verify-Encrypt-Info", verify_shared_);
    const QByteArray signature = airplay::Ed25519Sign(
        kSeed, verify_our_public_ + PairingId() + verify_their_public_);
    const QByteArray sub = tlv8::Write(
        {{tlv8::kIdentifier, PairingId()}, {tlv8::kSignature, signature}});
    Respond(request, 200,
            tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 2)},
                         {tlv8::kPublicKey, verify_our_public_},
                         {tlv8::kEncryptedData,
                          airplay::Seal(key, QByteArray(4, '\0') + "PV-Msg02",
                                        {}, sub)}}));
    return;
  }

  const QByteArray key = airplay::HkdfSha512(
      "Pair-Verify-Encrypt-Salt", "Pair-Verify-Encrypt-Info", verify_shared_);
  QByteArray plain;
  bool ok = airplay::Open(key, QByteArray(4, '\0') + "PV-Msg03", {},
                          tlv8::Get(items, tlv8::kEncryptedData), &plain);
  if (ok) {
    const tlv8::Items sub = tlv8::Read(plain);
    const QByteArray id = tlv8::Get(sub, tlv8::kIdentifier);
    ok = senders_.contains(id) &&
         airplay::Ed25519Verify(senders_[id], tlv8::Get(sub, tlv8::kSignature),
                                verify_their_public_ + id + verify_our_public_);
  }
  if (!ok) {
    Respond(request, 200,
            tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 4)},
                         {tlv8::kError, QByteArray(1, 2)}}));
    return;
  }
  Respond(request, 200, tlv8::Write({{tlv8::kSeqNo, QByteArray(1, 4)}}));
  Encrypt(verify_shared_);
}
