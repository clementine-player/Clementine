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

#include "playlistsequence.h"
#include "ui_playlistsequence.h"
#include "ui/iconloader.h"

#include <QMenu>
#include <QActionGroup>
#include <QSettings>
#include <QtDebug>
#include <QPainter>

const char* PlaylistSequence::kSettingsGroup = "PlaylistSequence";

PlaylistSequence::PlaylistSequence(QWidget* parent, SettingsProvider* settings)
    : QWidget(parent),
      ui_(new Ui_PlaylistSequence),
      settings_(settings ? settings : new DefaultSettingsProvider),
      repeat_menu_(new QMenu(this)),
      shuffle_menu_(new QMenu(this)),
      loading_(false),
      repeat_mode_(Repeat_Off),
      shuffle_mode_(Shuffle_Off),
      dynamic_(false) {
  ui_->setupUi(this);

  // Icons
  ui_->repeat->setIcon(
      AddDesaturatedIcon(IconLoader::Load("media-playlist-repeat", IconLoader::Base)));
  ui_->shuffle->setIcon(
      AddDesaturatedIcon(IconLoader::Load("media-playlist-shuffle", IconLoader::Base)));

  // Remove arrow indicators
  ui_->repeat->setStyleSheet("QToolButton::menu-indicator { image: none; }");
  ui_->shuffle->setStyleSheet("QToolButton::menu-indicator { image: none; }");

  settings_->set_group(kSettingsGroup);

  QActionGroup* repeat_group = new QActionGroup(this);
  repeat_group->addAction(ui_->action_repeat_off);
  repeat_group->addAction(ui_->action_repeat_track);
  repeat_group->addAction(ui_->action_repeat_album);
  repeat_group->addAction(ui_->action_repeat_playlist);
  repeat_group->addAction(ui_->action_repeat_onebyone);
  repeat_group->addAction(ui_->action_repeat_intro);
  repeat_menu_->addActions(repeat_group->actions());
  ui_->repeat->setMenu(repeat_menu_);

  QActionGroup* shuffle_group = new QActionGroup(this);
  shuffle_group->addAction(ui_->action_shuffle_off);
  shuffle_group->addAction(ui_->action_shuffle_all);
  shuffle_group->addAction(ui_->action_shuffle_inside_album);
  shuffle_group->addAction(ui_->action_shuffle_albums);
  shuffle_menu_->addActions(shuffle_group->actions());
  ui_->shuffle->setMenu(shuffle_menu_);

  connect(repeat_group, SIGNAL(triggered(QAction*)),
          SLOT(RepeatActionTriggered(QAction*)));
  connect(shuffle_group, SIGNAL(triggered(QAction*)),
          SLOT(ShuffleActionTriggered(QAction*)));

  Load();
}

PlaylistSequence::~PlaylistSequence() { delete ui_; }

void PlaylistSequence::Load() {
  loading_ = true;  // Stops these setter functions calling Save()
  SetShuffleMode(
      ShuffleMode(settings_->value("shuffle_mode", Shuffle_Off).toInt()));
  SetRepeatMode(
      RepeatMode(settings_->value("repeat_mode", Repeat_Off).toInt()));
  loading_ = false;
}

void PlaylistSequence::Save() {
  if (loading_) return;

  settings_->setValue("shuffle_mode", shuffle_mode_);
  settings_->setValue("repeat_mode", repeat_mode_);
}

QIcon PlaylistSequence::AddDesaturatedIcon(const QIcon& icon) {
  QIcon ret;
  for (const QSize& size : icon.availableSizes()) {
    QPixmap on(icon.pixmap(size));
    QPixmap off(DesaturatedPixmap(on));

    ret.addPixmap(off, QIcon::Normal, QIcon::Off);
    ret.addPixmap(on, QIcon::Normal, QIcon::On);
  }
  return ret;
}

QPixmap PlaylistSequence::DesaturatedPixmap(const QPixmap& pixmap) {
  QPixmap ret(pixmap.size());
  ret.fill(Qt::transparent);

  QPainter p(&ret);
  p.setOpacity(0.5);
  p.drawPixmap(0, 0, pixmap);
  p.end();

  return ret;
}

