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

  RadioItem(SimpleTreeModel<RadioItem>* model);
  RadioItem(RadioService* _service, int type, const QString& key = QString::null,
            RadioItem* parent = NULL);

  QUrl Url() const;
  QString Title() const;

  QIcon icon;
  RadioService* service;
  bool playable;
};

#endif // RADIOITEM_H
