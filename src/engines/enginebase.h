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

//Copyright: (C) 2003 Mark Kretschmann
//           (C) 2004 Max Howell, <max.howell@methylblue.com>
//License:   See COPYING

#ifndef AMAROK_ENGINEBASE_H
#define AMAROK_ENGINEBASE_H

#include <QUrl>
#include <QObject>
#include <QList>

#include <sys/types.h>
#include <vector>
#include <boost/noncopyable.hpp>

#include "engine_fwd.h"

namespace Engine {

typedef std::vector<int16_t> Scope;

class Base : public QObject, boost::noncopyable {
  Q_OBJECT

 public:
  virtual ~Base();

  virtual bool Init() = 0;
  virtual bool CanDecode(const QUrl &url) = 0;

  virtual void StartPreloading(const QUrl&) {}
  virtual bool Play(uint offset = 0) = 0;
  virtual void Stop() = 0;
  virtual void Pause() = 0;
  virtual void Unpause() = 0;
  virtual void Seek( uint ms ) = 0;

  virtual int AddBackgroundStream(const QUrl& url);
  virtual void StopBackgroundStream(int id) {}
  virtual void SetBackgroundStreamVolume(int id, int volume) {}

  virtual State state() const = 0;
  virtual uint position() const = 0;
  virtual uint length() const { return 0; }

  // Helpers
  virtual bool Load(const QUrl &url, TrackChangeType change);
  bool Play(const QUrl &u, TrackChangeType c);
  void SetVolume( uint value );


  // Simple accessors
  inline uint volume() const { return volume_; }
  virtual const Scope &scope() { return scope_; }
  bool is_fadeout_enabled() const { return fadeout_enabled_; }
  bool is_crossfade_enabled() const { return crossfade_enabled_; }
  bool is_autocrossfade_enabled() const { return autocrossfade_enabled_; }

  static const char* kSettingsGroup;
  static const int kScopeSize = 1024;

 public slots:
  virtual void ReloadSettings();

  virtual void SetEqualizerEnabled(bool) {}
  virtual void SetEqualizerParameters(int preamp, const QList<int>& bandGains) {}

 signals:
  // Emitted when crossfading is enabled and the track is crossfade_duration_
  // away from finishing
  void TrackAboutToEnd();

  void TrackEnded();

  void StatusText(const QString&);
  void Error(const QString&);

  void MetaData(const Engine::SimpleMetaBundle&);

  void StateChanged(Engine::State);

 protected:
  Base();

  virtual void SetVolumeSW( uint percent ) = 0;
  static uint MakeVolumeLogarithmic( uint volume );
  void EmitAboutToEnd();

 protected:
  uint  volume_;
  QUrl  url_;
  Scope scope_;

  bool fadeout_enabled_;
  int fadeout_duration_;
  bool crossfade_enabled_;
  bool autocrossfade_enabled_;
  int next_background_stream_id_;

 private:
  bool about_to_end_emitted_;
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

} // namespace

#endif