void PlaylistSequence::RepeatActionTriggered(QAction* action) {
  RepeatMode mode = Repeat_Off;
  if (action == ui_->action_repeat_track) mode = Repeat_Track;
  if (action == ui_->action_repeat_album) mode = Repeat_Album;
  if (action == ui_->action_repeat_playlist) mode = Repeat_Playlist;
  if (action == ui_->action_repeat_onebyone) mode = Repeat_OneByOne;
  if (action == ui_->action_repeat_intro) mode = Repeat_Intro;

  SetRepeatMode(mode);
}

void PlaylistSequence::ShuffleActionTriggered(QAction* action) {
  ShuffleMode mode = Shuffle_Off;
  if (action == ui_->action_shuffle_all) mode = Shuffle_All;
  if (action == ui_->action_shuffle_inside_album) mode = Shuffle_InsideAlbum;
  if (action == ui_->action_shuffle_albums) mode = Shuffle_Albums;

  SetShuffleMode(mode);
}

void PlaylistSequence::SetRepeatMode(RepeatMode mode) {
  ui_->repeat->setChecked(mode != Repeat_Off);

  switch (mode) {
    case Repeat_Off:
      ui_->action_repeat_off->setChecked(true);
      break;
    case Repeat_Track:
      ui_->action_repeat_track->setChecked(true);
      break;
    case Repeat_Album:
      ui_->action_repeat_album->setChecked(true);
      break;
    case Repeat_Playlist:
      ui_->action_repeat_playlist->setChecked(true);
      break;
    case Repeat_OneByOne:
      ui_->action_repeat_onebyone->setChecked(true);
      break;
    case Repeat_Intro:
      ui_->action_repeat_intro->setChecked(true);
      break;
  }

  if (mode != repeat_mode_) {
    repeat_mode_ = mode;
    emit RepeatModeChanged(mode);
  }

  Save();
}

void PlaylistSequence::SetShuffleMode(ShuffleMode mode) {
  ui_->shuffle->setChecked(mode != Shuffle_Off);

  switch (mode) {
    case Shuffle_Off:
      ui_->action_shuffle_off->setChecked(true);
      break;
    case Shuffle_All:
      ui_->action_shuffle_all->setChecked(true);
      break;
    case Shuffle_InsideAlbum:
      ui_->action_shuffle_inside_album->setChecked(true);
      break;
    case Shuffle_Albums:
      ui_->action_shuffle_albums->setChecked(true);
      break;
  }

  if (mode != shuffle_mode_) {
    shuffle_mode_ = mode;
    emit ShuffleModeChanged(mode);
  }

  Save();
}

void PlaylistSequence::SetUsingDynamicPlaylist(bool dynamic) {
  dynamic_ = dynamic;
  const QString not_available(
      tr("Not available while using a dynamic playlist"));

  setEnabled(!dynamic);
  ui_->shuffle->setToolTip(dynamic ? not_available : tr("Shuffle"));
  ui_->repeat->setToolTip(dynamic ? not_available : tr("Repeat"));
}

PlaylistSequence::ShuffleMode PlaylistSequence::shuffle_mode() const {
  return dynamic_ ? Shuffle_Off : shuffle_mode_;
}

PlaylistSequence::RepeatMode PlaylistSequence::repeat_mode() const {
  return dynamic_ ? Repeat_Off : repeat_mode_;
}

// called from global shortcut
void PlaylistSequence::CycleShuffleMode() {
  ShuffleMode mode = Shuffle_Off;
  // we cycle through the shuffle modes
  switch (shuffle_mode()) {
    case Shuffle_Off:
      mode = Shuffle_All;
      break;
    case Shuffle_All:
      mode = Shuffle_InsideAlbum;
      break;
    case Shuffle_InsideAlbum:
      mode = Shuffle_Albums;
      break;
    case Shuffle_Albums:
      break;
  }

  SetShuffleMode(mode);
}

// called from global shortcut
void PlaylistSequence::CycleRepeatMode() {
  RepeatMode mode = Repeat_Off;
  // we cycle through the repeat modes
  switch (repeat_mode()) {
    case Repeat_Off:
      mode = Repeat_Track;
      break;
    case Repeat_Track:
      mode = Repeat_Album;
      break;
    case Repeat_Album:
      mode = Repeat_Playlist;
      break;
    case Repeat_Playlist:
      mode = Repeat_OneByOne;
      break;
    case Repeat_OneByOne:
      mode = Repeat_Intro;
      break;
    case Repeat_Intro:
      break;
  }

  SetRepeatMode(mode);
}
