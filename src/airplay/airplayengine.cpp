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

#include "airplayengine.h"

#include "airplaysession.h"
#include "core/logging.h"

AirPlayEngine::AirPlayEngine(const AirPlayDevice& device, QObject* parent)
    : device_(device), session_(nullptr) {
  AirPlaySession::Options options;
  options.sender_id = AirPlaySession::SavedSenderId();
  session_ = new AirPlaySession(options, this);
  setParent(parent);
  connect(session_, SIGNAL(WaitingForApproval()), SLOT(WaitingForApproval()));
  connect(session_, SIGNAL(SetUp()), SLOT(SessionSetUp()));
  connect(session_, SIGNAL(Failed(QString)), SLOT(SessionFailed(QString)));
}

AirPlayEngine::~AirPlayEngine() {}

void AirPlayEngine::SetDevice(const AirPlayDevice& device) { device_ = device; }

QString AirPlayEngine::output_icon() const {
  if (device_.model.startsWith("AppleTV")) return "tv";
  return "speaker";
}

bool AirPlayEngine::Load(const MediaPlaybackRequest& req,
                         Engine::TrackChangeFlags change,
                         bool force_stop_at_end, quint64 beginning_nanosec,
                         qint64 end_nanosec) {
  Engine::Base::Load(req, change, force_stop_at_end, beginning_nanosec,
                     end_nanosec);
  return true;
}

bool AirPlayEngine::Play(quint64) {
  if (!session_->is_open()) session_->Open(device_);
  return true;
}

void AirPlayEngine::Stop(bool) { session_->Close(); }

void AirPlayEngine::WaitingForApproval() {
  // TRANSLATORS: Shown while an AirPlay receiver, such as a Mac, asks its
  // user whether to let Clementine play on it. %1 is what Clementine calls
  // itself there, such as "Clementine on Laptop"; %2 is the receiver's name.
  emit StatusText(tr("Allow %1 on %2 to play there")
                      .arg(AirPlaySession::DefaultName(), device_.name));
}

void AirPlayEngine::SessionSetUp() {
  session_->Close();
  // TRANSLATORS: %1 is the name of an AirPlay speaker, such as "Living Room".
  emit Error(tr("Clementine can pair with %1, but can't play on AirPlay "
                "receivers yet")
                 .arg(device_.name));
  emit DeviceFailed();
}

void AirPlayEngine::SessionFailed(const QString& error) {
  emit Error(error);
  emit DeviceFailed();
}
