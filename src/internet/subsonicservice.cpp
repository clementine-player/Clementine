#include "subsonicservice.h"
#include "internetmodel.h"

const char* SubsonicService::kServiceName = "Subsonic";
const char* SubsonicService::kSettingsGroup = "Subsonic";
const char* SubsonicService::kApiVersion = "1.6.0";
const char* SubsonicService::kApiClientName = "Clementine";

SubsonicService::SubsonicService(InternetModel *parent)
  : InternetService(kServiceName, parent, parent)
{
}

SubsonicService::~SubsonicService()
{
}

QStandardItem* SubsonicService::CreateRootItem()
{
  root_ = new QStandardItem(QIcon(":providers/subsonic.png"), kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void SubsonicService::LazyPopulate(QStandardItem *item)
{

}

QModelIndex SubsonicService::GetCurrentIndex()
{
  return context_item_;
}

QUrl SubsonicService::BuildRequestUrl(const QString &view, const RequestOptions &options)
{
  QUrl url(server_url_ + "rest/" + view + ".view");
  url.addQueryItem("v", kApiVersion);
  url.addQueryItem("c", kApiClientName);
  url.addQueryItem("u", username_);
  url.addQueryItem("p", password_);
  for (RequestOptions::const_iterator i = options.begin(); i != options.end(); ++i)
  {
    url.addQueryItem(i.key(), i.value());
  }
  return url;
}
