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

#include "rendererregistry.h"

#include <QHostAddress>
#include <QNetworkInterface>

#include "airplay/airplaydiscovery.h"
#include "airplay/airplayengine.h"
#include "chromecast/castdiscovery.h"
#include "chromecast/chromecastengine.h"
#include "core/logging.h"
#include "engines/enginerouter.h"
#include "eventcallbacktable.h"
#include "networkremote/networkremote.h"
#include "remoteengine.h"
#include "streamitemtable.h"
#include "upnp/upnpdiscovery.h"
#include "upnp/upnpengine.h"

const char* RendererRegistry::kLocalOutputId = "local";

namespace {
bool sUpnpEnabled = false;
bool sAirPlayEnabled = false;

// Client-chosen strings kept for as long as the renderer is connected, and
// shown in output pickers.
const int kMaxRendererIdLength = 128;
const int kMaxDisplayNameLength = 64;
}  // namespace

RendererRegistry::RendererRegistry(Application* app, EngineRouter* router)
    : app_(app),
      router_(router),
      items_(new StreamItemTable),
      event_callbacks_(new EventCallbackTable) {
  connect(router_, SIGNAL(OutputsChanged()), SLOT(RouterOutputsChanged()));
}

void RendererRegistry::SetUpnpEnabled(bool enabled) { sUpnpEnabled = enabled; }

void RendererRegistry::SetAirPlayEnabled(bool enabled) {
  sAirPlayEnabled = enabled;
}

RendererRegistry::~RendererRegistry() {
  for (RemoteEngine* engine : engines_) {
    if (router_) router_->RemoveOutput(engine);
    delete engine;
  }
  for (ChromecastEngine* engine : cast_engines_) {
    if (router_) router_->RemoveOutput(engine);
    delete engine;
  }
  for (UpnpEngine* engine : upnp_engines_) {
    if (router_) router_->RemoveOutput(engine);
    delete engine;
  }
  for (AirPlayEngine* engine : airplay_engines_) {
    if (router_) router_->RemoveOutput(engine);
    delete engine;
  }
}

void RendererRegistry::StartDiscovery() {
  if (!cast_discovery_) {
    cast_discovery_ = CastDiscovery::Create(this);
    if (cast_discovery_) {
      connect(cast_discovery_, &CastDiscovery::DeviceFound, this,
              &RendererRegistry::CastDeviceFound);
      connect(cast_discovery_, &CastDiscovery::DeviceLost, this,
              &RendererRegistry::CastDeviceLost);
      cast_discovery_->Start();
    } else {
      qLog(Info) << "Can't look for Cast devices on this platform";
    }
  }

  if (sUpnpEnabled && !upnp_discovery_) {
    upnp_discovery_ = new UpnpDiscovery(this);
    connect(upnp_discovery_, &UpnpDiscovery::DeviceFound, this,
            &RendererRegistry::UpnpDeviceFound);
    connect(upnp_discovery_, &UpnpDiscovery::DeviceLost, this,
            &RendererRegistry::UpnpDeviceLost);
    upnp_discovery_->Start();
  }

  if (sAirPlayEnabled && !airplay_discovery_) {
    airplay_discovery_ =
        AirPlayDiscovery::Create(AirPlayDiscovery::kAirPlayServiceType, this);
    raop_discovery_ =
        AirPlayDiscovery::Create(AirPlayDiscovery::kRaopServiceType, this);
    if (airplay_discovery_ && raop_discovery_) {
      for (AirPlayDiscovery* discovery :
           {airplay_discovery_, raop_discovery_}) {
        connect(discovery, &AirPlayDiscovery::DeviceFound, this,
                [this, discovery](const AirPlayDevice& device) {
                  AirPlayFound(discovery, device);
                });
        connect(discovery, &AirPlayDiscovery::DeviceLost, this,
                [this, discovery](const QString& id) {
                  AirPlayLost(discovery, id);
                });
        discovery->Start();
      }
    } else {
      qLog(Info) << "Can't look for AirPlay receivers on this platform";
    }
  }
}

QString RendererRegistry::CastOutputId(const CastDevice& device) {
  return "cast:" + device.id;
}

ChromecastEngine* RendererRegistry::CastEngine(const QString& device_id) const {
  for (ChromecastEngine* engine : cast_engines_) {
    if (engine->device().id == device_id) return engine;
  }
  return nullptr;
}

