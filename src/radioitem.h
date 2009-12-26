#ifndef RADIOITEM_H
#define RADIOITEM_H

#include <QIcon>

#include "simpletreeitem.h"

class RadioService;

class RadioItem : public SimpleTreeItem<RadioItem> {
 public:
  enum Type {
    Type_Root = 1,
    Type_Service,
  };

  RadioItem(RadioService* _service, int type, const QString& key = QString::null,
            RadioItem* parent = NULL);

  QIcon icon;
  RadioService* service;
};

#endif // RADIOITEM_H
