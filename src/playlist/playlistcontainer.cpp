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

#include "playlistcontainer.h"
#include "ui_playlistcontainer.h"
#include "ui/iconloader.h"

PlaylistContainer::PlaylistContainer(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_PlaylistContainer)
{
  ui_->setupUi(this);

  // Icons
  ui_->clear->setIcon(IconLoader::Load("edit-clear-locationbar-ltr"));

  // Tab bar
  ui_->tab_bar->setExpanding(false);
  ui_->tab_bar->setMovable(true);
  ui_->tab_bar->setShape(QTabBar::RoundedSouth);
  ui_->tab_bar->addTab("foo");
  ui_->tab_bar->addTab("bar");

  // Connections
  connect(ui_->clear, SIGNAL(clicked()), SLOT(ClearFilter()));
  connect(ui_->tab_bar, SIGNAL(Rename(int,QString)), SIGNAL(Rename(int,QString)));
  connect(ui_->tab_bar, SIGNAL(Remove(int)), SIGNAL(Remove(int)));
}

PlaylistContainer::~PlaylistContainer() {
  delete ui_;
}

PlaylistView* PlaylistContainer::view() const {
  return ui_->playlist;
}

void PlaylistContainer::SetActions(
    QAction *new_playlist, QAction *save_playlist, QAction *load_playlist) {
  ui_->create_new->setDefaultAction(new_playlist);
  ui_->save->setDefaultAction(save_playlist);
  ui_->load->setDefaultAction(load_playlist);

  ui_->tab_bar->SetActions(new_playlist, save_playlist, load_playlist);
}

void PlaylistContainer::ClearFilter() {
  ui_->filter->clear();
  ui_->filter->setFocus();
}
