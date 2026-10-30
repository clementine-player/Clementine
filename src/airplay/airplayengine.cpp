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

#include <QBuffer>
#include <QInputDialog>
#include <QLineEdit>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QSettings>
#include <QTimer>

#include "airplaysession.h"
#include "airplaysink.h"
#include "airplaystream.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/timeconstants.h"
#include "covers/currentartloader.h"

namespace {

// As big as a Mac shows it, as the UPnP engine sends them.
const int kMaxCoverSize = 600;
// How long a new position takes to settle after a seek or a new track.
const int kProgressDelayMsec = 1000;

QByteArray EncodeCover(const QImage& image) {
  if (image.isNull()) return QByteArray();
  QImage scaled = image;
  if (image.width() > kMaxCoverSize || image.height() > kMaxCoverSize) {
    scaled = image.scaled(kMaxCoverSize, kMaxCoverSize, Qt::KeepAspectRatio,
                          Qt::SmoothTransformation);
  }
  QByteArray data;
  QBuffer buffer(&data);
  buffer.open(QIODevice::WriteOnly);
  if (!scaled.convertToFormat(QImage::Format_RGB32).save(&buffer, "JPG", 90)) {
    return QByteArray();
  }
  return data;
}

}  // namespace

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
      generation_(0),
      repaired_(false),
      progress_timer_(new QTimer(this)) {
  setParent(parent);
  progress_timer_->setSingleShot(true);
  progress_timer_->setInterval(kProgressDelayMsec);
  connect(progress_timer_, SIGNAL(timeout()), SLOT(SendProgress()));
  if (app) {
    connect(app->current_art_loader(), &CurrentArtLoader::ArtLoaded, this,
            &AirPlayEngine::ArtLoaded);
  }
  session_ = new AirPlaySession(AirPlaySession::Options(), this);
  connect(session_, SIGNAL(PasswordNeeded(bool)), SLOT(PasswordNeeded(bool)));
  connect(session_, SIGNAL(PinShown()), SLOT(PinShown()));
  connect(session_, SIGNAL(PairedPersistently(QByteArray, QByteArray)),
          SLOT(PairedPersistently(QByteArray, QByteArray)));
  connect(session_, SIGNAL(PairingForgotten()), SLOT(PairingForgotten()));
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
  if (!session_->is_open()) OpenSession();
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
    if (!session_->is_open()) OpenSession();
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
  progress_timer_->start();
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
  progress_timer_->start();
}

void AirPlayEngine::StreamFlushed() {
  session_->Flush();
  // After a seek.
  progress_timer_->start();
}

// The player's current song changed, or its cover was found.
void AirPlayEngine::ArtLoaded(const Song& song, const QString&,
                              const QImage& image) {
  song_ = song;
  cover_ = image;
  if (!session_->is_open()) return;
  SendNowPlaying();
  progress_timer_->start();
}

void AirPlayEngine::SendNowPlaying() {
  session_->SetNowPlaying(song_.PrettyTitle(), song_.artist(), song_.album());
  session_->SetArtwork(EncodeCover(cover_));
}

void AirPlayEngine::SendProgress() {
  session_->SetProgress(position_nanosec() / kNsecPerMsec,
                        song_.length_nanosec() / kNsecPerMsec);
}

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

QString AirPlayEngine::SettingsGroup() const {
  // Device ids look like MAC addresses, whose colons settings keys avoid.
  return QString("AirPlay/receivers/%1").arg(QString(device_.id).remove(':'));
}

void AirPlayEngine::OpenSession() {
  QSettings s;
  AirPlaySession::Options options;
  options.sender_id = AirPlaySession::SavedSenderId();
  options.identity_seed = AirPlaySession::SavedIdentitySeed();
  s.beginGroup(SettingsGroup());
  options.password = s.value("password").toString();
  options.receiver_pairing_id = s.value("pairing_id").toByteArray();
  options.receiver_public_key =
      QByteArray::fromHex(s.value("public_key").toByteArray());
  s.endGroup();
  // Receivers that need a PIN pair with long-term keys, so they're asked
  // for it only once. Others pair transiently, unless the hidden setting
  // AirPlay/persistent_pairing says otherwise; AirPlay/show_pin, also
  // hidden, tries the PIN flow with any receiver.
  const bool needs_pin = device_.status_flags & AirPlayDevice::PinRequired ||
                         s.value("AirPlay/show_pin").toBool();
  if (!needs_pin && !s.value("AirPlay/persistent_pairing").toBool()) {
    options.identity_seed.clear();
  }
  options.show_pin = needs_pin && options.receiver_public_key.isEmpty();
  // Hidden too: plays to AirPlay 2 receivers with AirPlay 1.
  options.force_airplay1 = s.value("AirPlay/force_airplay1").toBool();
  session_->set_options(options);
  if (song_.is_valid()) SendNowPlaying();
  session_->Open(device_);
}

// Asks for the receiver's password, then tries again with it.
void AirPlayEngine::PasswordNeeded(bool wrong) {
  // What was to play stays loaded meanwhile.
  const qint64 offset = pending_play_nanosec_;
  QInputDialog* dialog = new QInputDialog;
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowTitle(tr("AirPlay password"));
  dialog->setLabelText(
      wrong ? tr("That password was wrong. Enter %1's AirPlay password:")
                  .arg(device_.name)
            : tr("Enter %1's AirPlay password:").arg(device_.name));
  dialog->setTextEchoMode(QLineEdit::Password);
  connect(dialog, &QInputDialog::textValueSelected, this,
          [this, offset](const QString& password) {
            QSettings s;
            s.beginGroup(SettingsGroup());
            s.setValue("password", password);
            pending_play_nanosec_ = qMax<qint64>(0, offset);
            OpenSession();
          });
  connect(dialog, &QInputDialog::rejected, this, [this]() { Stop(); });
  dialog->open();
}

// Asks for the PIN the receiver shows.
void AirPlayEngine::PinShown() {
  QInputDialog* dialog = new QInputDialog;
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowTitle(tr("AirPlay PIN"));
  dialog->setLabelText(tr("Enter the PIN %1 shows:").arg(device_.name));
  connect(dialog, &QInputDialog::textValueSelected, session_,
          &AirPlaySession::SubmitPin);
  connect(dialog, &QInputDialog::rejected, this, [this]() { Stop(); });
  dialog->open();
}

void AirPlayEngine::PairedPersistently(const QByteArray& receiver_pairing_id,
                                       const QByteArray& receiver_public_key) {
  QSettings s;
  s.beginGroup(SettingsGroup());
  s.setValue("pairing_id", receiver_pairing_id);
  s.setValue("public_key", receiver_public_key.toHex());
  repaired_ = false;
}

// The receiver forgot this sender, so it pairs again, once.
void AirPlayEngine::PairingForgotten() {
  QSettings s;
  s.beginGroup(SettingsGroup());
  s.remove("pairing_id");
  s.remove("public_key");
  if (repaired_) {
    SessionFailed(tr("%1 doesn't remember Clementine, and pairing again "
                     "didn't work")
                      .arg(device_.name));
    return;
  }
  repaired_ = true;
  OpenSession();
}
