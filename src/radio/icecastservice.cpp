#include "icecastservice.h"

#include <algorithm>
using std::sort;
using std::unique;

#include <QFutureWatcher>
#include <QMultiHash>
#include <QNetworkReply>
#include <QRegExp>
#include <QtConcurrentRun>

#include "core/network.h"

const char* IcecastService::kServiceName = "Icecast";
const char* IcecastService::kDirectoryUrl = "http://dir.xiph.org/yp.xml";

IcecastService::IcecastService(RadioModel* parent)
    : RadioService(kServiceName, parent),
      network_(new NetworkAccessManager(this)) {
}

IcecastService::~IcecastService() {
}

RadioItem* IcecastService::CreateRootItem(RadioItem* parent) {
  root_ = new RadioItem(this, RadioItem::Type_Service, kServiceName, parent);
  root_->icon = QIcon(":last.fm/icon_radio.png");
  return root_;
}

void IcecastService::LazyPopulate(RadioItem* item) {
  switch (item->type) {
    case RadioItem::Type_Service:
      LoadDirectory();
      break;
    default:
      break;
  }
}

void IcecastService::LoadDirectory() {
  QNetworkRequest req = QNetworkRequest(QUrl(kDirectoryUrl));
  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(DownloadDirectoryFinished()));
}

void IcecastService::DownloadDirectoryFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);

  QFuture<QList<Station> > future =
      QtConcurrent::run(this, &IcecastService::ParseDirectory, reply);
  QFutureWatcher<QList<Station> >* watcher =
      new QFutureWatcher<QList<Station> >(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(ParseDirectoryFinished()));
}

namespace {
template <typename T>
struct GenreSorter {
  GenreSorter(const QMultiHash<QString, T>& genres)
      : genres_(genres) {
  }

  bool operator() (const QString& a, const QString& b) const {
    return genres_.count(a) > genres_.count(b);
  }

 private:
  const QMultiHash<QString, T>& genres_;
};

template <typename T>
struct StationSorter {
  bool operator() (T a, T b) const {
    return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
  }
};

template <typename T>
struct StationEquality {
  bool operator() (T a, T b) const {
    return a.name == b.name;
  }
};

QStringList FilterGenres(const QStringList& genres) {
  QStringList ret;
  foreach (const QString& genre, genres) {
    if (genre.length() < 2) continue;
    if (genre.contains("ÃÂ")) continue;  // Broken unicode.
    if (genre.contains(QRegExp("^#x[0-9a-f][0-9a-f]"))) continue;  // Broken XML entities.

    // Convert 80 -> 80s.
    if (genre.contains(QRegExp("^[0-9]0$"))) {
      ret << genre + 's';
    } else {
      ret << genre;
    }
  }

  if (ret.empty()) {
    ret << "other";
  }
  return ret;
}

}

void IcecastService::ParseDirectoryFinished() {
  QFutureWatcher<QList<Station> >* watcher =
      static_cast<QFutureWatcher<QList<Station> >*>(sender());
  QList<Station> all_stations = watcher->result();
  sort(all_stations.begin(), all_stations.end(), StationSorter<Station>());
  // Remove duplicates by name. These tend to be multiple URLs for the same station.
  QList<Station>::iterator it =
      unique(all_stations.begin(), all_stations.end(), StationEquality<Station>());
  all_stations.erase(it, all_stations.end());

  // Cluster stations by genre.
  QMultiHash<QString, const Station*> genres;
  // Add stations in reverse.
  QListIterator<Station> reverse_it(all_stations);
  reverse_it.toBack();
  while (reverse_it.hasPrevious()) {
    const Station& s = reverse_it.previous();
    QStringList filtered_genres = FilterGenres(s.genres);
    foreach (const QString& genre, filtered_genres) {
      genres.insert(genre, &s);
    }
  }

  // Sort genres by station count.
  // HACK: De-dupe keys.
  QList<QString> genre_names = genres.keys().toSet().toList();
  sort(genre_names.begin(), genre_names.end(), GenreSorter<const Station*>(genres));

  foreach (const QString& genre, genre_names) {
    QString genre_name(genre);
    genre_name[0] = genre_name[0].toUpper();
    RadioItem* genre_item = new RadioItem(this, Type_Genre, genre_name);
    genre_item->icon = QIcon(":last.fm/icon_tag.png");

    QList<const Station*> stations = genres.values(genre);
    foreach (const Station* station, stations) {
      RadioItem* radio = new RadioItem(
          this, Type_Stream, station->url.toString(), genre_item);
      radio->lazy_loaded = true;
      radio->playable = true;
      radio->icon = QIcon(":last.fm/icon_radio.png");
      radio->display_text = station->name;
    }
    genre_item->InsertNotify(root_);
  }

  root_->lazy_loaded = true;
  delete watcher;
}

QList<IcecastService::Station> IcecastService::ParseDirectory(QIODevice* device) const {
  QXmlStreamReader reader(device);
  QList<Station> stations;
  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "entry") {
      stations << ReadStation(&reader);
    }
  }
  device->deleteLater();
  return stations;
}

IcecastService::Station IcecastService::ReadStation(QXmlStreamReader* reader) const {
  Station station;
  while (!reader->atEnd()) {
    reader->readNext();
    if (reader->tokenType() == QXmlStreamReader::EndElement)
      break;

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      QStringRef name = reader->name();
      QString value = reader->readElementText(QXmlStreamReader::SkipChildElements);

      if (name == "server_name") station.name = value;
      if (name == "listen_url")  station.url = QUrl(value);
      if (name == "server_type") station.mime_type = value;
      if (name == "bitrate")     station.bitrate = value.toInt();
      if (name == "channels")    station.channels = value.toInt();
      if (name == "samplerate")  station.samplerate = value.toInt();
      if (name == "genre")       station.genres = value.split(' ', QString::SkipEmptyParts);
    }
  }

  // HACK: This hints to the player that the artist and title metadata needs swapping.
  station.url.setFragment("icecast");

  return station;
}
