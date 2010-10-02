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

#include "songinfobase.h"

SongInfoBase::SongInfoBase(QWidget* parent)
  : QWidget(parent),
    dirty_(false)
{
}

void SongInfoBase::SongChanged(const Song& metadata) {
  if (isVisible()) {
    Update(metadata);
    dirty_ = false;
  } else {
    queued_metadata_ = metadata;
    dirty_ = true;
  }
}

void SongInfoBase::SongFinished() {
  dirty_ = false;
}

void SongInfoBase::showEvent(QShowEvent* e) {
  if (dirty_) {
    Update(queued_metadata_);
    dirty_ = false;
  }
  QWidget::showEvent(e);
}
