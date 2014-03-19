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

#include "lastfmstationdialog.h"
#include "ui_lastfmstationdialog.h"

LastFMStationDialog::LastFMStationDialog(QWidget* parent)
    : QDialog(parent), ui_(new Ui_LastFMStationDialog) {
  ui_->setupUi(this);

  resize(sizeHint());
}

LastFMStationDialog::~LastFMStationDialog() { delete ui_; }

void LastFMStationDialog::SetType(Type type) {
  ui_->type->setCurrentIndex(type);
  ui_->content->clear();
  ui_->content->setFocus(Qt::OtherFocusReason);
}

QString LastFMStationDialog::content() const { return ui_->content->text(); }
