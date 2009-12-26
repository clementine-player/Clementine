#include "radioplaylistitem.h"
#include "radioservice.h"
#include "radiomodel.h"

#include <QSettings>

RadioPlaylistItem::RadioPlaylistItem()
  : service_(NULL)
{
}

RadioPlaylistItem::RadioPlaylistItem(RadioService* service, const QUrl& url)
  : service_(service),
    url_(url)
{
}

void RadioPlaylistItem::Save(QSettings& settings) const {
  settings.setValue("service", service_->name());
  settings.setValue("url", url_.toString());
}

void RadioPlaylistItem::Restore(const QSettings& settings) {
  service_ = RadioModel::ServiceByName(settings.value("service").toString());
  url_ = settings.value("url").toString();
}

QString RadioPlaylistItem::Title() const {
  if (service_)
    return url_.toString();
  return "Radio service couldn't be loaded :-(";
}

QString RadioPlaylistItem::Artist() const {
  return QString::null;
}

QString RadioPlaylistItem::Album() const {
  return QString::null;
}

int RadioPlaylistItem::Length() const {
  return -1;
}

int RadioPlaylistItem::Track() const {
  return -1;
}

bool RadioPlaylistItem::StartLoading() {
  if (service_)
    service_->StartLoading(url_);

  return false;
}

QUrl RadioPlaylistItem::Url() {
  return url_;
}
