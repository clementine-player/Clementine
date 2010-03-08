#include "shufflerepeatwidget.h"

#include <QMenu>
#include <QActionGroup>
#include <QSettings>
#include <QtDebug>

const char* ShuffleRepeatWidget::kSettingsGroup = "ShuffleRepeat";

ShuffleRepeatWidget::ShuffleRepeatWidget(QWidget *parent)
  : QWidget(parent),
    repeat_menu_(new QMenu(this)),
    shuffle_menu_(new QMenu(this)),
    loading_(false),
    repeat_mode_(Repeat_Off),
    shuffle_mode_(Shuffle_Off)
{
  ui_.setupUi(this);

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

void ShuffleRepeatWidget::Load() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  loading_ = true; // Stops these setter functions calling Save()
  SetShuffleMode(ShuffleMode(s.value("shuffle_mode", Shuffle_Off).toInt()));
  SetRepeatMode(RepeatMode(s.value("repeat_mode", Repeat_Off).toInt()));
  loading_ = false;
}

void ShuffleRepeatWidget::Save() {
  if (loading_) return;

  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.setValue("shuffle_mode", shuffle_mode_);
  s.setValue("repeat_mode", repeat_mode_);
}

void ShuffleRepeatWidget::RepeatActionTriggered(QAction* action) {
  RepeatMode mode = Repeat_Off;
  if (action == ui_.action_repeat_track)    mode = Repeat_Track;
  if (action == ui_.action_repeat_album)    mode = Repeat_Album;
  if (action == ui_.action_repeat_playlist) mode = Repeat_Playlist;

  SetRepeatMode(mode);
}

void ShuffleRepeatWidget::ShuffleActionTriggered(QAction* action) {
  ShuffleMode mode = Shuffle_Off;
  if (action == ui_.action_shuffle_all)   mode = Shuffle_All;
  if (action == ui_.action_shuffle_album) mode = Shuffle_Album;

  SetShuffleMode(mode);
}

void ShuffleRepeatWidget::SetRepeatMode(RepeatMode mode) {
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

void ShuffleRepeatWidget::SetShuffleMode(ShuffleMode mode) {
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
