#ifndef RADIOSERVICE_H
#define RADIOSERVICE_H

#include <QObject>
#include <QList>
#include <QUrl>

#include "radioitem.h"
#include "multiloadingindicator.h"

class Song;

class RadioService : public QObject {
  Q_OBJECT

 public:
  RadioService(const QString& name, QObject* parent = 0);
  virtual ~RadioService() {}

  QString name() const { return name_; }

  virtual RadioItem* CreateRootItem(RadioItem* parent) = 0;
  virtual void LazyPopulate(RadioItem* item) = 0;

  virtual QUrl UrlForItem(const RadioItem* item) const;
  virtual QString TitleForItem(const RadioItem* item) const;
  virtual QString ArtistForItem(const RadioItem* item) const;

  virtual void ShowContextMenu(RadioItem* item, const QPoint& global_pos) {
    Q_UNUSED(item); Q_UNUSED(global_pos); }

  virtual void StartLoading(const QUrl& url) = 0;
  virtual void LoadNext(const QUrl& url);

  virtual bool IsPauseAllowed() const { return true; }
  virtual bool ShowLastFmControls() const { return false; }

  virtual void ReloadSettings() {}

 signals:
  void TaskStarted(MultiLoadingIndicator::TaskType);
  void TaskFinished(MultiLoadingIndicator::TaskType);

  void StreamReady(const QUrl& original_url, const QUrl& media_url);
  void StreamFinished();
  void StreamError(const QString& message);
  void StreamMetadataFound(const QUrl& original_url, const Song& song);

  void AddItemToPlaylist(RadioItem* item);

 private:
  QString name_;
};

#endif // RADIOSERVICE_H
