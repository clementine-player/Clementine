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

#include "remotesmartplaylists.h"

#include "smartplaylists/generator.h"
#include "smartplaylists/generatormimedata.h"

RemoteSmartPlaylists::RemoteSmartPlaylists(const Hooks& hooks, QObject* parent)
    : QObject(parent), hooks_(hooks) {}

void RemoteSmartPlaylists::HandleMessage(int client_id,
                                         const QByteArray& data) {
  cpb::remote::Message msg;
  if (!msg.ParseFromArray(data.constData(), data.size())) return;

  cpb::remote::Message reply;
  switch (msg.type()) {
    case cpb::remote::REQUEST_SMART_PLAYLISTS: {
      reply.set_type(cpb::remote::SMART_PLAYLISTS);
      cpb::remote::ResponseSmartPlaylists* response =
          reply.mutable_response_smart_playlists();
      if (hooks_.list) {
        for (const cpb::remote::SmartPlaylist& smart : hooks_.list()) {
          *response->add_smart_playlists() = smart;
        }
      }
      break;
    }
    case cpb::remote::REQUEST_SMART_PLAYLIST_ADD:
      reply.set_type(cpb::remote::SMART_PLAYLIST_ADD_RESULT);
      Add(msg.request_smart_playlist_add(),
          reply.mutable_response_smart_playlist_add());
      break;
    default:
      return;
  }
  const std::string out = reply.SerializeAsString();
  emit SendToClient(client_id, QByteArray(out.data(), out.size()));
}

void RemoteSmartPlaylists::Add(
    const cpb::remote::RequestSmartPlaylistAdd& request,
    cpb::remote::ResponseSmartPlaylistAdd* response) {
  response->set_smart_playlist_id(request.smart_playlist_id());
  smart_playlists::GeneratorPtr generator;
  if (hooks_.generator) {
    generator =
        hooks_.generator(QString::fromStdString(request.smart_playlist_id()));
  }
  if (!generator) {
    response->set_result(cpb::remote::BROWSE_ADD_RESULT_GONE);
    return;
  }

  smart_playlists::GeneratorMimeData* data =
      new smart_playlists::GeneratorMimeData(generator);
  data->name_for_new_playlist_ = generator->name();
  data->override_user_settings_ = request.has_action();
  data->from_doubleclick_ = !request.has_action();
  switch (request.action()) {
    case cpb::remote::BROWSE_ADD_ACTION_PLAY_NOW:
      data->play_now_ = true;
      break;
    case cpb::remote::BROWSE_ADD_ACTION_PLAY_NEXT:
      data->enqueue_next_now_ = true;
      break;
    case cpb::remote::BROWSE_ADD_ACTION_REPLACE:
      data->clear_first_ = true;
      data->play_now_ = true;
      break;
    default:
      break;
  }
  emit AddToPlaylist(data);
  response->set_result(cpb::remote::BROWSE_ADD_RESULT_ADDED);
}
