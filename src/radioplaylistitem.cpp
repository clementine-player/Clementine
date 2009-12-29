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
  InitMetadata();
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

  InitMetadata();
}

void RadioPlaylistItem::InitMetadata() {
  if (!service_)
    metadata_.set_title("Radio service couldn't be loaded :-(");
  else if (!title_.isEmpty())
    metadata_.set_title(title_);
  else
    metadata_.set_title(url_.toString());
}

Song RadioPlaylistItem::Metadata() const {
  if (temp_metadata_.is_valid())
    return temp_metadata_;
  return metadata_;
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
  temp_metadata_ = metadata;
}

void RadioPlaylistItem::ClearTemporaryMetadata() {
  temp_metadata_ = Song();
}
