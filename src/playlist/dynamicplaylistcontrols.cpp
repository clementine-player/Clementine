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

#include "dynamicplaylistcontrols.h"
#include "ui_dynamicplaylistcontrols.h"

DynamicPlaylistControls::DynamicPlaylistControls(QWidget* parent)
    : QWidget(parent), ui_(new Ui_DynamicPlaylistControls) {
  ui_->setupUi(this);

  connect(ui_->expand, SIGNAL(clicked()), SIGNAL(Expand()));
  connect(ui_->repopulate, SIGNAL(clicked()), SIGNAL(Repopulate()));
  connect(ui_->off, SIGNAL(clicked()), SIGNAL(TurnOff()));
}

DynamicPlaylistControls::~DynamicPlaylistControls() { delete ui_; }
