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

#ifndef PLAYLISTSEQUENCE_H
#define PLAYLISTSEQUENCE_H

#include <memory>

#include <QWidget>

#include "core/settingsprovider.h"

class QMenu;

class Ui_PlaylistSequence;

class PlaylistSequence : public QWidget {
  Q_OBJECT

 public:
  PlaylistSequence(QWidget *parent = 0, SettingsProvider* settings = 0);
  ~PlaylistSequence();

  enum RepeatMode {
    Repeat_Off = 0,
    Repeat_Track = 1,
    Repeat_Album = 2,
    Repeat_Playlist = 3,
  };
  enum ShuffleMode {
    Shuffle_Off = 0,
    Shuffle_All = 1,
    Shuffle_InsideAlbum = 2,
    Shuffle_Albums = 3,
  };

  static const char* kSettingsGroup;

  RepeatMode repeat_mode() const;
  ShuffleMode shuffle_mode() const;

  QMenu* repeat_menu() const { return repeat_menu_; }
  QMenu* shuffle_menu() const { return shuffle_menu_; }

 public slots:
  void SetRepeatMode(PlaylistSequence::RepeatMode mode);
  void SetShuffleMode(PlaylistSequence::ShuffleMode mode);
  void CycleShuffleMode();
  void CycleRepeatMode();
  void SetUsingDynamicPlaylist(bool dynamic);

 signals:
  void RepeatModeChanged(PlaylistSequence::RepeatMode mode);
  void ShuffleModeChanged(PlaylistSequence::ShuffleMode mode);

 private slots:
  void RepeatActionTriggered(QAction*);
  void ShuffleActionTriggered(QAction*);

 private:
  void Load();
  void Save();
  static QIcon AddDesaturatedIcon(const QIcon& icon);
  static QPixmap DesaturatedPixmap(const QPixmap& pixmap);

 private:
  Ui_PlaylistSequence* ui_;
  std::unique_ptr<SettingsProvider> settings_;

  QMenu* repeat_menu_;
  QMenu* shuffle_menu_;

  bool loading_;
  RepeatMode repeat_mode_;
  ShuffleMode shuffle_mode_;
  bool dynamic_;
};

#endif // PLAYLISTSEQUENCE_H
