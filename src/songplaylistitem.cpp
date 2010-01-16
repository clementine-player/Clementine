#include "songplaylistitem.h"

#include <QtDebug>
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
  settings.setValue("library_directory", song_.directory_id());
}

void SongPlaylistItem::Restore(const QSettings& settings) {
  QString filename(settings.value("filename").toString());
  int directory_id(settings.value("library_directory", -1).toInt());

  song_.InitFromFile(filename, directory_id);
}

QUrl SongPlaylistItem::Url() {
  QUrl ret(QUrl::fromLocalFile(song_.filename()));
  ret.setHost("localhost");
  return ret;
}

void SongPlaylistItem::Reload() {
  song_.InitFromFile(song_.filename(), song_.directory_id());
}
