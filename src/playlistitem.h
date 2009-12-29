#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QStandardItem>
#include <QUrl>

class QSettings;

class Song;

class PlaylistItem {
 public:
  PlaylistItem() {}
  virtual ~PlaylistItem() {}

  static PlaylistItem* NewFromType(const QString& type);

  enum Type {
    Type_Song,
    Type_Radio,
  };

  enum Option {
    Default = 0x00,

    SpecialPlayBehaviour = 0x01,
    ContainsMultipleTracks = 0x02,
    PauseDisabled = 0x04,
    LastFMControls = 0x08,
  };
  Q_DECLARE_FLAGS(Options, Option);

  virtual Type type() const = 0;
  QString type_string() const;

  virtual Options options() const { return Default; }

  virtual void Save(QSettings& settings) const = 0;
  virtual void Restore(const QSettings& settings) = 0;

  virtual Song Metadata() const = 0;

  // If the item needs to do anything special before it can play (eg. start
  // streaming the radio stream), then it should implement StartLoading() and
  // return true.  If it returns false then the URL from Url() will be passed
  // directly to xine instead.
  virtual void StartLoading() {}
  virtual QUrl Url() = 0;

  // If the item is a radio station that can play another song after one has
  // finished then it should do so and return true
  virtual void LoadNext() {}

  virtual void SetTemporaryMetadata(const Song& metadata) {Q_UNUSED(metadata)}
  virtual void ClearTemporaryMetadata() {}
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlaylistItem::Options);

#endif // PLAYLISTITEM_H
