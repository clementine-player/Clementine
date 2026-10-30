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

#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>

#include "airplaysession.h"
#include "airplaysink.h"
#include "airplaystream.h"
#include "core/logging.h"

// What the engine's airplaysinks write to, from GStreamer's streaming
// thread: the session's stream while it's ready, else nowhere.
class AirPlayEngine::Target : public AirPlaySinkTarget {
 public:
  explicit Target(AirPlayEngine* engine) : engine_(engine), stream_(nullptr) {}

  void SetStream(AirPlayStream* stream) {
    QMutexLocker l(&mutex_);
    stream_ = stream;
  }

  void Write(const char* pcm, int size) override {
    QMutexLocker l(&mutex_);
    if (stream_) stream_->Write(pcm, size);
  }

  void Flushed() override {
    QMetaObject::invokeMethod(engine_, "StreamFlushed", Qt::QueuedConnection);
  }

 private:
  QPointer<AirPlayEngine> engine_;
  QMutex mutex_;
  AirPlayStream* stream_;
};

AirPlayEngine::AirPlayEngine(Application* app, const AirPlayDevice& device,
                             QObject* parent)
    : GstEngine(app),
      device_(device),
      session_(nullptr),
      target_(new Target(this)),
      pending_play_nanosec_(-1),
      generation_(0) {
  setParent(parent);
  AirPlaySession::Options options;
  options.sender_id = AirPlaySession::SavedSenderId();
  session_ = new AirPlaySession(options, this);
  connect(session_, SIGNAL(WaitingForApproval()), SLOT(WaitingForApproval()));
  connect(session_, SIGNAL(Ready()), SLOT(SessionReady()));
  connect(session_, SIGNAL(Failed(QString)), SLOT(SessionFailed(QString)));
  connect(session_, SIGNAL(NotAllowed(QString)),
          SLOT(SessionNotAllowed(QString)));
  connect(session_, &AirPlaySession::StreamEnding, this,
          [this]() { target_->SetStream(nullptr); });
  AirPlaySink::SetTarget(SinkDevice(), target_);
  ReloadSettings();
}

AirPlayEngine::~AirPlayEngine() {
  AirPlaySink::RemoveTarget(SinkDevice());
  target_->SetStream(nullptr);
}

void AirPlayEngine::SetDevice(const AirPlayDevice& device) { device_ = device; }

QString AirPlayEngine::SinkDevice() const {
  return QString("%1#%2").arg(device_.id).arg(generation_);
}

QString AirPlayEngine::output_icon() const {
  if (device_.model.startsWith("AppleTV")) return "tv";
  return "speaker";
}

void AirPlayEngine::ReloadSettings() {
  GstEngine::ReloadSettings();
  // Whatever the local output is.
  SetOutput(AirPlaySink::kFactoryName, SinkDevice());
}

bool AirPlayEngine::Load(const MediaPlaybackRequest& req,
                         Engine::TrackChangeFlags change,
                         bool force_stop_at_end, quint64 beginning_nanosec,
                         qint64 end_nanosec) {
  EnsureInitialised();
  AirPlaySink::RegisterElement();
  if (!session_->is_open()) session_->Open(device_);
  if (change & Engine::Manual && session_->is_ready()) {
    // The user skipped: what the receiver has buffered of the last track is
    // dropped, and the last track's pipeline, which may still be rendering,
    // no longer reaches the session. A track that follows on by itself
    // carries on in the stream instead.
    AirPlaySink::RemoveTarget(SinkDevice());
    ++generation_;
    AirPlaySink::SetTarget(SinkDevice(), target_);
    SetOutput(AirPlaySink::kFactoryName, SinkDevice());
    session_->Flush();
  }
  return GstEngine::Load(req, change, force_stop_at_end, beginning_nanosec,
                         end_nanosec);
}

bool AirPlayEngine::Play(quint64 offset_nanosec) {
  if (!session_->is_ready()) {
    // Plays once the receiver is ready, which it may first ask its user.
    pending_play_nanosec_ = offset_nanosec;
    if (!session_->is_open()) session_->Open(device_);
    return true;
  }
  return GstEngine::Play(offset_nanosec);
}

void AirPlayEngine::WaitingForApproval() {
  // TRANSLATORS: Shown while an AirPlay receiver, such as a Mac, asks its
  // user whether to let Clementine play on it. %1 is what Clementine calls
  // itself there, such as "Clementine on Laptop"; %2 is the receiver's name.
  emit StatusText(tr("Allow %1 on %2 to play there")
                      .arg(AirPlaySession::DefaultName(), device_.name));
}

void AirPlayEngine::SessionReady() {
  target_->SetStream(session_->stream());
  if (pending_play_nanosec_ >= 0) {
    const quint64 offset = pending_play_nanosec_;
    pending_play_nanosec_ = -1;
    GstEngine::Play(offset);
  }
}

void AirPlayEngine::Stop(bool stop_after) {
  pending_play_nanosec_ = -1;
  GstEngine::Stop(stop_after);
  target_->SetStream(nullptr);
  session_->Close();
}

// The receiver ran dry while paused: it plays from what comes next.
void AirPlayEngine::Unpause() {
  session_->Flush();
  GstEngine::Unpause();
}

void AirPlayEngine::StreamFlushed() { session_->Flush(); }

void AirPlayEngine::SetVolumeSW(uint percent) {
  // The receiver's volume, rather than the pipeline's.
  session_->SetVolume(percent);
}

qint64 AirPlayEngine::position_nanosec() const {
  // The receiver plays this far behind what's been sent.
  const qint64 latency = qint64(AirPlayStream::kLatencyFrames) * kNsecPerSec /
                         AirPlayStream::kSampleRate;
  return qMax<qint64>(0, GstEngine::position_nanosec() - latency);
}

void AirPlayEngine::SessionNotAllowed(const QString& message) {
  pending_play_nanosec_ = -1;
  target_->SetStream(nullptr);
  GstEngine::Stop();
  // TRANSLATORS: %1 is a message such as "Living Room didn't allow
  // Clementine on Laptop to play there".
  emit Error(tr("%1. Allow it there, then play again.").arg(message));
}

void AirPlayEngine::SessionFailed(const QString& error) {
  pending_play_nanosec_ = -1;
  target_->SetStream(nullptr);
  GstEngine::Stop();
  emit Error(error);
  emit DeviceFailed();
}
