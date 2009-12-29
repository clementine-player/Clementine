#include "radioplaylistitem.h"
#include "radioservice.h"
#include "radiomodel.h"

#include <QSettings>

RadioPlaylistItem::RadioPlaylistItem()
  : service_(NULL)
{
}

RadioPlaylistItem::RadioPlaylistItem(RadioService* service, const QUrl& url,
                                     const QString& title)
  : service_(service),
    url_(url),
    title_(title)
{
}

void RadioPlaylistItem::Save(QSettings& settings) const {
  settings.setValue("service", service_->name());
  settings.setValue("url", url_.toString());
  settings.setValue("title", title_);
}

void RadioPlaylistItem::Restore(const QSettings& settings) {
  service_ = RadioModel::ServiceByName(settings.value("service").toString());
  url_ = settings.value("url").toString();
  title_ = settings.value("title").toString();
}

QString RadioPlaylistItem::Title() const {
  if (!service_)
    return "Radio service couldn't be loaded :-(";

  if (metadata_.is_valid())
    return metadata_.title();

  if (!title_.isEmpty())
    return title_;

  return url_.toString();
}

QString RadioPlaylistItem::Artist() const {
  return metadata_.is_valid() ? metadata_.artist() : QString::null;
}

QString RadioPlaylistItem::Album() const {
  return metadata_.is_valid() ? metadata_.album() : QString::null;
}

int RadioPlaylistItem::Length() const {
  return metadata_.is_valid() ? metadata_.length() : -1;
}

int RadioPlaylistItem::Track() const {
  return metadata_.is_valid() ? metadata_.track() : -1;
}

void RadioPlaylistItem::StartLoading() {
  if (service_)
    service_->StartLoading(url_);
}

void RadioPlaylistItem::LoadNext() {
  if (service_)
    service_->LoadNext(url_);
}

QUrl RadioPlaylistItem::Url() {
  return url_;
}

PlaylistItem::Options RadioPlaylistItem::options() const {
  PlaylistItem::Options ret = SpecialPlayBehaviour;

  if (service_) {
    ret |= ContainsMultipleTracks;

    if (!service_->IsPauseAllowed())
      ret |= PauseDisabled;
    if (service_->ShowLastFmControls())
      ret |= LastFMControls;
  }

  return ret;
}

void RadioPlaylistItem::SetTemporaryMetadata(const Song& metadata) {
  metadata_ = metadata;
}

void RadioPlaylistItem::ClearTemporaryMetadata() {
  metadata_ = Song();
}
