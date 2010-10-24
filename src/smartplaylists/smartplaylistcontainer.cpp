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

#include "smartplaylistcontainer.h"
#include "smartplaylistmodel.h"
#include "ui_smartplaylistcontainer.h"
#include "playlist/playlistmanager.h"
#include "ui/iconloader.h"

SmartPlaylistContainer::SmartPlaylistContainer(QWidget *parent)
  : QWidget(parent),
    ui_(new Ui_SmartPlaylistContainer),
    first_show_(true),
    model_(new SmartPlaylistModel(this)),
    playlist_manager_(NULL)
{
  ui_->setupUi(this);

  connect(ui_->view, SIGNAL(Play(QModelIndex,bool,bool)), SLOT(Play(QModelIndex,bool,bool)));

  ui_->view->setModel(model_);
}

SmartPlaylistContainer::~SmartPlaylistContainer() {
  delete ui_;
}

void SmartPlaylistContainer::set_library(LibraryBackend* library) {
  model_->set_library(library);
}

void SmartPlaylistContainer::showEvent(QShowEvent*) {
  if (!first_show_)
    return;
  first_show_ = false;

  ui_->add->setIcon(IconLoader::Load("list-add"));
  ui_->remove->setIcon(IconLoader::Load("list-remove"));
  ui_->play->setIcon(IconLoader::Load("media-playback-start"));
}

void SmartPlaylistContainer::Play(const QModelIndex& index, bool as_new, bool clear) {
  PlaylistGeneratorPtr generator = model_->CreateGenerator(index);
  if (!generator)
    return;

  playlist_manager_->PlaySmartPlaylist(generator, as_new, clear);
}
