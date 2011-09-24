/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "globalsearchpopup.h"

#include <QShortcut>

#include "core/logging.h"
#include "core/player.h"
#include "ui/iconloader.h"

GlobalSearchPopup::GlobalSearchPopup(QWidget* parent)
    : QWidget(parent),
      ui_(new Ui_GlobalSearchPopup),
      mac_psn_(NULL) {
  Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;
  setWindowFlags(flags);

  ui_->setupUi(this);

  ui_->previous->setIcon(IconLoader::Load("media-skip-backward"));
  ui_->next->setIcon(IconLoader::Load("media-skip-forward"));
  ui_->play_pause->setIcon(IconLoader::Load("media-playback-start"));
  ui_->stop->setIcon(IconLoader::Load("media-playback-stop"));

  QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
  connect(shortcut, SIGNAL(activated()), SLOT(hide()));
}

void GlobalSearchPopup::Init(GlobalSearch* engine, Player* player) {
  ui_->search_widget->Init(engine);

  // Forward AddToPlaylist signal.
  connect(ui_->search_widget, SIGNAL(AddToPlaylist(QMimeData*)),
      SIGNAL(AddToPlaylist(QMimeData*)));

  connect(ui_->previous, SIGNAL(clicked(bool)), player, SLOT(Previous()));
  connect(ui_->next, SIGNAL(clicked(bool)), player, SLOT(Next()));
  connect(ui_->play_pause, SIGNAL(clicked(bool)), player, SLOT(PlayPause()));
  connect(ui_->stop, SIGNAL(clicked(bool)), player, SLOT(Stop()));
}

void GlobalSearchPopup::setFocus(Qt::FocusReason reason) {
  ui_->search_widget->setFocus(reason);
}

void GlobalSearchPopup::showEvent(QShowEvent* e) {
#ifdef Q_OS_DARWIN
  StorePreviousProcess();
#endif
  QWidget::showEvent(e);
  raise();
}

void GlobalSearchPopup::hide() {
#ifdef Q_OS_DARWIN
  ActivatePreviousProcess();
#endif
  QWidget::hide();
}