void RendererRegistry::CastDeviceFound(const CastDevice& device) {
  if (ChromecastEngine* engine = CastEngine(device.id)) {
    engine->SetDevice(device);
    // Its name may have changed.
    if (router_) router_->NotifyOutputsChanged();
    return;
  }
  if (!router_) return;

  ChromecastEngine* engine =
      new ChromecastEngine(app_, device, this, items_.get(), this);
  cast_engines_ << engine;
  connect(engine, &ChromecastEngine::DeviceFailed, this,
          [this, engine]() { CastEngineFailed(engine); });
  router_->AddOutput(engine);
  qLog(Info) << "Cast device available:" << device.name;
}

void RendererRegistry::CastDeviceLost(const QString& id) {
  ChromecastEngine* engine = CastEngine(id);
  if (!engine) return;
  cast_engines_.removeOne(engine);
  qLog(Info) << "Cast device gone:" << engine->device().name;
  if (router_) router_->RemoveOutput(engine);
  engine->deleteLater();
}

void RendererRegistry::CastEngineFailed(ChromecastEngine* engine) {
  if (!router_ || router_->active_engine() != engine) return;
  // Fall back to this computer, paused, as for a renderer that goes away,
  // but keep offering the device.
  router_->RemoveOutput(engine);
  engine->Stop();
  router_->AddOutput(engine);
}

QString RendererRegistry::UpnpOutputId(const UpnpDevice& device) {
  return "upnp:" + device.id;
}

UpnpEngine* RendererRegistry::FindUpnpEngine(const QString& device_id) const {
  for (UpnpEngine* engine : upnp_engines_) {
    if (engine->device().id == device_id) return engine;
  }
  return nullptr;
}

void RendererRegistry::UpnpDeviceFound(const UpnpDevice& device) {
  if (UpnpEngine* engine = FindUpnpEngine(device.id)) {
    engine->SetDevice(device);
    // Its name may have changed.
    if (router_) router_->NotifyOutputsChanged();
    return;
  }
  if (!router_) return;

  UpnpEngine* engine = new UpnpEngine(app_, device, this, items_.get(), this);
  upnp_engines_ << engine;
  connect(engine, &UpnpEngine::DeviceFailed, this,
          [this, engine]() { UpnpEngineFailed(engine); });
  router_->AddOutput(engine);
  qLog(Info) << "UPnP renderer available:" << device.name;
}

void RendererRegistry::UpnpDeviceLost(const QString& id) {
  UpnpEngine* engine = FindUpnpEngine(id);
  if (!engine) return;
  upnp_engines_.removeOne(engine);
  qLog(Info) << "UPnP renderer gone:" << engine->device().name;
  if (router_) router_->RemoveOutput(engine);
  engine->deleteLater();
}

void RendererRegistry::UpnpEngineFailed(UpnpEngine* engine) {
  if (!router_ || router_->active_engine() != engine) return;
  // As for a Cast device.
  router_->RemoveOutput(engine);
  engine->Stop();
  router_->AddOutput(engine);
}

QString RendererRegistry::AirPlayOutputId(const AirPlayDevice& device) {
  return "airplay:" + device.id;
}

AirPlayEngine* RendererRegistry::FindAirPlayEngine(
    const QString& device_id) const {
  for (AirPlayEngine* engine : airplay_engines_) {
    if (engine->device().id == device_id) return engine;
  }
  return nullptr;
}

// Ids are MAC addresses, which _raop._tcp gives without colons and
// _airplay._tcp as it likes, so they're compared in upper case.
void RendererRegistry::AirPlayFound(AirPlayDiscovery* discovery,
                                    const AirPlayDevice& device) {
  AirPlayDevice found = device;
  found.id = device.id.toUpper();
  (discovery == raop_discovery_ ? raop_found_ : airplay_found_)[found.id] =
      found;
  AirPlayChanged(found.id);
}

void RendererRegistry::AirPlayLost(AirPlayDiscovery* discovery,
                                   const QString& id) {
  (discovery == raop_discovery_ ? raop_found_ : airplay_found_)
      .remove(id.toUpper());
  AirPlayChanged(id.toUpper());
}

void RendererRegistry::AirPlayChanged(const QString& id) {
  const auto airplay = airplay_found_.constFind(id);
  const auto raop = raop_found_.constFind(id);
  if (airplay == airplay_found_.cend() && raop == raop_found_.cend()) {
    AirPlayDeviceLost(id);
    return;
  }
  // An old Apple TV advertises _airplay._tcp for video, but plays audio
  // with AirPlay 1.
  const bool use_airplay =
      airplay != airplay_found_.cend() &&
      (raop == raop_found_.cend() || airplay->supports_airplay2());
  AirPlayDeviceFound(use_airplay ? *airplay : *raop);
}

