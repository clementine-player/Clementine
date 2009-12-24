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

QString SongPlaylistItem::Title() const {
  return song_.PrettyTitle();
}

QString SongPlaylistItem::Artist() const {
  return song_.artist();
}

QString SongPlaylistItem::Album() const {
  return song_.album();
}

int SongPlaylistItem::Length() const {
  return song_.length();
}

int SongPlaylistItem::Track() const {
  return song_.track();
}

QUrl SongPlaylistItem::Url() {
  QUrl ret(QUrl::fromLocalFile(song_.filename()));
  ret.setHost("localhost");
  return ret;
}
