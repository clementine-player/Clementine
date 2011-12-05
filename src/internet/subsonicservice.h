#ifndef SUBSONICSERVICE_H
#define SUBSONICSERVICE_H

#include "internetservice.h"

class SubsonicService : public InternetService
{
  Q_OBJECT
 public:
  SubsonicService(InternetModel *parent);
  ~SubsonicService();

  typedef QMap<QString, QString> RequestOptions;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem *item);

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kApiVersion;
  static const char* kApiClientName;

 protected:
  QModelIndex GetCurrentIndex();

 private:
  QUrl BuildRequestUrl(const QString &view, const RequestOptions &options);

  QModelIndex context_item_;
  QStandardItem* root_;

  // Configuration
  QString server_url_;
  QString username_;
  QString password_;

};

#endif // SUBSONICSERVICE_H
