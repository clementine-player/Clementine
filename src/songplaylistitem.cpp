#include "songplaylistitem.h"

#include <QtDebug>
#include <QFile>
#include <QSettings>

SongPlaylistItem::SongPlaylistItem()
{
}

SongPlaylistItem::SongPlaylistItem(const Song& song)
  : song_(song)
{
}

void SongPlaylistItem::Save(QSettings& settings) const {
  settings.setValue("filename", song_.filename());
  settings.setValue("art_automatic", song_.art_automatic());
  settings.setValue("art_manual", song_.art_manual());

  if (song_.filetype() == Song::Type_Stream) {
    SaveStream(settings);
  } else {
    SaveFile(settings);
  }
}

void SongPlaylistItem::SaveFile(QSettings& settings) const {
  settings.setValue("stream", false);
  settings.setValue("library_directory", song_.directory_id());
}

void SongPlaylistItem::SaveStream(QSettings& settings) const {
  settings.setValue("stream", true);
  settings.setValue("title", song_.title());
  settings.setValue("artist", song_.artist());
  settings.setValue("album", song_.album());
  settings.setValue("length", song_.length());
}

void SongPlaylistItem::Restore(const QSettings& settings) {
  song_.set_art_automatic(settings.value("art_automatic").toString());
  song_.set_art_manual(settings.value("art_manual").toString());

  const bool stream = settings.value("stream", false).toBool();
  if (stream) {
    RestoreStream(settings);
  } else {
    RestoreFile(settings);
  }
}

void SongPlaylistItem::RestoreFile(const QSettings& settings) {
  QString filename(settings.value("filename").toString());

  int directory_id(settings.value("library_directory", -1).toInt());
  song_.InitFromFile(filename, directory_id);
}

void SongPlaylistItem::RestoreStream(const QSettings& settings) {
  QString filename(settings.value("filename").toString());
  song_.set_filename(filename);
  song_.set_filetype(Song::Type_Stream);

  song_.Init(settings.value("title", "Unknown").toString(),
             settings.value("artist", "Unknown").toString(),
             settings.value("album", "Unknown").toString(),
             settings.value("length", -1).toInt());
}

QUrl SongPlaylistItem::Url() {
  if (QFile::exists(song_.filename())) {
    return QUrl::fromLocalFile(song_.filename());
  } else {
    return song_.filename();
  }
}

void SongPlaylistItem::Reload() {
  song_.InitFromFile(song_.filename(), song_.directory_id());
}
