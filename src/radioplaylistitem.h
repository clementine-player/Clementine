#ifndef RADIOPLAYLISTITEM_H
#define RADIOPLAYLISTITEM_H

#include "playlistitem.h"
#include "song.h"

#include <QUrl>

class RadioService;

class RadioPlaylistItem : public PlaylistItem {
 public:
  RadioPlaylistItem();
  RadioPlaylistItem(RadioService* service, const QUrl& url, const QString& title);

  Type type() const { return Type_Radio; }

  void Save(QSettings& settings) const;
  void Restore(const QSettings& settings);

  QString Title() const;
  QString Artist() const;
  QString Album() const;
  int Length() const;
  int Track() const;

  bool StartLoading();
  QUrl Url();

  bool LoadNext();

  void SetTemporaryMetadata(const Song& metadata);
  void ClearTemporaryMetadata();

 private:
  RadioService* service_;
  QUrl url_;
  QString title_;

  Song metadata_;
};

#endif // RADIOPLAYLISTITEM_H
