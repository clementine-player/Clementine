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

#include "airplaystream.h"

#include <QMetaObject>
#include <QMutexLocker>
#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QThread>
#include <QUdpSocket>
#include <QtEndian>
#include <chrono>

#include "airplaycrypto.h"
#include "core/logging.h"

namespace {

// How many recent packets are kept for resend requests (about 8 s).
const int kBacklog = 1000;
// How far the stream may fall behind the time before it's started again
// from now (a quarter of a second).
const quint64 kMaxGapFrames = AirPlayStream::kSampleRate / 4;

quint64 NtpToTimestamp(quint64 ntp) {
  return ((ntp >> 16) * AirPlayStream::kSampleRate) >> 16;
}

quint64 TimestampToNtp(quint64 timestamp) {
  return ((timestamp << 16) / AirPlayStream::kSampleRate) << 16;
}

}  // namespace

quint64 AirPlayStream::NtpNow() {
  using namespace std::chrono;
  const quint64 us =
      duration_cast<microseconds>(system_clock::now().time_since_epoch())
          .count();
  // NTP counts from 1900.
  // Seconds from 1900 to 1970 (RFC 5905).
  const quint64 secs = us / 1000000 + 0x83AA7E80;
  const quint64 frac = ((us % 1000000) << 32) / 1000000;
  return (secs << 32) | frac;
}

AirPlayStream::AirPlayStream(const QHostAddress& host, quint16 data_port,
                             quint16 control_port, QUdpSocket* control_socket,
                             const QByteArray& audio_key, quint32 ssrc,
                             QObject* parent)
    : QObject(parent),
      host_(host),
      data_port_(data_port),
      control_port_(control_port),
      audio_key_(audio_key),
      ssrc_(ssrc),
      audio_socket_(new QUdpSocket(this)),
      control_socket_(control_socket),
      seq_(quint16(QRandomGenerator::global()->generate())),
      start_timestamp_(NtpToTimestamp(AirPlayStream::NtpNow())),
      head_timestamp_(start_timestamp_),
      nonce_(0),
      marker_(true),
      first_sync_(true),
      frames_sent_(0) {
  connect(control_socket_, SIGNAL(readyRead()), SLOT(ControlReadyRead()));
}

AirPlayStream::~AirPlayStream() {}

void AirPlayStream::Write(const char* pcm, int size) {
  const int packet_size = kFramesPerPacket * kFrameSize;
  QMutexLocker l(&mutex_);
  // Nothing came for a while, between tracks say, so the receiver has run
  // dry and what comes now would be late: it plays from now instead.
  if (NtpToTimestamp(NtpNow()) > head_timestamp_ + kMaxGapFrames) {
    qLog(Debug) << "AirPlay stream ran dry; starting again from now";
    ReanchorLocked();
    QMetaObject::invokeMethod(this, "SendSync", Qt::QueuedConnection);
  }
  pending_.append(pcm, size);
  int pos = 0;
  for (; pos + packet_size <= pending_.size(); pos += packet_size) {
    SendPacketLocked(pending_.mid(pos, packet_size));
  }
  pending_.remove(0, pos);
}

void AirPlayStream::SendPacketLocked(const QByteArray& pcm) {
  // L16 is big endian.
  QByteArray payload(pcm.size(), '\0');
  for (int i = 0; i + 1 < pcm.size(); i += 2) {
    payload[i] = pcm[i + 1];
    payload[i + 1] = pcm[i];
  }

  const quint32 rtptime =
      quint32(head_timestamp_ - start_timestamp_ + kLatencyFrames);
  QByteArray packet;
  packet.append(char(0x80));
  // RTP version 2, then payload type 0x60 (96, dynamic, as AirPlay's audio
  // is), with the marker bit on the first packet of a burst.
  packet.append(char(marker_ ? 0xE0 : 0x60));
  packet += airplay::Bytes(qToBigEndian(seq_));
  packet += airplay::Bytes(qToBigEndian(rtptime));
  packet += airplay::Bytes(qToBigEndian(ssrc_));
  // The payload and its tag, then the last 8 bytes of the nonce.
  const QByteArray nonce = airplay::Nonce(nonce_++);
  packet += airplay::Seal(audio_key_, nonce, packet.mid(4, 8), payload);
  packet += nonce.mid(4);
  marker_ = false;

  backlog_.insert(seq_, packet);
  backlog_.remove(quint16(seq_ - kBacklog));
  ++seq_;
  head_timestamp_ += kFramesPerPacket;
  frames_sent_ += kFramesPerPacket;
  SendDatagram(packet, host_, data_port_);
}