void RendererRegistry::AirPlayDeviceFound(const AirPlayDevice& device) {
  if (AirPlayEngine* engine = FindAirPlayEngine(device.id)) {
    engine->SetDevice(device);
    // Its name may have changed.
    if (router_) router_->NotifyOutputsChanged();
    return;
  }
  if (!router_) return;

  AirPlayEngine* engine = new AirPlayEngine(app_, device, this);
  airplay_engines_ << engine;
  connect(engine, &AirPlayEngine::DeviceFailed, this,
          [this, engine]() { AirPlayEngineFailed(engine); });
  router_->AddOutput(engine);
  qLog(Info) << "AirPlay receiver available:" << device.name;
}

void RendererRegistry::AirPlayDeviceLost(const QString& id) {
  AirPlayEngine* engine = FindAirPlayEngine(id);
  if (!engine) return;
  airplay_engines_.removeOne(engine);
  qLog(Info) << "AirPlay receiver gone:" << engine->device().name;
  if (router_) router_->RemoveOutput(engine);
  engine->deleteLater();
}

void RendererRegistry::AirPlayEngineFailed(AirPlayEngine* engine) {
  if (!router_ || router_->active_engine() != engine) return;
  // Fall back to this computer, paused, as for a Cast device, but keep
  // offering the receiver.
  router_->RemoveOutput(engine);
  engine->Stop();
  router_->AddOutput(engine);
}

void RendererRegistry::RegisterRenderer(int client_id, const QByteArray& data,
                                        const QString& local_address,
                                        quint16 local_port,
                                        const QString& peer_address) {
  cpb::remote::RendererCapabilities caps;
  if (!caps.ParseFromArray(data.constData(), data.size())) return;

  const QString id = QString::fromStdString(caps.renderer_id());
  if (id.isEmpty() || id == kLocalOutputId ||
      id.size() > kMaxRendererIdLength) {
    qLog(Warning) << "Ignoring a renderer without a usable renderer_id";
    return;
  }
  if (caps.display_name().size() > kMaxDisplayNameLength) {
    caps.set_display_name(QString::fromStdString(caps.display_name())
                              .left(kMaxDisplayNameLength)
                              .toStdString());
  }
  if (!router_) return;

  RendererEndpoint endpoint;
  endpoint.client_id = client_id;
  endpoint.local_address = QHostAddress(local_address);
  endpoint.local_port = local_port;
  endpoint.peer_address = QHostAddress(peer_address);

  for (RemoteEngine* engine : engines_) {
    if (engine->client_id() == client_id) {
      Remove(engine);
      break;
    }
    if (engine->renderer_id() != id) continue;

    // A renderer reconnecting from the same address replaces its old
    // registration, which may not have noticed its connection drop yet.
    // Another device can't take over its id, and with it the SET_OUTPUT
    // requests meant for it.
    if (NormalisedAddress(engine->peer_address()) !=
        NormalisedAddress(endpoint.peer_address)) {
      qLog(Warning) << "Renderer id" << id << "is already used by"
                    << engine->peer_address().toString()
                    << "- ignoring the one from" << peer_address;
      return;
    }
    Remove(engine);
    break;
  }

  RemoteEngine* engine =
      new RemoteEngine(app_, endpoint, caps, items_.get(), this);
  engines_ << engine;
  connect(engine, SIGNAL(RendererFailed()), SLOT(EngineFailed()));
  connect(engine, SIGNAL(SendToClient(int, QByteArray)),
          SIGNAL(SendToClient(int, QByteArray)));
  router_->AddOutput(engine);

  qLog(Info) << "Renderer registered:" << engine->display_name();
}

RemoteEngine* RendererRegistry::EngineForClient(int client_id) const {
  for (RemoteEngine* engine : engines_) {
    if (engine->client_id() == client_id) return engine;
  }
  return nullptr;
}

void RendererRegistry::Remove(RemoteEngine* engine) {
  if (!engines_.removeOne(engine)) return;
  qLog(Info) << "Renderer gone:" << engine->display_name();
  if (router_) router_->RemoveOutput(engine);
  engine->deleteLater();
}

void RendererRegistry::ClientDisconnected(int client_id) {
  if (RemoteEngine* engine = EngineForClient(client_id)) Remove(engine);
}

void RendererRegistry::EngineFailed() {
  Remove(qobject_cast<RemoteEngine*>(sender()));
}

void RendererRegistry::RouterOutputsChanged() {
  emit SendToAll(OutputsMessage());
  emit OutputsChanged();
}

