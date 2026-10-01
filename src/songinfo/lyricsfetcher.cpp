/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "lyricsfetcher.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QStringDecoder>
#include <QTimer>
#include <QUrlQuery>

#include "core/logging.h"
#include "core/network.h"
#include "core/timeconstants.h"
#include "songinfo/lrcparser.h"

const char* LyricsFetcher::kSettingsGroup = "SongInfo";
const char* LyricsFetcher::kOnlineLyricsKey = "online_lyrics";

std::atomic<qint64> LyricsFetcher::sLrclibRetryAfter(0);

namespace {

const char* kLrclibUrl = "https://lrclib.net/api/";
// LRCLIB asks clients to say who they are: our User-Agent already starts
// "Clementine <version>".
const char* kLrclibUserAgent = "(https://www.clementine-player.org)";
// How far a search result's length can be from the song's.
const int kDurationToleranceSecs = 10;
// Lyrics files bigger than this aren't lyrics.
const qint64 kMaxLyricsFileSize = 1024 * 1024;
// How long to leave LRCLIB alone after a 429 that doesn't say.
const int kDefaultRetryAfterSecs = 60;

// Letters and digits only, lower case, so "AC/DC" matches "ACDC".
QString Normalise(const QString& text) {
  QString ret;
  for (const QChar& c : text.toCaseFolded()) {
    if (c.isLetterOrNumber()) ret += c;
  }
  return ret;
}

QString ReadLyricsFile(const QString& filename) {
  QFile file(filename);
  if (file.size() > kMaxLyricsFileSize || !file.open(QIODevice::ReadOnly)) {
    return QString();
  }
  const QByteArray data = file.readAll();
  QStringDecoder utf8(QStringConverter::Utf8);
  QString text = utf8.decode(data);
  // Not UTF-8: older lyrics files are often in the system's encoding.
  if (utf8.hasError()) text = QString::fromLocal8Bit(data);
  return text;
}

}  // namespace

LyricsFetcher::LyricsFetcher(QObject* parent)
    : QObject(parent), network_(new NetworkAccessManager(this)), next_id_(1) {
  qRegisterMetaType<Lyrics>("Lyrics");
}

int LyricsFetcher::Fetch(const Song& song) {
  const int id = next_id_++;

  Lyrics local = FromLocal(song);
  QSettings s;
  s.beginGroup(kSettingsGroup);
  const bool online = s.value(kOnlineLyricsKey, true).toBool();

  if (!local.IsEmpty() || !online || song.artist().isEmpty() ||
      song.title().isEmpty() || LrclibWaiting()) {
    // Always after Fetch() returns, so callers can store the id first.
    QTimer::singleShot(0, this, [this, id, local] { Finish(id, local); });
  } else {
    LrclibGet(id, song);
  }
  return id;
}

Lyrics LyricsFetcher::FromText(const QString& text, const QString& title) {
  Lyrics ret;
  ret.title = title;
  if (LrcParser::IsLrc(text)) {
    ret.lrc = text;
    ret.plain = LrcParser::ToPlainText(text);
  } else {
    ret.plain = text.trimmed();
  }
  return ret;
}

Lyrics LyricsFetcher::FromLocal(const Song& song) {
  if (!song.lyrics().trimmed().isEmpty()) {
    return FromText(song.lyrics(), tr("Lyrics from the file's tags"));
  }

  // A file per track: not for tracks of a CUE sheet, which share one.
  if (!song.url().isLocalFile() || song.has_cue()) return Lyrics();

  const QFileInfo info(song.url().toLocalFile());
  const QString base = info.path() + "/" + info.completeBaseName();
  for (const char* extension : {"lrc", "LRC", "txt", "TXT"}) {
    const QString filename = base + "." + QLatin1String(extension);
    if (!QFile::exists(filename)) continue;

    Lyrics ret =
        FromText(ReadLyricsFile(filename),
                 tr("Lyrics from %1").arg(QFileInfo(filename).fileName()));
    if (!ret.IsEmpty()) return ret;
  }
  return Lyrics();
}

bool LyricsFetcher::LrclibWaiting() {
  return QDateTime::currentMSecsSinceEpoch() < sLrclibRetryAfter;
}

bool LyricsFetcher::CheckRateLimit(QNetworkReply* reply) {
  if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() !=
      429) {
    return false;
  }
  // LRCLIB says how many seconds to wait, and bans clients that don't.
  bool ok = false;
  int seconds = reply->rawHeader("Retry-After").trimmed().toInt(&ok);
  if (!ok || seconds <= 0) seconds = kDefaultRetryAfterSecs;
  sLrclibRetryAfter = QDateTime::currentMSecsSinceEpoch() + seconds * 1000ll;
  qLog(Warning) << "LRCLIB asked us to wait" << seconds << "seconds";
  return true;
}

