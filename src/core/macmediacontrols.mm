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

#include "core/macmediacontrols.h"

#import <AppKit/NSImage.h>
#import <MediaPlayer/MediaPlayer.h>

#include <QImage>
#include <QList>
#include <QPair>

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/timeconstants.h"
#include "covers/currentartloader.h"
#include "engines/enginebase.h"
#include "playlist/playlistmanager.h"

struct MacMediaControls::Private {
  // The handlers added to the command center, to take them off again.
  QList<QPair<MPRemoteCommand*, id>> targets;
  // The current song's cover, retained; nil when it has none.
  NSImage* artwork = nil;

  void SetArtwork(NSImage* image) {
    [image retain];
    [artwork release];
    artwork = image;
  }
};

MacMediaControls::MacMediaControls(Application* app, QObject* parent)
    : QObject(parent), app_(app), d_(new Private) {
  MPRemoteCommandCenter* center = [MPRemoteCommandCenter sharedCommandCenter];

  // The command center may call these on any thread, so each queues its
  // command for the GUI thread.
  auto add = [this](MPRemoteCommand* command, Command c) {
    command.enabled = YES;
    id target = [command
        addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent*) {
          QMetaObject::invokeMethod(this, "HandleCommand", Qt::QueuedConnection,
                                    Q_ARG(int, c));
          return MPRemoteCommandHandlerStatusSuccess;
        }];
    d_->targets << qMakePair(command, target);
  };
  add(center.playCommand, Play);
  add(center.pauseCommand, Pause);
  add(center.togglePlayPauseCommand, TogglePlayPause);
  add(center.stopCommand, Stop);
  add(center.nextTrackCommand, Next);
  add(center.previousTrackCommand, Previous);

  // Dragging the position in Control Center.
  MPRemoteCommand* seek = center.changePlaybackPositionCommand;
  seek.enabled = YES;
  id seek_target = [seek
      addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent* event) {
        const double seconds =
            ((MPChangePlaybackPositionCommandEvent*)event).positionTime;
        QMetaObject::invokeMethod(this, "SeekTo", Qt::QueuedConnection,
                                  Q_ARG(double, seconds));
        return MPRemoteCommandHandlerStatusSuccess;
      }];
  d_->targets << qMakePair(seek, seek_target);

  connect(app_->player()->engine(), SIGNAL(StateChanged(Engine::State)),
          SLOT(EngineStateChanged(Engine::State)));
  connect(app_->player(), SIGNAL(Seeked(qlonglong)), SLOT(Seeked(qlonglong)));
  connect(app_->playlist_manager(), SIGNAL(CurrentSongChanged(Song)),
          SLOT(CurrentSongChanged(Song)));
  connect(app_->current_art_loader(), SIGNAL(ArtLoaded(Song, QString, QImage)),
          SLOT(ArtLoaded(Song, QString, QImage)));

  EngineStateChanged(app_->player()->GetState());
}

MacMediaControls::~MacMediaControls() {
  for (const auto& target : d_->targets) {
    [target.first removeTarget:target.second];
  }
  MPNowPlayingInfoCenter* info_center = [MPNowPlayingInfoCenter defaultCenter];
  info_center.nowPlayingInfo = nil;
  info_center.playbackState = MPNowPlayingPlaybackStateStopped;
  d_->SetArtwork(nil);
}

void MacMediaControls::HandleCommand(int command) {
  Player* player = app_->player();
  const Engine::State state = player->GetState();

  switch (command) {
    case Play:
      // Player::Play() restarts the song if it's already playing, which isn't
      // what a headphone play button should do.
      if (state != Engine::Playing) player->PlayPause();
      break;
    case Pause:
      // PlayPause() rather than Pause() so streams that can't be paused are
      // stopped instead.
      if (state == Engine::Playing) player->PlayPause();
      break;
    case TogglePlayPause:
      player->PlayPause();
      break;
    case Stop:
      player->Stop();
      break;
    case Next:
      player->Next();
      break;
    case Previous:
      player->Previous();
      break;
    default:
      break;
  }
}

void MacMediaControls::SeekTo(double seconds) {
  app_->player()->SeekTo(static_cast<int>(seconds));
}

void MacMediaControls::EngineStateChanged(Engine::State state) {
  MPNowPlayingInfoCenter* info_center = [MPNowPlayingInfoCenter defaultCenter];
  switch (state) {
    case Engine::Playing:
      info_center.playbackState = MPNowPlayingPlaybackStatePlaying;
      break;
    case Engine::Paused:
      info_center.playbackState = MPNowPlayingPlaybackStatePaused;
      break;
    case Engine::Empty:
    case Engine::Idle:
    case Engine::Error:
    default:
      info_center.playbackState = MPNowPlayingPlaybackStateStopped;
      break;
  }
  // The position and rate, which macOS counts on from.
  UpdateNowPlaying();
}

// Send the song as soon as it changes...
void MacMediaControls::CurrentSongChanged(const Song& song) {
  song_ = song;
  d_->SetArtwork(nil);
  UpdateNowPlaying();
}

// ... and again with the cover once it's been loaded.  CurrentArtLoader only
// emits this for the current song.
void MacMediaControls::ArtLoaded(const Song&, const QString& uri,
                                 const QImage& image) {
  // An empty URI means the song has no cover and image is a placeholder.
  NSImage* artwork = nil;
  if (!uri.isEmpty() && !image.isNull()) {
    CGImageRef cg_image = image.toCGImage();
    if (cg_image) {
      artwork = [[[NSImage alloc] initWithCGImage:cg_image size:NSZeroSize]
          autorelease];
      CGImageRelease(cg_image);
    }
  }
  d_->SetArtwork(artwork);
  UpdateNowPlaying();
}

void MacMediaControls::Seeked(qlonglong) { UpdateNowPlaying(); }

void MacMediaControls::UpdateNowPlaying() {
  MPNowPlayingInfoCenter* info_center = [MPNowPlayingInfoCenter defaultCenter];
  if (!song_.is_valid() || app_->player()->GetState() == Engine::Empty) {
    info_center.nowPlayingInfo = nil;
    return;
  }

  NSMutableDictionary* info = [NSMutableDictionary dictionary];
  info[MPNowPlayingInfoPropertyMediaType] = @(MPNowPlayingInfoMediaTypeAudio);
  info[MPMediaItemPropertyTitle] = song_.PrettyTitle().toNSString();
  if (!song_.artist().isEmpty()) {
    info[MPMediaItemPropertyArtist] = song_.artist().toNSString();
  }
  if (!song_.album().isEmpty()) {
    info[MPMediaItemPropertyAlbumTitle] = song_.album().toNSString();
  }
  if (song_.length_nanosec() > 0) {
    info[MPMediaItemPropertyPlaybackDuration] =
        @(double(song_.length_nanosec()) / kNsecPerSec);
  }

  // macOS moves the position on by itself at the rate given, so it only needs
  // telling where it is when playback starts, stops or jumps.
  const bool playing = app_->player()->GetState() == Engine::Playing;
  info[MPNowPlayingInfoPropertyElapsedPlaybackTime] =
      @(double(app_->player()->engine()->position_nanosec()) / kNsecPerSec);
  info[MPNowPlayingInfoPropertyPlaybackRate] = @(playing ? 1.0 : 0.0);

  if (d_->artwork) {
    NSImage* artwork = d_->artwork;
    info[MPMediaItemPropertyArtwork] = [[[MPMediaItemArtwork alloc]
        initWithBoundsSize:artwork.size
            requestHandler:^NSImage*(CGSize) {
              return artwork;
            }] autorelease];
  }

  info_center.nowPlayingInfo = info;
}