void RendererRegistry::HandleMessage(int client_id, const QByteArray& data) {
  cpb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) return;

  switch (msg.type()) {
    case cpb::remote::RENDERER_STATUS:
    case cpb::remote::RENDERER_TRACK_ENDED:
    case cpb::remote::RENDERER_ERROR:
      if (RemoteEngine* engine = EngineForClient(client_id)) {
        engine->HandleMessage(msg);
      }
      break;

    case cpb::remote::REQUEST_OUTPUTS:
      emit SendToClient(client_id, OutputsMessage());
      break;

    case cpb::remote::SET_OUTPUT:
      SetOutput(QString::fromStdString(msg.request_set_output().output_id()));
      break;

    default:
      break;
  }
}

QByteArray RendererRegistry::OutputsMessage() const {
  cpb::remote::Message msg;
  msg.set_type(cpb::remote::OUTPUTS);
  cpb::remote::ResponseOutputs* outputs = msg.mutable_response_outputs();

  const bool local_active = !router_ || router_->is_local();
  cpb::remote::Output* local = outputs->add_outputs();
  local->set_output_id(kLocalOutputId);
  local->set_display_name(NetworkRemote::RemoteName().toStdString());
  local->set_state(local_active ? cpb::remote::OUTPUT_STATE_ACTIVE
                                : cpb::remote::OUTPUT_STATE_AVAILABLE);

  for (RemoteEngine* engine : engines_) {
    cpb::remote::Output* output = outputs->add_outputs();
    output->set_output_id(engine->renderer_id().toStdString());
    output->set_display_name(engine->display_name().toStdString());
    output->set_state(router_ && router_->active_engine() == engine
                          ? cpb::remote::OUTPUT_STATE_ACTIVE
                          : cpb::remote::OUTPUT_STATE_AVAILABLE);
  }
  for (ChromecastEngine* engine : cast_engines_) {
    cpb::remote::Output* output = outputs->add_outputs();
    output->set_output_id(CastOutputId(engine->device()).toStdString());
    output->set_display_name(engine->device().name.toStdString());
    output->set_state(router_ && router_->active_engine() == engine
                          ? cpb::remote::OUTPUT_STATE_ACTIVE
                          : cpb::remote::OUTPUT_STATE_AVAILABLE);
  }
  for (UpnpEngine* engine : upnp_engines_) {
    cpb::remote::Output* output = outputs->add_outputs();
    output->set_output_id(UpnpOutputId(engine->device()).toStdString());
    output->set_display_name(engine->device().name.toStdString());
    output->set_state(router_ && router_->active_engine() == engine
                          ? cpb::remote::OUTPUT_STATE_ACTIVE
                          : cpb::remote::OUTPUT_STATE_AVAILABLE);
  }
  for (AirPlayEngine* engine : airplay_engines_) {
    cpb::remote::Output* output = outputs->add_outputs();
    output->set_output_id(AirPlayOutputId(engine->device()).toStdString());
    output->set_display_name(engine->device().name.toStdString());
    output->set_state(router_ && router_->active_engine() == engine
                          ? cpb::remote::OUTPUT_STATE_ACTIVE
                          : cpb::remote::OUTPUT_STATE_AVAILABLE);
  }

  const std::string data = msg.SerializeAsString();
  return QByteArray(data.data(), static_cast<int>(data.size()));
}

bool RendererRegistry::SetOutput(const QString& output_id) {
  if (!router_) return false;
  if (output_id == kLocalOutputId) {
    router_->SetLocalOutput();
    return true;
  }
  for (RemoteEngine* engine : engines_) {
    if (engine->renderer_id() == output_id) {
      router_->SetOutput(engine);
      return true;
    }
  }
  for (ChromecastEngine* engine : cast_engines_) {
    if (CastOutputId(engine->device()) == output_id) {
      router_->SetOutput(engine);
      return true;
    }
  }
  for (UpnpEngine* engine : upnp_engines_) {
    if (UpnpOutputId(engine->device()) == output_id) {
      router_->SetOutput(engine);
      return true;
    }
  }
  for (AirPlayEngine* engine : airplay_engines_) {
    if (AirPlayOutputId(engine->device()) == output_id) {
      router_->SetOutput(engine);
      return true;
    }
  }
  qLog(Warning) << "No output called" << output_id;
  return false;
}

void RendererRegistry::SetServing(const Serving& serving) {
  serving_ = serving;
  if (serving.enabled) {
    qLog(Debug) << "Media for other devices is served on port" << serving.port;
  }
  emit ServingChanged();
}

