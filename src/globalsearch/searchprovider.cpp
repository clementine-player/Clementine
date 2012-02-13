/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "searchprovider.h"
#include "core/boundfuturewatcher.h"
#include "playlist/songmimedata.h"

#include <QPainter>
#include <QtConcurrentRun>

const int SearchProvider::kArtHeight = 32;


SearchProvider::SearchProvider(Application* app, QObject* parent)
  : QObject(parent),
    app_(app),
    hints_(0)
{
}

void SearchProvider::Init(const QString& name, const QString& id,
                          const QIcon& icon, Hints hints) {
  name_ = name;
  id_ = id;
  icon_ = icon;
  hints_ = hints;
}

QStringList SearchProvider::TokenizeQuery(const QString& query) {
  QStringList tokens(query.split(QRegExp("\\s+")));

  for (QStringList::iterator it = tokens.begin() ; it != tokens.end() ; ++it) {
    (*it).remove('(');
    (*it).remove(')');
    (*it).remove('"');

    const int colon = (*it).indexOf(":");
    if (colon != -1) {
      (*it).remove(0, colon + 1);
    }
  }

  return tokens;
}

globalsearch::MatchQuality SearchProvider::MatchQuality(
    const QStringList& tokens, const QString& string) {
  globalsearch::MatchQuality ret = globalsearch::Quality_None;

  foreach (const QString& token, tokens) {
    const int index = string.indexOf(token, 0, Qt::CaseInsensitive);
    if (index == 0) {
      return globalsearch::Quality_AtStart;
    } else if (index != -1) {
      ret = globalsearch::Quality_Middle;
    }
  }

  return ret;
}

BlockingSearchProvider::BlockingSearchProvider(Application* app, QObject* parent)
  : SearchProvider(app, parent) {
}

void BlockingSearchProvider::SearchAsync(int id, const QString& query) {
  QFuture<ResultList> future = QtConcurrent::run(
      this, &BlockingSearchProvider::Search, id, query);

  BoundFutureWatcher<ResultList, int>* watcher =
      new BoundFutureWatcher<ResultList, int>(id);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(BlockingSearchFinished()));
}

void BlockingSearchProvider::BlockingSearchFinished() {
  BoundFutureWatcher<ResultList, int>* watcher =
      static_cast<BoundFutureWatcher<ResultList, int>*>(sender());
  watcher->deleteLater();

  const int id = watcher->data();
  emit ResultsAvailable(id, watcher->result());
  emit SearchFinished(id);
}

QImage SearchProvider::ScaleAndPad(const QImage& image) {
  if (image.isNull())
    return QImage();

  const QSize target_size = QSize(kArtHeight, kArtHeight);

  if (image.size() == target_size)
    return image;

  // Scale the image down
  QImage copy;
  copy = image.scaled(target_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  // Pad the image to kHeight x kHeight
  if (copy.size() == target_size)
    return copy;

  QImage padded_image(kArtHeight, kArtHeight, QImage::Format_ARGB32);
  padded_image.fill(0);

  QPainter p(&padded_image);
  p.drawImage((kArtHeight - copy.width()) / 2, (kArtHeight - copy.height()) / 2,
              copy);
  p.end();

  return padded_image;
}

namespace {
  bool SortSongsCompare(const Song& left, const Song& right) {
    if (left.disc() < right.disc())
      return true;
    if (left.disc() > right.disc())
      return false;

    return left.track() < right.track();
  }
}

void SearchProvider::SortSongs(SongList* list) {
  qStableSort(list->begin(), list->end(), SortSongsCompare);
}

void SearchProvider::LoadArtAsync(int id, const Result& result) {
  emit ArtLoaded(id, QImage());
}

void SearchProvider::LoadTracksAsync(int id, const Result& result) {
  SongMimeData* mime_data = new SongMimeData;
  mime_data->songs = SongList() << result.metadata_;

  emit TracksLoaded(id, mime_data);
}
