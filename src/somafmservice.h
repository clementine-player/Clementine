#ifndef SOMAFMSERVICE_H
#define SOMAFMSERVICE_H

#include "radioservice.h"

class QNetworkAccessManager;
class QXmlStreamReader;
class QMenu;

class SomaFMService : public RadioService {
  Q_OBJECT

 public:
  SomaFMService(QObject* parent = 0);
  ~SomaFMService();

  enum ItemType {
    Type_Stream = 2000,
  };

  static const char* kServiceName;
  static const char* kLoadingChannelsText;
  static const char* kLoadingStreamText;
  static const char* kChannelListUrl;
  static const char* kHomepage;

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem* item);

  QString TitleForItem(const RadioItem* item) const;

  void ShowContextMenu(RadioItem* item, const QPoint& global_pos);

  void StartLoading(const QUrl& url);

 private slots:
  void RefreshChannels();
  void RefreshChannelsFinished();
  void LoadPlaylistFinished();

  void AddToPlaylist();
  void Homepage();

 private:
  void ReadChannel(QXmlStreamReader& reader);
  void ConsumeElement(QXmlStreamReader& reader);

 private:
  RadioItem* root_;
  QMenu* context_menu_;
  RadioItem* context_item_;

  QNetworkAccessManager* network_;
};

#endif // SOMAFMSERVICE_H