void LyricsFetcher::LrclibGet(int id, const Song& song) {
  // An exact match on the song's details, including its length.
  QUrlQuery query;
  query.addQueryItem("artist_name", song.artist());
  query.addQueryItem("track_name", song.title());
  if (!song.album().isEmpty()) query.addQueryItem("album_name", song.album());
  if (song.length_nanosec() > 0) {
    query.addQueryItem(
        "duration",
        QString::number(qRound64(double(song.length_nanosec()) / kNsecPerSec)));
  }

  QUrl url(QString(kLrclibUrl) + "get");
  url.setQuery(query);
  QNetworkRequest request(url);
  request.setRawHeader("User-Agent", kLrclibUserAgent);
  QNetworkReply* reply = network_->get(request);
  connect(reply, &QNetworkReply::finished, this,
          [=] { LrclibGetFinished(reply, id, song); });
}

void LyricsFetcher::LrclibGetFinished(QNetworkReply* reply, int id,
                                      const Song& song) {
  reply->deleteLater();
  if (CheckRateLimit(reply)) {
    Finish(id, Lyrics());
    return;
  }

  const int status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (status == 200) {
    Lyrics lyrics =
        FromLrclib(QJsonDocument::fromJson(reply->readAll()).object());
    if (!lyrics.IsEmpty()) {
      Finish(id, lyrics);
      return;
    }
  } else if (status != 404) {
    qLog(Warning) << "LRCLIB lookup failed:" << status << reply->errorString();
    Finish(id, Lyrics());
    return;
  }

  // Not under exactly these details: search, which doesn't need the album or
  // length to match.
  QUrlQuery query;
  query.addQueryItem("artist_name", song.artist());
  query.addQueryItem("track_name", song.title());
  QUrl url(QString(kLrclibUrl) + "search");
  url.setQuery(query);
  QNetworkRequest request(url);
  request.setRawHeader("User-Agent", kLrclibUserAgent);
  QNetworkReply* search = network_->get(request);
  connect(search, &QNetworkReply::finished, this,
          [=] { LrclibSearchFinished(search, id, song); });
}

void LyricsFetcher::LrclibSearchFinished(QNetworkReply* reply, int id,
                                         const Song& song) {
  reply->deleteLater();
  if (CheckRateLimit(reply) || reply->error() != QNetworkReply::NoError) {
    if (reply->error() != QNetworkReply::NoError) {
      qLog(Warning) << "LRCLIB search failed:" << reply->errorString();
    }
    Finish(id, Lyrics());
    return;
  }

  const QJsonArray results = QJsonDocument::fromJson(reply->readAll()).array();
  Finish(id, FromLrclib(ChooseSearchResult(results, song)));
}

QJsonObject LyricsFetcher::ChooseSearchResult(const QJsonArray& results,
                                              const Song& song) {
  const QString artist = Normalise(song.artist());
  const QString title = Normalise(song.title());
  const qint64 length = qRound64(double(song.length_nanosec()) / kNsecPerSec);

  // The same artist and title, with words, of the closest length.
  QJsonObject best;
  qint64 best_difference = 0;
  for (const QJsonValue& value : results) {
    const QJsonObject result = value.toObject();
    if (Normalise(result["artistName"].toString()) != artist ||
        Normalise(result["trackName"].toString()) != title) {
      continue;
    }
    if (FromLrclib(result).IsEmpty()) continue;

    qint64 difference = 0;
    if (length > 0) {
      difference = qAbs(qRound64(result["duration"].toDouble()) - length);
      if (difference > kDurationToleranceSecs) continue;
    }
    if (best.isEmpty() || difference < best_difference) {
      best = result;
      best_difference = difference;
    }
  }
  return best;
}

Lyrics LyricsFetcher::FromLrclib(const QJsonObject& result) {
  Lyrics ret;
  ret.title = tr("Lyrics from LRCLIB");
  if (result["instrumental"].toBool()) {
    ret.instrumental = true;
    return ret;
  }
  ret.lrc = result["syncedLyrics"].toString();
  ret.plain = result["plainLyrics"].toString().trimmed();
  if (ret.plain.isEmpty() && !ret.lrc.isEmpty()) {
    ret.plain = LrcParser::ToPlainText(ret.lrc);
  }
  return ret;
}

void LyricsFetcher::Finish(int id, const Lyrics& lyrics) {
  emit Finished(id, lyrics);
}
