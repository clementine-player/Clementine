#ifndef RADIOITEM_H
#define RADIOITEM_H

#include <QIcon>
#include <QUrl>

#include "simpletreeitem.h"

class RadioService;

class RadioItem : public SimpleTreeItem<RadioItem> {
 public:
  enum Type {
    Type_Root = 1,
    Type_Service,
  };

  struct PlaylistData {
    PlaylistData(const QString& _title, const QUrl& _url) : title(_title), url(_url) {}

    QString title;
    QUrl url;
  };

  RadioItem(RadioService* _service, int type, const QString& key = QString::null,
            RadioItem* parent = NULL);

  QIcon icon;
  RadioService* service;
  bool playable;
};

#endif // RADIOITEM_H
