#include "radiomodel.h"
#include "radioservice.h"
#include "lastfmservice.h"
#include "radiomimedata.h"

#include <QMimeData>
#include <QtDebug>

QMap<QString, RadioService*> RadioModel::sServices;

RadioModel::RadioModel(QObject* parent)
  : SimpleTreeModel<RadioItem>(new RadioItem(NULL, RadioItem::Type_Root), parent)
{
  Q_ASSERT(sServices.isEmpty());

  root_->lazy_loaded = true;

  AddService(new LastFMService(this));
}

void RadioModel::AddService(RadioService *service) {
  sServices[service->name()] = service;
  service->CreateRootItem(root_);

  connect(service, SIGNAL(LoadingStarted()), SIGNAL(LoadingStarted()));
  connect(service, SIGNAL(LoadingFinished()), SIGNAL(LoadingFinished()));
  connect(service, SIGNAL(StreamReady(QUrl,QUrl)), SIGNAL(StreamReady(QUrl,QUrl)));
  connect(service, SIGNAL(StreamFinished()), SIGNAL(StreamFinished()));
  connect(service, SIGNAL(StreamError(QString)), SIGNAL(StreamError(QString)));
  connect(service, SIGNAL(StreamMetadataFound(QUrl,Song)), SIGNAL(StreamMetadataFound(QUrl,Song)));
}

RadioService* RadioModel::ServiceByName(const QString& name) {
  if (sServices.contains(name))
    return sServices[name];
  return NULL;
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

Qt::ItemFlags RadioModel::flags(const QModelIndex& index) const {
  RadioItem* item = IndexToItem(index);
  if (item->playable)
    return Qt::ItemIsSelectable |
           Qt::ItemIsEnabled |
           Qt::ItemIsDragEnabled;

  return Qt::ItemIsSelectable |
         Qt::ItemIsEnabled;
}

QStringList RadioModel::mimeTypes() const {
  return QStringList() << "text/uri-list";
}

QMimeData* RadioModel::mimeData(const QModelIndexList& indexes) const {
  QList<QUrl> urls;
  QList<RadioItem*> items;

  foreach (const QModelIndex& index, indexes) {
    RadioItem* item = IndexToItem(index);
    if (!item || !item->service || !item->playable)
      continue;

    items << item;
    urls << item->service->UrlForItem(item);
  }

  if (urls.isEmpty())
    return NULL;

  RadioMimeData* data = new RadioMimeData;
  data->setUrls(urls);
  data->items = items;

  return data;
}

LastFMService* RadioModel::GetLastFMService() const {
  if (sServices.contains(LastFMService::kServiceName))
    return static_cast<LastFMService*>(sServices[LastFMService::kServiceName]);
  return NULL;
}

void RadioModel::ShowContextMenu(RadioItem* item, const QPoint& global_pos) {
  if (item->service)
    item->service->ShowContextMenu(item, global_pos);
}
