/* This file is part of Clementine.
   Copyright 2012, Andreas Muttscheller <asfa194@gmail.com>

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

#include "core/logging.h"

#include "networkremote.h"
#include "networkremotehelper.h"

NetworkRemoteHelper* NetworkRemoteHelper::sInstance = nullptr;

NetworkRemoteHelper::NetworkRemoteHelper(Application* app) : app_(app) {
  app_ = app;
  connect(this, SIGNAL(ReloadSettingsSig()), app_->network_remote(),
          SLOT(ReloadSettings()));
  connect(this, SIGNAL(StartServerSig()), app_->network_remote(),
          SLOT(StartServer()));
  connect(this, SIGNAL(SetupServerSig()), app_->network_remote(),
          SLOT(SetupServer()));

  // Start the server once the playlistmanager is initialized
  connect(app_->playlist_manager(), SIGNAL(PlaylistManagerInitialized()), this,
          SLOT(StartServer()));

  sInstance = this;
}

NetworkRemoteHelper::~NetworkRemoteHelper() {}

void NetworkRemoteHelper::StartServer() {
  emit SetupServerSig();
  emit StartServerSig();
}

void NetworkRemoteHelper::ReloadSettings() { emit ReloadSettingsSig(); }

// For using in Settingsdialog, we haven't the appication there
NetworkRemoteHelper* NetworkRemoteHelper::Instance() {
  if (!sInstance) {
    // normally he shouldn't go here. Only for safety
    return nullptr;
  }
  return sInstance;
}
