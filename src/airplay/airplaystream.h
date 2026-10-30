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

#ifndef AIRPLAY_AIRPLAYSTREAM_H_
#define AIRPLAY_AIRPLAYSTREAM_H_

#include <QByteArray>
#include <QHash>
#include <QHostAddress>
#include <QMutex>
#include <QObject>

class QUdpSocket;

// The audio of an AirPlay session: 44.1 kHz 16-bit stereo PCM, in RTP
// packets of 352 frames. AirPlay 2 sends it as L16 (big endian), each packet
// encrypted with ChaCha20-Poly1305 under the key the sender chose in SETUP.
// AirPlay 1 sends it unencrypted, as ALAC frames that hold it uncompressed,
// which every receiver decodes and which costs nothing to make.
//
// Write() may be called from any thread, such as GStreamer's streaming
// thread, which delivers audio in real time. The packets are sent from the
// thread this object lives on, as are sync packets and resends.
class AirPlayStream : public QObject {
  Q_OBJECT

 public:
  static const int kSampleRate = 44100;
  // What every AirPlay sender puts in a packet, from AirTunes' ALAC frames.
  static const int kFramesPerPacket = 352;
  // Interleaved S16 stereo.
  static const int kFrameSize = 4;
  // How far behind the RTP timestamps receivers play (1.5 s): AirTunes'
  // traditional 2 s less the 0.5 s it counts from, as pyatv and
  // shairport-sync have it.
  static const quint32 kLatencyFrames = 22050 + kSampleRate;

  // |audio_key| is the 32-byte key sent in SETUP as "shk", or empty for
  // AirPlay 1's unencrypted ALAC. Sync packets
  // go from |control_socket|, whose port SETUP gave the receiver, to
  // |control_port|; it isn't owned.
  AirPlayStream(const QHostAddress& host, quint16 data_port,
                quint16 control_port, QUdpSocket* control_socket,
                const QByteArray& audio_key, quint32 ssrc,
                QObject* parent = nullptr);
  ~AirPlayStream();

  // Now, as a 64-bit NTP timestamp: seconds since 1900, then the fraction.
  static quint64 NtpNow();
  // An ALAC frame of |pcm|, S16LE stereo of kFramesPerPacket frames, stored
  // uncompressed.
  static QByteArray UncompressedAlac(const QByteArray& pcm);

  // Sends |size| bytes of S16LE stereo PCM, in whole packets: a partial one
  // waits for the next call.
  void Write(const char* pcm, int size);

  // Drops any partial packet, and returns where the stream goes on from, for
  // FLUSH's RTP-Info: from now, if it had fallen behind. The next packet is
  // marked as the start of a burst, and the next sync packet as the first.
  void Flush(quint16* seq, quint32* rtptime);
  // Where the next packet goes, for the first FLUSH.
  void Next(quint16* seq, quint32* rtptime) const;

  // Sends a sync packet on the control channel: what's playing now, and the
  // time and RTP timestamp of the next packet.
  Q_INVOKABLE void SendSync();

  // How many frames have been sent.
  qint64 frames_sent() const;

 private slots:
  void ControlReadyRead();

 private:
  void SendPacketLocked(const QByteArray& pcm);
  // Makes the stream go on from now, after it fell behind the time while
  // nothing was written, eg. while paused.
  void ReanchorLocked();
  void SendDatagram(const QByteArray& datagram, const QHostAddress& host,
                    quint16 port);

  const QHostAddress host_;
  const quint16 data_port_;
  const quint16 control_port_;
  const QByteArray audio_key_;
  const quint32 ssrc_;

  QUdpSocket* audio_socket_;
  QUdpSocket* control_socket_;

  mutable QMutex mutex_;
  // Everything below is guarded by mutex_.
  QByteArray pending_;
  quint16 seq_;
  quint64 start_timestamp_;
  quint64 head_timestamp_;
  quint64 nonce_;
  bool marker_;
  bool first_sync_;
  // Recent packets, for resend requests.
  QHash<quint16, QByteArray> backlog_;
  qint64 frames_sent_;
};

#endif  // AIRPLAY_AIRPLAYSTREAM_H_
