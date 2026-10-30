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

#include <memory>

#include "airplaydevice.h"
#include "engines/gstengine.h"

class AirPlaySession;
class Application;

// The engine for one AirPlay receiver: an output of the EngineRouter.
//
// It's a GstEngine whose pipelines all end in an airplaysink, which feeds
// the one session this engine keeps with the receiver. So the equalizer,
// ReplayGain and gapless playback work as they do locally, and a new track
// carries on in the same stream.
//
// The session is opened when the engine is first asked to play, which a
// receiver may first ask its user to allow, and kept until playback stops.
// Pausing leaves the receiver to run dry, and flushes it on resuming, as
// does a seek. Clementine's volume slider sets the receiver's volume.
class AirPlayEngine : public GstEngine {
  Q_OBJECT

 public:
  AirPlayEngine(Application* app, const AirPlayDevice& device,
                QObject* parent = nullptr);
  ~AirPlayEngine();

  const AirPlayDevice& device() const { return device_; }
  // When the receiver's name, address or flags change.
  void SetDevice(const AirPlayDevice& device);

  // Engine::Base
  bool Load(const MediaPlaybackRequest& req, Engine::TrackChangeFlags change,
            bool force_stop_at_end, quint64 beginning_nanosec,
            qint64 end_nanosec) override;
  bool Play(quint64 offset_nanosec) override;
  void Stop(bool stop_after = false) override;
  void Unpause() override;
  qint64 position_nanosec() const override;
  QString output_name() const override { return device_.name; }
  QString output_icon() const override;
  void ReloadSettings() override;

 signals:
  // The receiver couldn't be paired with or played on, or went away. Not
  // when its user didn't allow Clementine: it stays the output, stopped, and
  // playing again asks again.
  void DeviceFailed();

 protected:
  void SetVolumeSW(uint percent) override;

 private slots:
  void WaitingForApproval();
  void SessionReady();
  void SessionFailed(const QString& error);
  void SessionNotAllowed(const QString& message);
  void PasswordNeeded(bool wrong);
  void PinShown();
  void PairedPersistently(const QByteArray& receiver_pairing_id,
                          const QByteArray& receiver_public_key);
  void PairingForgotten();
  void StreamFlushed();

 private:
  class Target;

  QString SinkDevice() const;
  // Opens the session with what's kept in the settings for this receiver.
  void OpenSession();
  // Where this receiver's password and pairing are kept.
  QString SettingsGroup() const;

  AirPlayDevice device_;
  AirPlaySession* session_;
  std::shared_ptr<Target> target_;
  // Play() waits for the session: where to start, or -1.
  qint64 pending_play_nanosec_;
  // Which pipelines' sinks may write to the session: those made since the
  // last manual track change, which a sink names in its device property.
  int generation_;
  // Whether pairing again was tried after the receiver forgot this sender.
  bool repaired_;
};

#endif  // AIRPLAY_AIRPLAYENGINE_H_
