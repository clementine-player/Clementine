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
#include <QMap>
#include <QNetworkAddressEntry>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <memory>

#include "airplay/airplaydevice.h"
#include "remotecontrolmessages.pb.h"

class AirPlayDiscovery;
class AirPlayEngine;
class Application;
class CastDiscovery;
class ChromecastEngine;
struct CastDevice;
class EngineRouter;
class RemoteEngine;
class UpnpDiscovery;
class UpnpEngine;
struct UpnpDevice;
class EventCallbackTable;
class StreamItemTable;
struct RendererEndpoint;

namespace Engine {
class Base;
}

// Keeps a RemoteEngine for every connected renderer, and an engine for every
// Cast device, UPnP renderer and AirPlay receiver found, makes each one an
// output of the EngineRouter, and answers the output-related remote messages.
//
// It exists whether or not the network remote is on, so that devices are
// offered as outputs either way. They fetch media from the network remote,
// though, so whether one can play depends on how that's set up; see Reach.
//
// Lives on the main thread with the Player. The network remote's thread
// talks to it only through queued slots and signals carrying serialized
// messages, and client ids rather than RemoteClient pointers.
class RendererRegistry : public QObject {
  Q_OBJECT

 public:
  static const char* kLocalOutputId;

  // Whether to offer UPnP MediaRenderers as outputs too, from --upnp. Set
  // before discovery starts.
  static void SetUpnpEnabled(bool enabled);
  // Whether to offer AirPlay receivers as outputs too, from --airplay. Set
  // before discovery starts.
  static void SetAirPlayEnabled(bool enabled);

  RendererRegistry(Application* app, EngineRouter* router);
  ~RendererRegistry();

  // Shared with MediaHttpServer, which reads it from another thread.
  std::shared_ptr<StreamItemTable> items() const { return items_; }
  // Where renderers' events go, for MediaHttpServer to take.
  std::shared_ptr<EventCallbackTable> event_callbacks() const {
    return event_callbacks_;
  }

  // Moves playback to the output with this id ("local" or a renderer id).
  bool SetOutput(const QString& output_id);

  // A serialized OUTPUTS message.
  QByteArray OutputsMessage() const;

  // How the network remote is serving media. NetworkRemote sets it whenever
  // it starts or stops.
  struct Serving {
    // Whether the network remote is turned on.
    bool enabled = false;
    // The port it listens on, or 0 if it couldn't listen anywhere.
    quint16 port = 0;
    // The addresses it listens on, or empty for every address.
    QList<QHostAddress> addresses;
    // Whether it only allows connections from the local network.
    bool only_local = true;
  };
  void SetServing(const Serving& serving);
  const Serving& serving() const { return serving_; }

  // Whether a device that doesn't connect to the remote itself, such as a
  // Cast device, can fetch media from it, or else why not.
  enum class Reach {
    Reachable,
    // The network remote is off.
    RemoteOff,
    // It's on, but couldn't listen on its port.
    NotListening,
    // It doesn't listen on an address in the device's network.
    NotOnNetwork,
    // It only allows connections from the local network, and the device
    // doesn't count as on it.
    NotLocal,
  };
  Reach ReachFor(const QHostAddress& device) const;
  // For an output of the router, by its output_address: Reachable for this
  // computer, and for renderers, which connect to the remote themselves.
  Reach ReachFor(Engine::Base* output) const;
  // ReachFor with the local network |interfaces|, and the local address the
  // device would fetch from in |local|, when Reachable.
  static Reach ReachFor(const QHostAddress& device, const Serving& serving,
                        const QList<QNetworkAddressEntry>& interfaces,
                        QHostAddress* local = nullptr);

  // Why a device called |name| can't play, for the user, or empty when
  // |reach| is Reachable.
  QString ReachMessage(Reach reach, const QString& name) const;

  // Where a device that doesn't connect to the remote itself fetches media
  // from: "http://address:port", with the address that the remote listens on
  // in the same network as |device|. Empty unless the device is Reachable.
  QUrl MediaBaseUrl(const QHostAddress& device) const;

  // The local address in |interfaces| that's in the same subnet as |device|
  // and in |listening| (anything, if that's empty), or a null address.
  static QHostAddress LocalAddressFor(
      const QHostAddress& device, const QList<QHostAddress>& listening,
      const QList<QNetworkAddressEntry>& interfaces);

 public slots:
  // Starts looking for Cast devices, UPnP MediaRenderers if SetUpnpEnabled,
  // and AirPlay receivers if SetAirPlayEnabled, and offers them as outputs
  // for as long as this registry exists.
  void StartDiscovery();
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
  // SetServing was called, so ReachFor may say something else.
  void ServingChanged();

 private slots:
  void EngineFailed();
  void RouterOutputsChanged();

 private:
  RemoteEngine* EngineForClient(int client_id) const;
  void Remove(RemoteEngine* engine);

  Application* app_;
  QPointer<EngineRouter> router_;
  std::shared_ptr<StreamItemTable> items_;
  std::shared_ptr<EventCallbackTable> event_callbacks_;
  QList<RemoteEngine*> engines_;

  // Output ids for Cast devices.
  static QString CastOutputId(const CastDevice& device);
  void CastDeviceFound(const CastDevice& device);
  void CastDeviceLost(const QString& id);
  void CastEngineFailed(ChromecastEngine* engine);
  ChromecastEngine* CastEngine(const QString& device_id) const;
  QList<ChromecastEngine*> cast_engines_;
  // Owned; null until StartDiscovery, or if this platform has none.
  CastDiscovery* cast_discovery_ = nullptr;

  // Output ids for UPnP renderers.
  static QString UpnpOutputId(const UpnpDevice& device);
  void UpnpDeviceFound(const UpnpDevice& device);
  void UpnpDeviceLost(const QString& id);
  void UpnpEngineFailed(UpnpEngine* engine);
  UpnpEngine* FindUpnpEngine(const QString& device_id) const;
  QList<UpnpEngine*> upnp_engines_;
  // Owned; null until StartDiscovery, or without --upnp.
  UpnpDiscovery* upnp_discovery_ = nullptr;

  // Output ids for AirPlay receivers.
  static QString AirPlayOutputId(const AirPlayDevice& device);
  // What |discovery| found or lost.
  void AirPlayFound(AirPlayDiscovery* discovery, const AirPlayDevice& device);
  void AirPlayLost(AirPlayDiscovery* discovery, const QString& id);
  // Offers the receiver |id| as both discoveries have it, or stops.
  void AirPlayChanged(const QString& id);
  void AirPlayDeviceFound(const AirPlayDevice& device);
  void AirPlayDeviceLost(const QString& id);
  void AirPlayEngineFailed(AirPlayEngine* engine);
  AirPlayEngine* FindAirPlayEngine(const QString& device_id) const;
  QList<AirPlayEngine*> airplay_engines_;
  // Owned; null until StartDiscovery, without --airplay, or if this platform
  // has none. One looks for _airplay._tcp and the other for _raop._tcp.
  AirPlayDiscovery* airplay_discovery_ = nullptr;
  AirPlayDiscovery* raop_discovery_ = nullptr;
  // What each found, by id. An AirPlay 2 receiver advertises both, and is
  // played to as _airplay._tcp says; an AirPlay 1 receiver only _raop._tcp.
  QMap<QString, AirPlayDevice> airplay_found_;
  QMap<QString, AirPlayDevice> raop_found_;

  Serving serving_;
};

#endif  // NETWORKREMOTE_STREAMING_RENDERERREGISTRY_H_
