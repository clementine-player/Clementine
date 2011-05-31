#include "lastfmsuggester.h"

#include <lastfm/XmlQuery>

#include "core/logging.h"
#include "core/timeconstants.h"

using lastfm::XmlQuery;

LastFMSuggester::LastFMSuggester(QObject* parent)
  : QObject(parent),
    next_id_(0) {
}

int LastFMSuggester::SuggestSongs(const Song& song) {
  QMap<QString, QString> params;
  params["method"] = "track.getsimilar";
  params["track"] = song.title();
  params["artist"] = song.artist();
  params["limit"] = "25";
  QNetworkReply* reply = lastfm::ws::get(params);
  connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));

  int id = next_id_++;
  replies_[reply] = id;

  return id;
}

void LastFMSuggester::RequestFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);
  reply->deleteLater();

  QMap<QNetworkReply*, int>::iterator it = replies_.find(reply);
  if (it == replies_.end()) {
    return;
  }
  int id = it.value();
  replies_.erase(it);

  try {
    lastfm::XmlQuery const lfm = lastfm::ws::parse(reply);
#ifdef Q_OS_WIN32
    if (lastfm::ws::last_parse_error != lastfm::ws::NoError)
      throw std::runtime_error("");
#endif

    const QList<XmlQuery> tracks = lfm["similartracks"].children("track");
    SongList songs;
    foreach (const XmlQuery& q, tracks) {
      Song song;
      song.Init(
          q["name"].text(),
          q["artist"]["name"].text(),
          QString::null,
          QString(q["duration"].text()).toInt() * kNsecPerMsec);
      songs << song;
    }
    qLog(Debug) << songs.length() << "suggested songs from Last.fm";
    emit SuggestSongsFinished(id, songs);
  } catch (std::runtime_error& e) {
    qLog(Error) << e.what();
    emit SuggestSongsFinished(id, SongList());
  }
}
