#ifndef SOMAFMSERVICE_H
#define SOMAFMSERVICE_H

#include "radioservice.h"

class QNetworkAccessManager;
class QXmlStreamReader;

class SomaFMService : public RadioService {
  Q_OBJECT

 public:
  SomaFMService(QObject* parent = 0);

  enum ItemType {
    Type_Stream = 2000,
  };

  static const char* kServiceName;
  static const char* kLoadingChannelsText;
  static const char* kLoadingStreamText;
  static const char* kChannelListUrl;

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem* item);

  QString TitleForItem(const RadioItem* item) const;

  void ShowContextMenu(RadioItem* item, const QPoint& global_pos);

  void StartLoading(const QUrl& url);

 private slots:
  void RefreshChannelsFinished();
  void LoadPlaylistFinished();

 private:
  void RefreshChannels();
  void ReadChannel(QXmlStreamReader& reader);
  void ConsumeElement(QXmlStreamReader& reader);

 private:
  RadioItem* root_;

  QNetworkAccessManager* network_;
};

#endif // SOMAFMSERVICE_H
