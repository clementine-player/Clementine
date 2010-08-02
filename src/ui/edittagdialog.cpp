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

#include "edittagdialog.h"
#include "ui_edittagdialog.h"
#include "library/library.h"
#include "playlist/playlistdelegates.h"

#include <boost/bind.hpp>
using boost::bind;

#include <QtConcurrentMap>
#include <QtDebug>
#include <QDir>

const char* EditTagDialog::kHintText = QT_TR_NOOP("[click to edit]");

EditTagDialog::EditTagDialog(QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_EditTagDialog)
{
  ui_->setupUi(this);
  ui_->busy->hide();
  connect(&watcher_, SIGNAL(finished()), SLOT(SongsEdited()));
}

EditTagDialog::~EditTagDialog() {
  delete ui_;
}

bool EditTagDialog::SetSongs(const SongList &s) {
  SongList songs;

  foreach (const Song& song, s) {
    if (song.IsEditable())
      songs << song;
  }
  songs_ = songs;

  if (songs.count() == 0)
    return false;

  // Don't allow editing of fields that don't make sense for multiple items
  ui_->title->setEnabled(songs.count() == 1);
  ui_->track->setEnabled(songs.count() == 1);
  ui_->comment->setEnabled(songs.count() == 1);

  common_artist_ = songs[0].artist();
  common_album_ = songs[0].album();
  common_genre_ = songs[0].genre();
  common_year_ = songs[0].year();

  if (songs.count() == 1) {
    const Song& song = songs[0];

    ui_->title->setText(song.title());
    ui_->artist->setText(song.artist());
    ui_->album->setText(song.album());
    ui_->genre->setText(song.genre());
    ui_->year->setValue(song.year());
    ui_->track->setValue(song.track());
    ui_->comment->setPlainText(song.comment());

    ui_->filename->setText(QDir::toNativeSeparators(song.filename()));

    ui_->artist->ClearHint();
    ui_->album->ClearHint();
    ui_->genre->ClearHint();
  } else {
    // Find any fields that are common to all items

    ui_->title->clear();
    ui_->track->clear();
    ui_->comment->clear();

    foreach (const Song& song, songs) {
      if (common_artist_ != song.artist()) {
        common_artist_ = QString::null;
        ui_->artist->SetHint(kHintText);
      }

      if (common_album_ != song.album()) {
        common_album_ = QString::null;
        ui_->album->SetHint(kHintText);
      }

      if (common_genre_ != song.genre()) {
        common_genre_ = QString::null;
        ui_->genre->SetHint(kHintText);
      }

      if (common_year_ != song.year())
        common_year_ = -1;
    }

    ui_->artist->setText(common_artist_);
    ui_->album->setText(common_album_);
    ui_->genre->setText(common_genre_);
    ui_->year->setValue(common_year_);

    ui_->filename->setText(tr("Editing %n tracks", "", songs.count()));
  }

  return true;
}

void EditTagDialog::SetTagCompleter(LibraryBackend* backend) {
  new TagCompleter(backend, Playlist::Column_Artist, ui_->artist);
  new TagCompleter(backend, Playlist::Column_Album, ui_->album);
}

void EditTagDialog::SaveSong(const Song& old) {
  Song song(old);

  int track = ui_->track->text().isEmpty() ? -1 : ui_->track->value();
  int year = ui_->year->text().isEmpty() ? -1 : ui_->year->value();

  if (ui_->title->isEnabled())
    song.set_title(ui_->title->text());

  if (ui_->artist->isEnabled() && !(common_artist_.isNull() && ui_->artist->text().isEmpty()))
    song.set_artist(ui_->artist->text());
  if (ui_->album->isEnabled() && !(common_album_.isNull() && ui_->album->text().isEmpty()))
    song.set_album(ui_->album->text());
  if (ui_->genre->isEnabled() && !(common_genre_.isNull() && ui_->genre->text().isEmpty()))
    song.set_genre(ui_->genre->text());
  if (ui_->year->isEnabled() && !(common_year_ == -1 && year == -1))
    song.set_year(year);

  if (ui_->track->isEnabled())
    song.set_track(track);

  if (ui_->comment->isEnabled())
    song.set_comment(ui_->comment->toPlainText());

  {
    QMutexLocker l(&taglib_mutex_);
    song.Save();
  }

  // Corresponding slots should automatically be called in the receiver's thread, assuming
  // the connection is an auto connection.
  emit SongEdited(old, song);
}

void EditTagDialog::accept() {
  QFuture<void> future = QtConcurrent::map(songs_, bind(&EditTagDialog::SaveSong, this, _1));
  watcher_.setFuture(future);
  ui_->busy->show();
  ui_->buttonBox->setEnabled(false);
}

void EditTagDialog::SongsEdited() {
  qDebug() << Q_FUNC_INFO;
  ui_->busy->hide();
  ui_->buttonBox->setEnabled(true);

  QDialog::accept();
}
