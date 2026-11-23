/* This file is part of Clementine.
   Copyright 2013, Andreas Muttscheller <asfa194@gmail.com>

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

#include "remoteclient.h"

#include <QDataStream>
#include <QSettings>
#include <QTimer>

#include "core/logging.h"
#include "networkremote.h"
#include "networkremote/authattemptlimiter.h"

namespace {
// Cap pre-auth frames at 64 KiB so unauthenticated peers cannot buffer up to
// kMaxMessageLength (64 MiB) before passing PIN verification.
constexpr quint32 kMaxPreAuthMessageLength = 64 * 1024;
constexpr quint32 kMaxMessageLength = 64 * 1024 * 1024;
constexpr int kAuthTimeoutMsec = 10000;
}  // namespace

std::atomic<int> RemoteClient::sNextId(1);

RemoteClient::RemoteClient(Application* app, QTcpSocket* client,
                           AuthAttemptLimiter* auth_limiter)
    : app_(app),
      id_(sNextId++),
      auth_limiter_(auth_limiter),
      use_auth_code_(false),
      auth_code_(0),
      authenticated_(false),
      allow_downloads_(false),
      downloader_(false),
      closing_(false),
      disconnected_emitted_(false),
      client_(client),
      reading_protobuf_(false),
      expected_length_(0),
      song_sender_(new SongSender(app, this)) {
  // The socket arrives as a child of the QTcpServer that accepted it, which
  // would delete it along with the server, such as when the remote's settings
  // are reloaded, while this still uses it. Claim ownership on this object.
  client_->setParent(this);

  // Connect to the slot IncomingData when receiving data
  connect(client, SIGNAL(readyRead()), this, SLOT(IncomingData()));
  connect(client, SIGNAL(disconnected()), this, SLOT(SocketDisconnected()));
  connect(client, &QIODevice::bytesWritten, this, &RemoteClient::BytesWritten);

  // Check if we use auth code
  QSettings s;

  s.beginGroup(NetworkRemote::kSettingsGroup);
  use_auth_code_ = s.value("use_auth_code", false).toBool();
  auth_code_ = s.value("auth_code", 0).toInt();
  allow_downloads_ = s.value("allow_downloads", false).toBool();
  files_root_folder_ = s.value("files_root_folder", "").toString();
  files_music_extensions_ =
      s.value("files_music_extensions",
              Application::kDefaultMusicExtensionsAllowedRemotely)
          .toStringList();
  s.endGroup();

  // If we don't use an auth code, we don't need to authenticate the client.
  authenticated_ = !use_auth_code_;
  if (!authenticated_) {
    // Drop peers that do not authenticate within kAuthTimeoutMsec to free
    // client slots.
    QTimer::singleShot(kAuthTimeoutMsec, this, [this]() {
      if (!authenticated_ && !closing_) {
        DisconnectClient(cpb::remote::Not_Authenticated);
      }
    });
  }
}

RemoteClient::~RemoteClient() {
  // Tear down song_sender_ and disconnect socket signals before closing so
  // destructor cleanup never blocks in waitForDisconnected() or re-enters.
  closing_ = true;
  download_hold_.reset();
  song_sender_.reset();
  client_->disconnect(this);
  client_->close();
}

void RemoteClient::setDownloader(bool downloader) {
  downloader_ = downloader;
  // A downloader connects only to download songs, and goes when it's done.
  if (downloader && app_) {
    if (!download_hold_) {
      download_hold_ = app_->sleep_inhibitor()->Acquire(
          SleepInhibitor::Activity::RemoteTransfer);
    }
  } else {
    download_hold_.reset();
  }
}

void RemoteClient::SocketDisconnected() {
  // Emit Disconnected at most once across close() and socket teardown.
  if (disconnected_emitted_) return;
  disconnected_emitted_ = true;
  closing_ = true;
  download_hold_.reset();
  emit Disconnected(id_);
}

void RemoteClient::IncomingData() {
  // Stop draining pipelined frames as soon as closing_ becomes true.
  while (!closing_ && client_->bytesAvailable()) {
    if (!reading_protobuf_) {
      // If we have less than 4 byte, we cannot read the length. Wait for more
      // data (short reads make QDataStream yield 0 and desynchronize framing).
      if (client_->bytesAvailable() < static_cast<qint64>(sizeof(quint32))) {
        break;
      }
      // Read the length of the next message
      QDataStream s(client_);
      s >> expected_length_;

      const quint32 max_length =
          authenticated_ ? kMaxMessageLength : kMaxPreAuthMessageLength;
      // Reset framing state before closing on invalid or oversized lengths.
      if (s.status() != QDataStream::Ok || expected_length_ > max_length) {
        qLog(Debug) << "Received invalid message length:" << expected_length_;
        closing_ = true;
        buffer_.clear();
        reading_protobuf_ = false;
        client_->close();
        return;
      }

      reading_protobuf_ = true;
    }

    // Skip read(0) for 0-byte messages; break on empty reads to wait for
    // readyRead instead of spinning on bytesAvailable().
    const qint64 remaining =
        static_cast<qint64>(expected_length_) - buffer_.size();
    if (remaining > 0) {
      const QByteArray chunk = client_->read(remaining);
      if (chunk.isEmpty()) break;
      buffer_.append(chunk);
    }

    // Did we get everything?
    if (buffer_.size() == static_cast<qint64>(expected_length_)) {
      // Clear framing state before ParseMessage(), which may set closing_.
      const QByteArray payload = buffer_;
      buffer_.clear();
      reading_protobuf_ = false;
      ParseMessage(payload);
    }
  }
}

void RemoteClient::ParseMessage(const QByteArray& data) {
  // Ignore payloads left from pipelined reads once a disconnect started.
  if (closing_) return;

  cpb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) {
    qLog(Info) << "Couldn't parse data";
    return;
  }

  if (msg.type() == cpb::remote::CONNECT && use_auth_code_) {
    // While its address is locked out, the code isn't even looked at, so
    // there's nothing to learn from trying.
    if (auth_limiter_) {
      const qint64 locked_out_for = auth_limiter_->LockedOutFor(peer_address());
      if (locked_out_for > 0) {
        DisconnectClient(cpb::remote::Too_Many_Wrong_Auth_Codes,
                         locked_out_for);
        return;
      }
    }
    // Require presence because protobuf defaults an omitted auth_code to 0.
    if (!msg.request_connect().has_auth_code() ||
        msg.request_connect().auth_code() != auth_code_) {
      // The wrong code that starts a lockout says so, so the remote can say
      // to wait rather than ask for the code again.
      const qint64 lockout =
          auth_limiter_ ? auth_limiter_->RecordFailure(peer_address()) : 0;
      if (lockout > 0) {
        DisconnectClient(cpb::remote::Too_Many_Wrong_Auth_Codes, lockout);
      } else {
        DisconnectClient(cpb::remote::Wrong_Auth_Code);
      }
      return;
    } else {
      if (auth_limiter_) auth_limiter_->RecordSuccess(peer_address());
      authenticated_ = true;
    }
  }

  if (msg.type() == cpb::remote::CONNECT) {
    setDownloader(msg.request_connect().downloader());
    qDebug() << "Downloader" << downloader_;
  }

  // Check if downloads are allowed
  if (msg.type() == cpb::remote::DOWNLOAD_SONGS && !allow_downloads_) {
    DisconnectClient(cpb::remote::Download_Forbidden);
    return;
  }

  if (msg.type() == cpb::remote::DISCONNECT) {
    closing_ = true;
    client_->close();
    qDebug() << "Client disconnected";
    return;
  }

  // Check if the client has sent the correct auth code
  if (!authenticated_) {
    DisconnectClient(cpb::remote::Not_Authenticated);
    return;
  }

  // Now parse the other data
  emit Parse(msg);
}

void RemoteClient::DisconnectClient(cpb::remote::ReasonDisconnect reason,
                                    qint64 retry_after_msec) {
  // Guard against duplicate DISCONNECT frames or re-entrant closes.
  if (closing_) return;
  closing_ = true;

  cpb::remote::Message msg;
  msg.set_type(cpb::remote::DISCONNECT);

  msg.mutable_response_disconnect()->set_reason_disconnect(reason);
  if (retry_after_msec > 0) {
    msg.mutable_response_disconnect()->set_retry_after_seconds(
        (retry_after_msec + 999) / 1000);
  }
  SendDataToClient(&msg);

  // Close gracefully so the disconnect packet flushes before
  // SocketDisconnected.
  client_->close();
}

void RemoteClient::FlushBeforeDelete(int timeout_ms) {
  if (client_->state() != QAbstractSocket::UnconnectedState &&
      client_->bytesToWrite() > 0) {
    client_->waitForBytesWritten(timeout_ms);
  }
}

// Sends data to client without check if authenticated
void RemoteClient::SendDataToClient(cpb::remote::Message* msg) {
  // Set the default version
  msg->set_version(msg->default_instance().version());
  const std::string data = msg->SerializeAsString();
  WriteFramedBytes(data.data(), static_cast<qint64>(data.size()));
}

void RemoteClient::WriteFramedBytes(const char* data, qint64 length) {
  // Check if we are still connected
  if (client_->state() == QTcpSocket::ConnectedState) {
    // write the length of the data first
    QDataStream s(client_);
    s << qint32(length);
    if (downloader_) {
      // Don't use QDataStream for large files
      client_->write(data, length);
    } else {
      s.writeRawData(data, length);
    }

    // Do NOT flush data here! If the client is already disconnected, it
    // causes a SIGPIPE termination!!!
  } else {
    closing_ = true;
    client_->close();
  }
}

void RemoteClient::SendData(cpb::remote::Message* msg) {
  // Check if client is authenticated before sending the data
  if (authenticated_ && !closing_) {
    SendDataToClient(msg);
  }
}

void RemoteClient::SendRaw(const QByteArray& serialized_proto) {
  if (authenticated_ && !closing_) {
    WriteFramedBytes(serialized_proto.constData(), serialized_proto.size());
  }
}

QAbstractSocket::SocketState RemoteClient::State() { return client_->state(); }
