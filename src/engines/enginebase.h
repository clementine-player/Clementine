/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

// Copyright: (C) 2003 Mark Kretschmann
//           (C) 2004 Max Howell, <max.howell@methylblue.com>
// License:   See COPYING

#ifndef AMAROK_ENGINEBASE_H
#define AMAROK_ENGINEBASE_H

#include <stdint.h>
#include <sys/types.h>

#include <vector>

#include <QList>
#include <QObject>
#include <QUrl>

#include "engine_fwd.h"

namespace Engine {

typedef std::vector<int16_t> Scope;

class Base : public QObject {
  Q_OBJECT

 public:
  virtual ~Base();

  virtual bool Init() = 0;

  virtual void StartPreloading(const QUrl&, bool, qint64, qint64) {}
  virtual bool Play(quint64 offset_nanosec) = 0;
  virtual void Stop(bool stop_after = false) = 0;
  virtual void Pause() = 0;
  virtual void Unpause() = 0;
  virtual void Seek(quint64 offset_nanosec) = 0;

  virtual int AddBackgroundStream(const QUrl& url);
  virtual void StopBackgroundStream(int id) {}
  virtual void SetBackgroundStreamVolume(int id, int volume) {}

  virtual State state() const = 0;
  virtual qint64 position_nanosec() const = 0;
  virtual qint64 length_nanosec() const = 0;

  // Subclasses should respect given markers (beginning and end) which are
  // in miliseconds.
  virtual bool Load(const QUrl& url, TrackChangeFlags change,
                    bool force_stop_at_end, quint64 beginning_nanosec,
                    qint64 end_nanosec);
  // Sets new values for the beginning and end markers of the currently playing
  // song.
  // This doesn't change the state of engine or the stream's current position.
  virtual void RefreshMarkers(quint64 beginning_nanosec, qint64 end_nanosec) {
    beginning_nanosec_ = beginning_nanosec;
    end_nanosec_ = end_nanosec;
  }

  // Plays a media stream represented with the URL 'u' from the given
  // 'beginning'
  // to the given 'end' (usually from 0 to a song's length). Both markers
  // should be passed in nanoseconds. 'end' can be negative, indicating that the
  // real length of 'u' stream is unknown.
  bool Play(const QUrl& u, TrackChangeFlags c, bool force_stop_at_end,
            quint64 beginning_nanosec, qint64 end_nanosec);

  void SetVolume(uint value);

  // Simple accessors
  inline uint volume() const { return volume_; }
  virtual const Scope& scope(int chunk_length) { return scope_; }
  bool is_fadeout_enabled() const { return fadeout_enabled_; }
  bool is_crossfade_enabled() const { return crossfade_enabled_; }
  bool is_autocrossfade_enabled() const { return autocrossfade_enabled_; }
  bool crossfade_same_album() const { return crossfade_same_album_; }

  static const char* kSettingsGroup;
  static const int kScopeSize = 1024;

 public slots:
  virtual void ReloadSettings();

  virtual void SetEqualizerEnabled(bool) {}
  virtual void SetEqualizerParameters(int preamp, const QList<int>& bandGains) {
  }
  virtual void SetStereoBalance(float value) {}

signals:
  // Emitted when crossfading is enabled and the track is crossfade_duration_
  // away from finishing
  void TrackAboutToEnd();

  void TrackEnded();

  void FadeoutFinishedSignal();

  void StatusText(const QString&);
  void Error(const QString&);

  // Emitted when Engine was unable to play a song with the given QUrl.
  void InvalidSongRequested(const QUrl&);
  // Emitted when Engine successfully started playing a song with the
  // given QUrl.
  void ValidSongRequested(const QUrl&);

  void MetaData(const Engine::SimpleMetaBundle&);

  // Signals that the engine's state has changed (a stream was stopped for
  // example).
  // Always use the state from event, because it's not guaranteed that immediate
  // subsequent call to state() won't return a stale value.
  void StateChanged(Engine::State);

 protected:
  Base();

  virtual void SetVolumeSW(uint percent) = 0;
  static uint MakeVolumeLogarithmic(uint volume);
  void EmitAboutToEnd();

 protected:
  uint volume_;
  quint64 beginning_nanosec_;
  qint64 end_nanosec_;
  QUrl url_;
  Scope scope_;

  bool fadeout_enabled_;
  qint64 fadeout_duration_nanosec_;
  bool crossfade_enabled_;
  bool autocrossfade_enabled_;
  bool crossfade_same_album_;
  int next_background_stream_id_;
  bool fadeout_pause_enabled_;
  qint64 fadeout_pause_duration_nanosec_;

 private:
  bool about_to_end_emitted_;
  Q_DISABLE_COPY(Base);
};

struct SimpleMetaBundle {
  QString title;
  QString artist;
  QString album;
  QString comment;
  QString genre;
  QString bitrate;
  QString samplerate;
  QString length;
  QString year;
  QString tracknr;
};

}  // namespace

#endif
