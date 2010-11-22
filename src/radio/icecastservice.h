#ifndef ICECASTSERVICE_H
#define ICECASTSERVICE_H

#include "radioservice.h"

#include <QXmlStreamReader>

class NetworkAccessManager;

class IcecastService : public RadioService {
  Q_OBJECT
 public:
  IcecastService(RadioModel* parent);
  ~IcecastService();

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem* item);

  static const char* kServiceName;
  static const char* kDirectoryUrl;

  enum ItemType {
    Type_Stream = 3000,
    Type_Genre,
  };

 private:
  struct Station {
    Station()
      : bitrate(0),
        channels(0),
        samplerate(0) {
    }
    QString name;
    QUrl url;
    QString mime_type;
    int bitrate;
    int channels;
    int samplerate;
    QStringList genres;
  };

  void LoadDirectory();
  QList<Station> ParseDirectory(QIODevice* device) const;
  Station ReadStation(QXmlStreamReader* reader) const;

  RadioItem* root_;
  NetworkAccessManager* network_;

 private slots:
  void DownloadDirectoryFinished();
  void ParseDirectoryFinished();
};

#endif
