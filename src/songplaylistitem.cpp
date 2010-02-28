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

  if (!song_.art_automatic().isEmpty())
    settings.setValue("art_automatic", song_.art_automatic());

  if (!song_.art_manual().isEmpty())
    settings.setValue("art_manual", song_.art_manual());
}

void SongPlaylistItem::Restore(const QSettings& settings) {
  QString filename(settings.value("filename").toString());
  int directory_id(settings.value("library_directory", -1).toInt());

  song_.InitFromFile(filename, directory_id);
  song_.set_art_automatic(settings.value("art_automatic").toString());
  song_.set_art_manual(settings.value("art_manual").toString());
}

QUrl SongPlaylistItem::Url() {
  return QUrl::fromLocalFile(song_.filename());
}

void SongPlaylistItem::Reload() {
  song_.InitFromFile(song_.filename(), song_.directory_id());
}
