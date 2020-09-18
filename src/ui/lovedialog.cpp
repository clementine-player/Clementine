/* This file is part of Clementine.
   Copyright 2020, Dave McKellar

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

#include "ui/lovedialog.h"

#include <QLabel>
#include <QPushButton>

#include "core/application.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "library/librarybackend.h"
#include "ui_lovedialog.h"

const char* LoveDialog::kSettingsGroup = "LoveDialog";

LoveDialog::LoveDialog(Application* app, QWidget* parent)
    : QDialog(parent), ui_(new Ui_LoveDialog), app_(app) {
  ui_->setupUi(this);

  connect(ui_->button_box, SIGNAL(clicked(QAbstractButton*)),
          SLOT(ButtonClicked(QAbstractButton*)));
  connect(ui_->rating, SIGNAL(RatingChanged(float)), SLOT(SongRated(float)));
}

LoveDialog::~LoveDialog() { delete ui_; }

void LoveDialog::SetSong(const Song& song) {
  song_ = song;
  QString summary =
      "<b>" + song.PrettyTitleWithArtist().toHtmlEscaped() + "</b>";
  ui_->summary->setText(summary);
  ui_->rating->set_rating(song.rating());
}

void LoveDialog::ButtonClicked(QAbstractButton* button) {
  if (button == ui_->button_box->button(QDialogButtonBox::Cancel)) {
    reject();
  }
}

void LoveDialog::showEvent(QShowEvent* e) {
  // Set the dialog's height to the smallest possible
  resize(width(), sizeHint().height());

  // Restore the tab that was current last time.
  QSettings s;
  s.beginGroup(kSettingsGroup);

  QDialog::showEvent(e);
}

void LoveDialog::SongRated(float rating) {
  qLog(Debug) << "LoveDialog::SongRated: rating=" << rating;
  qLog(Debug) << "LoveDialog::SongRated: isValid=" << song_.is_valid()
              << " id=" << song_.id();
  if (!song_.is_valid() || song_.id() == -1) return;

  qLog(Debug) << "LoveDialog::SongRated: song is valid and id is good";
  song_.set_rating(rating);
  app_->library_backend()->UpdateSongRatingAsync(song_.id(), rating);
  QDialog::accept();
}
