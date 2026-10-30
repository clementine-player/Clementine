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

#ifndef AIRPLAY_AIRPLAYENGINE_H_
#define AIRPLAY_AIRPLAYENGINE_H_

#include "airplaydevice.h"
#include "engines/enginebase.h"

class AirPlaySession;

// The engine for one AirPlay receiver: an output of the EngineRouter.
//
// For now it only connects, pairs and sets up a session when asked to play,
// which a receiver may first ask its user to allow, then reports that it
// can't play yet.
class AirPlayEngine : public Engine::Base {
  Q_OBJECT

 public:
  explicit AirPlayEngine(const AirPlayDevice& device,
                         QObject* parent = nullptr);
  ~AirPlayEngine();

  const AirPlayDevice& device() const { return device_; }
  // When the receiver's name, address or flags change.
  void SetDevice(const AirPlayDevice& device);

  // Engine::Base
  bool Init() override { return true; }
  void StartPreloading(const MediaPlaybackRequest&, bool, qint64,
                       qint64) override {}
  bool Load(const MediaPlaybackRequest& req, Engine::TrackChangeFlags change,
            bool force_stop_at_end, quint64 beginning_nanosec,
            qint64 end_nanosec) override;
  bool Play(quint64 offset_nanosec) override;
  void Stop(bool stop_after = false) override;
  void Pause() override {}
  void Unpause() override {}
  void Seek(quint64) override {}
  Engine::State state() const override { return Engine::Empty; }
  qint64 position_nanosec() const override { return 0; }
  qint64 length_nanosec() const override { return 0; }
  QString output_name() const override { return device_.name; }
  QString output_icon() const override;

 signals:
  // The receiver couldn't be paired with, or can't be played on.
  void DeviceFailed();

 protected:
  void SetVolumeSW(uint) override {}

 private slots:
  void WaitingForApproval();
  void SessionSetUp();
  void SessionFailed(const QString& error);

 private:
  AirPlayDevice device_;
  AirPlaySession* session_;
};

#endif  // AIRPLAY_AIRPLAYENGINE_H_
