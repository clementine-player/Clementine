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

#include "core/logging.h"
#include "engines/enginerouter.h"
#include "remoteengine.h"
#include "streamitemtable.h"

const char* RendererRegistry::kLocalOutputId = "local";

namespace {
// Client-chosen strings kept for as long as the renderer is connected, and
// shown in output pickers.
const int kMaxRendererIdLength = 128;
const int kMaxDisplayNameLength = 64;
}  // namespace

RendererRegistry::RendererRegistry(Application* app, EngineRouter* router)
    : app_(app), router_(router), items_(new StreamItemTable) {
  connect(router_, SIGNAL(OutputsChanged()), SLOT(RouterOutputsChanged()));
}

RendererRegistry::~RendererRegistry() {
  for (RemoteEngine* engine : engines_) {
    if (router_) router_->RemoveOutput(engine);
    delete engine;
  }
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
  local->set_display_name(tr("This computer").toStdString());
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
  qLog(Warning) << "No output called" << output_id;
  return false;
}