namespace {

QList<QNetworkAddressEntry> UpInterfaces() {
  QList<QNetworkAddressEntry> interfaces;
  for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
    if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
    interfaces << iface.addressEntries();
  }
  return interfaces;
}

}  // namespace

RendererRegistry::Reach RendererRegistry::ReachFor(
    const QHostAddress& device, const Serving& serving,
    const QList<QNetworkAddressEntry>& interfaces, QHostAddress* local_ret) {
  if (!serving.enabled) return Reach::RemoteOff;
  if (serving.port == 0) return Reach::NotListening;

  const QHostAddress local =
      LocalAddressFor(device, serving.addresses, interfaces);
  if (local.isNull()) return Reach::NotOnNetwork;

  // As NetworkRemote decides when the device connects.
  if (serving.only_local &&
      !NetworkRemote::IsLocalClient(
          NormalisedAddress(device), local,
          serving.addresses.isEmpty()
              ? NetworkRemote::Listening::OnAllAddresses
              : NetworkRemote::Listening::OnChosenAddress)) {
    return Reach::NotLocal;
  }

  if (local_ret) *local_ret = local;
  return Reach::Reachable;
}

RendererRegistry::Reach RendererRegistry::ReachFor(
    const QHostAddress& device) const {
  return ReachFor(device, serving_, UpInterfaces());
}

RendererRegistry::Reach RendererRegistry::ReachFor(Engine::Base* output) const {
  const QHostAddress address = output->output_address();
  if (address.isNull()) return Reach::Reachable;
  return ReachFor(address);
}

QString RendererRegistry::ReachMessage(Reach reach, const QString& name) const {
  switch (reach) {
    case Reach::Reachable:
      break;
    case Reach::RemoteOff:
      // TRANSLATORS: Shown when the user picks a Chromecast or other device
      // to play on, and it can't get the music. Such devices download the
      // audio from Clementine through the network remote, the server the
      // phone remote apps also connect to (Settings, Network Remote). %1 is
      // the device's name, such as "Living Room TV".
      return tr("%1 gets its music from Clementine through the network "
                "remote, which is turned off.")
          .arg(name);
    case Reach::NotListening:
      // TRANSLATORS: Shown when a Chromecast or other device the user picks
      // can't get its music from Clementine's network remote: the remote is
      // on, but couldn't open its port, usually because another program has
      // it. %1 is the device's name; %2 is the port number, such as 5500.
      return tr("%1 gets its music from Clementine through the network "
                "remote, which couldn't listen on port %2. Another program "
                "may be using it.")
          .arg(name)
          .arg(serving_.port == 0 ? NetworkRemote::kDefaultServerPort
                                  : serving_.port);
    case Reach::NotOnNetwork:
      // TRANSLATORS: Shown when a Chromecast or other device the user picks
      // can't get its music from Clementine's network remote: the remote only
      // listens on addresses the user chose (Settings, Network Remote), and
      // none is on the device's network. %1 is the device's name.
      return tr("%1 gets its music from Clementine through the network "
                "remote, which doesn't listen on an address in %1's network.")
          .arg(name);
    case Reach::NotLocal:
      // TRANSLATORS: Shown when a Chromecast or other device the user picks
      // can't get its music from Clementine's network remote: the remote's
      // "Only allow connections from the local network" setting refuses it,
      // since its address isn't a private one. %1 is the device's name.
      return tr("%1 gets its music from Clementine through the network "
                "remote, which only allows connections from the local "
                "network, and %1 isn't on it.")
          .arg(name);
  }
  return QString();
}

QUrl RendererRegistry::MediaBaseUrl(const QHostAddress& device) const {
  QHostAddress local;
  if (ReachFor(device, serving_, UpInterfaces(), &local) != Reach::Reachable) {
    return QUrl();
  }

  QUrl url;
  url.setScheme("http");
  url.setHost(local.toString());
  url.setPort(serving_.port);
  return url;
}

QHostAddress RendererRegistry::LocalAddressFor(
    const QHostAddress& device, const QList<QHostAddress>& listening,
    const QList<QNetworkAddressEntry>& interfaces) {
  const QHostAddress peer = NormalisedAddress(device);
  for (const QNetworkAddressEntry& entry : interfaces) {
    const QHostAddress local = NormalisedAddress(entry.ip());
    if (local.protocol() != peer.protocol() || entry.prefixLength() < 0 ||
        !peer.isInSubnet(local, entry.prefixLength())) {
      continue;
    }
    if (listening.isEmpty()) return local;
    for (const QHostAddress& address : listening) {
      if (NormalisedAddress(address) == local) return local;
    }
  }
  return QHostAddress();
}
