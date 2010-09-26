/* This file is part of Clementine.

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

#ifndef LYRICFETCHER_H
#define LYRICFETCHER_H

#include <QObject>

#include <boost/scoped_ptr.hpp>

#include "core/song.h"

class LyricProvider;
class NetworkAccessManager;
class UltimateLyricsReader;

class LyricFetcher : public QObject {
  Q_OBJECT

public:
  LyricFetcher(NetworkAccessManager* network, QObject* parent = 0);
  ~LyricFetcher();

  static const char* kSettingsGroup;

  QList<LyricProvider*> providers() const { return providers_; }

  int SearchAsync(const Song& metadata);

public slots:
  void ReloadSettings();

signals:
  void SearchProgress(int id, const QString& provider);
  void SearchResult(int id, bool success, const QString& title, const QString& content);

private slots:
  void UltimateLyricsParsed();

private:
  void DoSearch(const Song& metadata, int id);
  LyricProvider* ProviderByName(const QString& name) const;

private:
  NetworkAccessManager* network_;

  int next_id_;
  QList<LyricProvider*> providers_;

  boost::scoped_ptr<UltimateLyricsReader> ultimate_reader_;
};

#endif // LYRICFETCHER_H
