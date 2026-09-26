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
#include <QList>
#include <QObject>
#include <QPointer>
#include <memory>

#include "remotecontrolmessages.pb.h"

class Application;
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

 public slots:
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
};

#endif  // NETWORKREMOTE_STREAMING_RENDERERREGISTRY_H_
