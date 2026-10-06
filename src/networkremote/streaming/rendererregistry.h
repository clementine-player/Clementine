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

#ifndef NETWORKREMOTE_STREAMING_RENDERERREGISTRY_H_
#define NETWORKREMOTE_STREAMING_RENDERERREGISTRY_H_

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QNetworkAddressEntry>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <memory>

#include "remotecontrolmessages.pb.h"

class Application;
class CastDiscovery;
class ChromecastEngine;
struct CastDevice;
class EngineRouter;
class RemoteEngine;
class StreamItemTable;
struct RendererEndpoint;

// Keeps a RemoteEngine for every connected renderer, makes each one an output
// of the EngineRouter, and answers the output-related remote messages.
//
// Lives on the main thread with the Player. The network remote's thread
// talks to it only through queued slots and signals carrying serialized
// messages, and client ids rather than RemoteClient pointers.
class RendererRegistry : public QObject {
  Q_OBJECT

 public:
  static const char* kLocalOutputId;

  RendererRegistry(Application* app, EngineRouter* router);
  ~RendererRegistry();

  // Shared with MediaHttpServer, which reads it from another thread.
  std::shared_ptr<StreamItemTable> items() const { return items_; }

  // Moves playback to the output with this id ("local" or a renderer id).
  bool SetOutput(const QString& output_id);

  // A serialized OUTPUTS message.
  QByteArray OutputsMessage() const;

  // Where the network remote listens for HTTP: |port|, on |addresses|, or on
  // every address if that's empty. NetworkRemote calls this once it's
  // listening.
  void SetListening(quint16 port, const QList<QHostAddress>& addresses);

  // Where a device that doesn't connect to the remote itself, such as a Cast
  // device, fetches media from: "http://address:port", with the address that
  // the remote listens on in the same network as |device|. Empty if there's
  // no such address.
  QUrl MediaBaseUrl(const QHostAddress& device) const;

  // The local address in |interfaces| that's in the same subnet as |device|
  // and in |listening| (anything, if that's empty), or a null address.
  static QHostAddress LocalAddressFor(
      const QHostAddress& device, const QList<QHostAddress>& listening,
      const QList<QNetworkAddressEntry>& interfaces);

 public slots:
  // Starts looking for Cast devices, and offers them as outputs for as long
  // as this registry exists.
  void StartCastDiscovery();
  // A client connected with RequestConnect.renderer set. |caps| is a
  // serialized RendererCapabilities.
  void RegisterRenderer(int client_id, const QByteArray& caps,
                        const QString& local_address, quint16 local_port,
                        const QString& peer_address);
  // A renderer or output message from a client, serialized.
  void HandleMessage(int client_id, const QByteArray& data);
  void ClientDisconnected(int client_id);

 signals:
  // A serialized cpb::remote::Message for one client, or for all of them.
  void SendToClient(int client_id, const QByteArray& data);
  void SendToAll(const QByteArray& data);

  // Outputs were added or removed, or the active one changed.
  void OutputsChanged();

 private slots:
  void EngineFailed();
  void RouterOutputsChanged();

 private:
  RemoteEngine* EngineForClient(int client_id) const;
  void Remove(RemoteEngine* engine);

  Application* app_;
  QPointer<EngineRouter> router_;
  std::shared_ptr<StreamItemTable> items_;
  QList<RemoteEngine*> engines_;

  // Output ids for Cast devices.
  static QString CastOutputId(const CastDevice& device);
  void CastDeviceFound(const CastDevice& device);
  void CastDeviceLost(const QString& id);
  void CastEngineFailed(ChromecastEngine* engine);
  ChromecastEngine* CastEngine(const QString& device_id) const;
  QList<ChromecastEngine*> cast_engines_;
  // Owned; null until StartCastDiscovery, or if this platform has none.
  CastDiscovery* cast_discovery_ = nullptr;

  // 0 until NetworkRemote says where it listens.
  quint16 listening_port_;
  QList<QHostAddress> listening_addresses_;
};

#endif  // NETWORKREMOTE_STREAMING_RENDERERREGISTRY_H_
