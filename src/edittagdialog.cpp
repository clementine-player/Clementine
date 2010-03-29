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

#include "playlistdelegates.h"
#include "edittagdialog.h"
#include "library.h"

#include <QtDebug>

EditTagDialog::EditTagDialog(QWidget* parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  QString hint_text(tr("[click to edit]"));
  ui_.album->SetHint(hint_text);
  ui_.artist->SetHint(hint_text);
  ui_.genre->SetHint(hint_text);
}

bool EditTagDialog::SetSongs(const SongList &s) {
  SongList songs;

  foreach (const Song& song, s) {
    if (song.IsEditable())
      songs << song;
  }
  songs_ = songs;

  // Don't allow editing of fields that don't make sense for multiple items
  ui_.title->setEnabled(songs.count() == 1);
  ui_.track->setEnabled(songs.count() == 1);
  ui_.comment->setEnabled(songs.count() == 1);

  if (songs.count() == 0)
    return false;
  else if (songs.count() == 1) {
    const Song& song = songs[0];

    ui_.title->setText(song.title());
    ui_.artist->setText(song.artist());
    ui_.album->setText(song.album());
    ui_.genre->setText(song.genre());
    ui_.year->setValue(song.year());
    ui_.track->setValue(song.track());
    ui_.comment->setPlainText(song.comment());

    ui_.filename->setText(song.filename());
  } else {
    // Find any fields that are common to all items

    ui_.title->clear();
    ui_.track->clear();
    ui_.comment->clear();

    QString artist(songs[0].artist());
    QString album(songs[0].album());
    QString genre(songs[0].genre());
    int year = songs[0].year();

    foreach (const Song& song, songs) {
      if (artist != song.artist())
        artist = QString::null;
      if (album != song.album())
        album = QString::null;
      if (genre != song.genre())
        genre = QString::null;
      if (year != song.year())
        year = -1;
    }

    ui_.artist->setText(artist);
    ui_.album->setText(album);
    ui_.genre->setText(genre);
    ui_.year->setValue(year);

    ui_.filename->setText(tr("Editing %n tracks", "", songs.count()));
  }

  return true;
}

void EditTagDialog::SetTagCompleter(Library* library) {
  new TagCompleter(library, Playlist::Column_Artist, ui_.artist);
  new TagCompleter(library, Playlist::Column_Album, ui_.album);
}

void EditTagDialog::accept() {
  foreach (const Song& old, songs_) {
    Song song(old);

    if (ui_.title->isEnabled() && !ui_.title->text().isEmpty())
      song.set_title(ui_.title->text());
    if (ui_.artist->isEnabled() && !ui_.artist->text().isEmpty())
      song.set_artist(ui_.artist->text());
    if (ui_.album->isEnabled() && !ui_.album->text().isEmpty())
      song.set_album(ui_.album->text());
    if (ui_.genre->isEnabled() && !ui_.genre->text().isEmpty())
      song.set_genre(ui_.genre->text());

    if (ui_.year->isEnabled())
      song.set_year(ui_.year->value());
    if (ui_.track->isEnabled())
      song.set_track(ui_.track->value());

    if (ui_.comment->isEnabled())
      song.set_comment(ui_.comment->toPlainText());

    song.Save();

    emit SongEdited(old, song);
  }

  QDialog::accept();
}
