#ifndef RADIOMODEL_H
#define RADIOMODEL_H

#include "radioitem.h"
#include "simpletreemodel.h"

class RadioService;

class RadioModel : public SimpleTreeModel<RadioItem> {
 public:
  RadioModel(QObject* parent = 0);

  enum {
    Role_Type = Qt::UserRole + 1,
    Role_SortText,
    Role_Key,
  };

  // QAbstractItemModel
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

 protected:
  void LazyPopulate(RadioItem* parent);

 private:
  QVariant data(const RadioItem* item, int role) const;

 private:
  QList<RadioService*> services_;
};

#endif // RADIOMODEL_H
