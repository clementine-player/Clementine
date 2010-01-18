#include "somafmservice.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QSettings>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QtDebug>

const char* SomaFMService::kServiceName = "SomaFM";
const char* SomaFMService::kLoadingChannelsText = "Getting channels";
const char* SomaFMService::kLoadingStreamText = "Loading stream";
const char* SomaFMService::kChannelListUrl = "http://somafm.com/channels.xml";

SomaFMService::SomaFMService(QObject* parent)
  : RadioService(kServiceName, parent),
    root_(NULL),
    network_(new QNetworkAccessManager(this))
{
}

RadioItem* SomaFMService::CreateRootItem(RadioItem* parent) {
  root_ = new RadioItem(this, RadioItem::Type_Service, kServiceName, parent);
  return root_;
}

void SomaFMService::LazyPopulate(RadioItem* item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      RefreshChannels();
      break;

    default:
      break;
  }

  item->lazy_loaded = true;
}

void SomaFMService::ShowContextMenu(RadioItem* item, const QPoint& global_pos) {

}

void SomaFMService::StartLoading(const QUrl& url) {
  // Load the playlist
  QNetworkRequest request = QNetworkRequest(url);
  request.setRawHeader("User-Agent", QString("%1 %2").arg(
      QCoreApplication::applicationName(), QCoreApplication::applicationVersion()).toUtf8());

  QNetworkReply* reply = network_->get(request);
  connect(reply, SIGNAL(finished()), SLOT(LoadPlaylistFinished()));

  emit TaskStarted(kLoadingStreamText);
}

void SomaFMService::LoadPlaylistFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  emit TaskFinished(kLoadingStreamText);

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: Error handling
    qDebug() << reply->errorString();
    return;
  }

  // TODO: Replace with some more robust .pls parsing :(
  QTemporaryFile temp_file;
  temp_file.open();
  temp_file.write(reply->readAll());
  temp_file.flush();

  QSettings s(temp_file.fileName(), QSettings::IniFormat);
  s.beginGroup("playlist");

  emit StreamReady(reply->url().toString(), s.value("File1").toString());
}

void SomaFMService::RefreshChannels() {
  QNetworkRequest request = QNetworkRequest(QUrl(kChannelListUrl));
  request.setRawHeader("User-Agent", QString("%1 %2").arg(
      QCoreApplication::applicationName(), QCoreApplication::applicationVersion()).toUtf8());

  QNetworkReply* reply = network_->get(request);
  connect(reply, SIGNAL(finished()), SLOT(RefreshChannelsFinished()));

  emit TaskStarted(kLoadingChannelsText);
}

void SomaFMService::RefreshChannelsFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  emit TaskFinished(kLoadingChannelsText);

  if (reply->error() != QNetworkReply::NoError) {
    // TODO: Error handling
    qDebug() << reply->errorString();
    return;
  }

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "channel") {
      ReadChannel(reader);
    }
  }
}

void SomaFMService::ReadChannel(QXmlStreamReader& reader) {
  RadioItem* item = new RadioItem(this, Type_Stream, QString::null);
  item->lazy_loaded = true;
  item->playable = true;

  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::EndElement:
        if (item->key.isNull()) {
          // Didn't find a URL
          delete item;
        } else {
          item->InsertNotify(root_);
        }
        return;

      case QXmlStreamReader::StartElement:
        if (reader.name() == "title") {
          item->display_text = reader.readElementText();
        } else if (reader.name() == "dj") {
          item->artist = reader.readElementText();
        } else if (reader.name() == "fastpls" && reader.attributes().value("format") == "mp3") {
          item->key = reader.readElementText();
        } else {
          ConsumeElement(reader);
        }
        break;

      default:
        break;
    }
  }

  delete item;
}

void SomaFMService::ConsumeElement(QXmlStreamReader& reader) {
  int level = 1;
  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::StartElement: level++; break;
      case QXmlStreamReader::EndElement:   level--; break;
      default: break;
    }

    if (level == 0)
      return;
  }
}

QString SomaFMService::TitleForItem(const RadioItem* item) const {
  return "SomaFM " + item->display_text;
}
