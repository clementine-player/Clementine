#include "httpconnection.h"

#include <QBuffer>
#include <QStringList>
#include <QTcpSocket>

#include <qjson/serializer.h>

#include "core/player.h"
#include "playlist/playlistitem.h"

HttpConnection::HttpConnection(Player* player, QTcpSocket* socket)
    : QObject(socket),
      socket_(socket),
      state_(WaitingForRequest),
      player_(player),
      cover_loader_(new AlbumCoverLoader(this)) {
  connect(socket_, SIGNAL(readyRead()), SLOT(ReadyRead()));
  connect(cover_loader_, SIGNAL(ImageLoaded(quint64, const QImage&)),
      SLOT(ImageReady(quint64, const QImage&)));
  connect(socket_, SIGNAL(disconnected()), socket_, SLOT(deleteLater()));

  cover_loader_->SetDesiredHeight(400);
  cover_loader_->SetScaleOutputImage(true);
}

void HttpConnection::ReadyRead() {
  switch (state_) {
    case WaitingForRequest:
      if (socket_->canReadLine()) {
        QString line = socket_->readLine();
        if (ParseRequest(line)) {
          state_ = WaitingForHeaders;
        } else {
          state_ = Error;
        }
      }
      break;
    case WaitingForHeaders:
      while (socket_->canReadLine()) {
        QByteArray line = socket_->readLine();
        if (line == "\r\n") {
          DoRequest();
          state_ = Finished;
        } else {
          // TODO: Parse headers.
        }
      }
      break;

    default:
      break;
  }

  if (state_ == Error) {
    socket_->close();
    socket_->deleteLater();
    return;
  }

  if (socket_->canReadLine()) {
    ReadyRead();
  }
}

bool HttpConnection::ParseRequest(const QString& line) {
  QStringList tokens = line.split(' ', QString::SkipEmptyParts);
  if (tokens.length() != 3) {
    return false;
  }
  const QString& method = tokens[0];
  if (method != "GET") {
    return false;
  }

  method_ = QNetworkAccessManager::GetOperation;
  path_ = tokens[1];
  return true;
}

void HttpConnection::DoRequest() {
  PlaylistItemPtr current_item = player_->GetCurrentItem();
  if (current_item) {
    quint64 id = cover_loader_->LoadImageAsync(current_item->Metadata());
    pending_tasks_[id] = current_item->Metadata();
    return;
  }

  SendHeaders();

  QVariantMap state = GetState();

  QJson::Serializer serializer;
  QString output = serializer.serialize(state);
  socket_->write(output.toUtf8());

  socket_->disconnectFromHost();
}

void HttpConnection::ImageReady(quint64 id, const QImage& image) {
  Song song = pending_tasks_.take(id);
  SendSongInfo(song, image);
}

void HttpConnection::SendHeaders() {
  socket_->write("HTTP/1.0 200 OK\r\n");
  socket_->write("Content-type: application/json\r\n");
  socket_->write("\r\n");
}

QVariantMap HttpConnection::GetState() {
  QVariantMap json;
  json.insert("state", player_->GetState());
  json.insert("volume", player_->GetVolume());
  return json;
}

void HttpConnection::SendSongInfo(Song song, const QImage& cover) {
  SendHeaders();

  QVariantMap metadata;
  metadata.insert("title", song.title());
  metadata.insert("album", song.album());
  metadata.insert("artist", song.artist());
  if (!cover.isNull()) {
    QByteArray cover_data;
    {
      QBuffer cover_buffer(&cover_data);
      cover_buffer.open(QIODevice::WriteOnly);
      cover.save(&cover_buffer, "PNG");
    }
    metadata.insert("cover", cover_data.toBase64());
  }

  QVariantMap state = GetState();
  state.insert("song", metadata);

  QJson::Serializer serializer;
  QString output = serializer.serialize(state);
  socket_->write(output.toUtf8());
  socket_->disconnectFromHost();
}
