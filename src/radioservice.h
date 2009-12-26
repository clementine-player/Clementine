#ifndef RADIOSERVICE_H
#define RADIOSERVICE_H

#include <QObject>
#include <QList>
#include <QUrl>

class RadioItem;

class RadioService : public QObject {
  Q_OBJECT

 public:
  RadioService(const QString& name, QObject* parent = 0);
  virtual ~RadioService() {}

  QString name() const { return name_; }

  virtual RadioItem* CreateRootItem(RadioItem* parent) = 0;
  virtual void LazyPopulate(RadioItem* item) = 0;

  virtual QList<QUrl> UrlsForItem(RadioItem* item) = 0;
  virtual void StartLoading(const QUrl& url) = 0;

 signals:
  void LoadingStarted();
  void LoadingFinished();
  void StreamReady(const QUrl& original_url, const QUrl& media_url);
  void StreamFinished();
  void StreamError(const QString& message);

 private:
  QString name_;
};

#endif // RADIOSERVICE_H
