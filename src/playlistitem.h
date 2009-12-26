#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QStandardItem>
#include <QUrl>

class QSettings;

class PlaylistItem {
 public:
  PlaylistItem() {}
  virtual ~PlaylistItem() {}

  static PlaylistItem* NewFromType(const QString& type);

  enum Type {
    Type_Song,
    Type_Radio,
  };

  virtual Type type() const = 0;
  QString type_string() const;

  virtual void Save(QSettings& settings) const = 0;
  virtual void Restore(const QSettings& settings) = 0;

  virtual QString Title() const = 0;
  virtual QString Artist() const = 0;
  virtual QString Album() const = 0;
  virtual int Length() const = 0;
  virtual int Track() const = 0;

  // If the item needs to do anything special before it can play (eg. start
  // streaming the radio stream), then it should implement StartLoading() and
  // return true.  If it returns false then the URL from Url() will be passed
  // directly to xine instead.
  virtual bool StartLoading() { return false; }
  virtual QUrl Url() = 0;
};

#endif // PLAYLISTITEM_H