void AirPlayStream::SendDatagram(const QByteArray& datagram,
                                 const QHostAddress& host, quint16 port) {
  if (QThread::currentThread() == thread()) {
    audio_socket_->writeDatagram(datagram, host, port);
    return;
  }
  // From the streaming thread: sockets belong to their own thread.
  QMetaObject::invokeMethod(
      this,
      [this, datagram, host, port]() {
        audio_socket_->writeDatagram(datagram, host, port);
      },
      Qt::QueuedConnection);
}

void AirPlayStream::ReanchorLocked() {
  // The RTP timestamps jump to match the time, which the next sync packet
  // tells the receiver about.
  head_timestamp_ = qMax(head_timestamp_, NtpToTimestamp(NtpNow()));
  marker_ = true;
  first_sync_ = true;
}

void AirPlayStream::Flush(quint16* seq, quint32* rtptime) {
  QMutexLocker l(&mutex_);
  pending_.clear();
  ReanchorLocked();
  *seq = seq_;
  *rtptime = quint32(head_timestamp_ - start_timestamp_ + kLatencyFrames);
}

void AirPlayStream::Next(quint16* seq, quint32* rtptime) const {
  QMutexLocker l(&mutex_);
  *seq = seq_;
  *rtptime = quint32(head_timestamp_ - start_timestamp_ + kLatencyFrames);
}

qint64 AirPlayStream::frames_sent() const {
  QMutexLocker l(&mutex_);
  return frames_sent_;
}

void AirPlayStream::SendSync() {
  QByteArray packet;
  {
    QMutexLocker l(&mutex_);
    const quint32 rtptime =
        quint32(head_timestamp_ - start_timestamp_ + kLatencyFrames);
    // RAOP's sync packet: payload type 0x54 with the marker bit, and the
    // extension bit on the first, as pyatv and shairport-sync have it.
    packet.append(char(first_sync_ ? 0x90 : 0x80));
    packet.append(char(0xD4));
    packet += airplay::Bytes(qToBigEndian(quint16(7)));
    packet += airplay::Bytes(qToBigEndian(quint32(rtptime - kLatencyFrames)));
    packet += airplay::Bytes(qToBigEndian(TimestampToNtp(head_timestamp_)));
    packet += airplay::Bytes(qToBigEndian(rtptime));
    first_sync_ = false;
  }
  control_socket_->writeDatagram(packet, host_, control_port_);
}

// Answers resend requests: the receiver missed |count| packets from |seq|.
void AirPlayStream::ControlReadyRead() {
  while (control_socket_->hasPendingDatagrams()) {
    const QNetworkDatagram datagram = control_socket_->receiveDatagram();
    const QByteArray data = datagram.data();
    // RAOP's resend request (0x55), answered with the packets wrapped in a
    // retransmit (0xD6, 0x56 with the marker bit).
    if (data.size() < 8 || (quint8(data[1]) & 0x7F) != 0x55) continue;
    const quint16 lost = qFromBigEndian<quint16>(data.constData() + 4);
    const quint16 count = qFromBigEndian<quint16>(data.constData() + 6);
    qLog(Debug) << "AirPlay receiver asked for" << count << "packets from"
                << lost;
    QMutexLocker l(&mutex_);
    for (quint16 i = 0; i < count; ++i) {
      const QByteArray packet = backlog_.value(quint16(lost + i));
      if (packet.isEmpty()) continue;
      QByteArray resend;
      resend.append(char(0x80));
      resend.append(char(0xD6));
      resend += packet.mid(2, 2);
      resend += packet;
      control_socket_->writeDatagram(resend, datagram.senderAddress(),
                                     datagram.senderPort());
    }
  }
}
