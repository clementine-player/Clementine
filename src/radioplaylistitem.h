#ifndef RADIOPLAYLISTITEM_H
#define RADIOPLAYLISTITEM_H

#include "playlistitem.h"
#include "song.h"

#include <QUrl>

class RadioService;

class RadioPlaylistItem : public PlaylistItem {
 public:
  RadioPlaylistItem();
  RadioPlaylistItem(RadioService* service, const QUrl& url,
                    const QString& title, const QString& artist);

  Type type() const { return Type_Radio; }
  Options options() const;

  void Save(QSettings& settings) const;
  void Restore(const QSettings& settings);

  Song Metadata() const;

  void StartLoading();
  QUrl Url();

  void LoadNext();

  void SetTemporaryMetadata(const Song& metadata);
  void ClearTemporaryMetadata();

 private:
  void InitMetadata();

 private:
  RadioService* service_;
  QUrl url_;
  QString title_;
  QString artist_;

  Song metadata_;
  Song temp_metadata_;
};

#endif // RADIOPLAYLISTITEM_H
