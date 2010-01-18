#include "radioservice.h"

RadioService::RadioService(const QString& name, QObject *parent)
  : QObject(parent),
    name_(name)
{
}

QUrl RadioService::UrlForItem(const RadioItem* item) const {
  return item->key;
}

QString RadioService::TitleForItem(const RadioItem* item) const {
  return item->DisplayText();
}

QString RadioService::ArtistForItem(const RadioItem* item) const {
  return item->artist;
}

void RadioService::LoadNext(const QUrl&) {
  emit StreamFinished();
}
