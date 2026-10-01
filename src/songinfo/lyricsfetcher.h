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

#ifndef SONGINFO_LYRICSFETCHER_H_
#define SONGINFO_LYRICSFETCHER_H_

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <atomic>

#include "core/song.h"

class NetworkAccessManager;
class QNetworkReply;

struct Lyrics {
  // The words to show, without any times.
  QString plain;
  // The words with the times they're sung, in LRC format, if known.
  QString lrc;
  // The song has no words.
  bool instrumental = false;
  // Where they came from, to title them with, eg. "Lyrics from LRCLIB".
  QString title;

  bool IsEmpty() const { return plain.isEmpty() && !instrumental; }
};
Q_DECLARE_METATYPE(Lyrics)

// Finds a song's lyrics: in its tags, then in a .lrc or .txt file of the same
// name next to it, then on LRCLIB (https://lrclib.net), unless online lyrics
// are turned off.
class LyricsFetcher : public QObject {
  Q_OBJECT

 public:
  explicit LyricsFetcher(QObject* parent = nullptr);

  static const char* kSettingsGroup;
  // Whether to look on LRCLIB.
  static const char* kOnlineLyricsKey;

  // Starts looking. Finished() is emitted with the returned id, always
  // after this returns.
  int Fetch(const Song& song);

  // The song's lyrics from its tags, or a file next to it.
  static Lyrics FromLocal(const Song& song);
  // The LRCLIB search result that's this song, if any.
  static QJsonObject ChooseSearchResult(const QJsonArray& results,
                                        const Song& song);

 signals:
  void Finished(int id, const Lyrics& lyrics);

 private:
  static Lyrics FromText(const QString& text, const QString& title);
  static Lyrics FromLrclib(const QJsonObject& result);
  // Whether LRCLIB has asked us to wait, after a 429.
  static bool LrclibWaiting();
  bool CheckRateLimit(QNetworkReply* reply);

  void LrclibGet(int id, const Song& song);
  void LrclibGetFinished(QNetworkReply* reply, int id, const Song& song);
  void LrclibSearchFinished(QNetworkReply* reply, int id, const Song& song);
  void Finish(int id, const Lyrics& lyrics);

  NetworkAccessManager* network_;
  int next_id_;

  // When LRCLIB said to try again (msec since the epoch), shared by the
  // fetchers in each thread.
  static std::atomic<qint64> sLrclibRetryAfter;
};

#endif  // SONGINFO_LYRICSFETCHER_H_
