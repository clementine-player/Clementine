#include "radiomodel.h"
#include "radioservice.h"
#include "lastfmservice.h"

RadioModel::RadioModel(QObject* parent)
  : SimpleTreeModel<RadioItem>(new RadioItem(NULL, RadioItem::Type_Root), parent)
{
  root_->lazy_loaded = true;

  LastFMService* lastfm = new LastFMService(this);
  services_ << lastfm;
  lastfm->CreateRootItem(root_);
}

QVariant RadioModel::data(const QModelIndex& index, int role) const {
  const RadioItem* item = IndexToItem(index);

  return data(item, role);
}

QVariant RadioModel::data(const RadioItem* item, int role) const {
  switch (role) {
    case Qt::DisplayRole:
      return item->DisplayText();

    case Qt::DecorationRole:
      return item->icon;
      break;

    case Role_Type:
      return item->type;

    case Role_Key:
      return item->key;

    case Role_SortText:
      return item->SortText();
  }
  return QVariant();
}

void RadioModel::LazyPopulate(RadioItem* parent) {
  if (parent->service)
    parent->service->LazyPopulate(parent);
}
