/* This file is part of Clementine.

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

#include "playlistsequence.h"

#include <QMenu>
#include <QActionGroup>
#include <QSettings>
#include <QtDebug>

const char* PlaylistSequence::kSettingsGroup = "PlaylistSequence";

PlaylistSequence::PlaylistSequence(QWidget *parent, SettingsProvider *settings)
  : QWidget(parent),
    settings_(settings ? settings : new DefaultSettingsProvider),
    repeat_menu_(new QMenu(this)),
    shuffle_menu_(new QMenu(this)),
    loading_(false),
    repeat_mode_(Repeat_Off),
    shuffle_mode_(Shuffle_Off)
{
  ui_.setupUi(this);

  settings_->set_group(kSettingsGroup);

  QActionGroup* repeat_group = new QActionGroup(this);
  repeat_group->addAction(ui_.action_repeat_off);
  repeat_group->addAction(ui_.action_repeat_track);
  repeat_group->addAction(ui_.action_repeat_album);
  repeat_group->addAction(ui_.action_repeat_playlist);
  repeat_menu_->addActions(repeat_group->actions());
  ui_.repeat->setMenu(repeat_menu_);

  QActionGroup* shuffle_group = new QActionGroup(this);
  shuffle_group->addAction(ui_.action_shuffle_off);
  shuffle_group->addAction(ui_.action_shuffle_all);
  shuffle_group->addAction(ui_.action_shuffle_album);
  shuffle_menu_->addActions(shuffle_group->actions());
  ui_.shuffle->setMenu(shuffle_menu_);

  connect(repeat_group, SIGNAL(triggered(QAction*)), SLOT(RepeatActionTriggered(QAction*)));
  connect(shuffle_group, SIGNAL(triggered(QAction*)), SLOT(ShuffleActionTriggered(QAction*)));

  Load();
}

void PlaylistSequence::Load() {
  loading_ = true; // Stops these setter functions calling Save()
  SetShuffleMode(ShuffleMode(settings_->value("shuffle_mode", Shuffle_Off).toInt()));
  SetRepeatMode(RepeatMode(settings_->value("repeat_mode", Repeat_Off).toInt()));
  loading_ = false;
}

void PlaylistSequence::Save() {
  if (loading_) return;

  settings_->setValue("shuffle_mode", shuffle_mode_);
  settings_->setValue("repeat_mode", repeat_mode_);
}

void PlaylistSequence::RepeatActionTriggered(QAction* action) {
  RepeatMode mode = Repeat_Off;
  if (action == ui_.action_repeat_track)    mode = Repeat_Track;
  if (action == ui_.action_repeat_album)    mode = Repeat_Album;
  if (action == ui_.action_repeat_playlist) mode = Repeat_Playlist;

  SetRepeatMode(mode);
}

void PlaylistSequence::ShuffleActionTriggered(QAction* action) {
  ShuffleMode mode = Shuffle_Off;
  if (action == ui_.action_shuffle_all)   mode = Shuffle_All;
  if (action == ui_.action_shuffle_album) mode = Shuffle_Album;

  SetShuffleMode(mode);
}

void PlaylistSequence::SetRepeatMode(RepeatMode mode) {
  ui_.repeat->setChecked(mode != Repeat_Off);

  switch(mode) {
    case Repeat_Off:      ui_.action_repeat_off->setChecked(true);      break;
    case Repeat_Track:    ui_.action_repeat_track->setChecked(true);    break;
    case Repeat_Album:    ui_.action_repeat_album->setChecked(true);    break;
    case Repeat_Playlist: ui_.action_repeat_playlist->setChecked(true); break;
  }

  if (mode != repeat_mode_)
    emit RepeatModeChanged(mode);
  repeat_mode_ = mode;
  Save();
}

void PlaylistSequence::SetShuffleMode(ShuffleMode mode) {
  ui_.shuffle->setChecked(mode != Shuffle_Off);

  switch (mode) {
    case Shuffle_Off:   ui_.action_shuffle_off->setChecked(true);   break;
    case Shuffle_All:   ui_.action_shuffle_all->setChecked(true);   break;
    case Shuffle_Album: ui_.action_shuffle_album->setChecked(true); break;
  }

  if (mode != shuffle_mode_)
    emit ShuffleModeChanged(mode);
  shuffle_mode_ = mode;
  Save();
}
