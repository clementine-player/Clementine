#include "edittagdialog.h"
#include "ui_edittagdialog.h"

#include <QtDebug>

EditTagDialog::EditTagDialog(QWidget* parent)
  : QDialog(parent)
{
  ui_.setupUi(this);

  static const char* kHintText = "[click to edit]";
  ui_.album->SetHint(kHintText);
  ui_.artist->SetHint(kHintText);
  ui_.genre->SetHint(kHintText);
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

    ui_.filename->setText("Editing " + QString::number(songs.count()) + " tracks");
  }

  return true;
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
