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

#include <QPainter>
#include <QtConcurrentRun>

const int SearchProvider::kArtHeight = 32;


SearchProvider::SearchProvider(QObject* parent)
  : QObject(parent)
{
}

void SearchProvider::Init(const QString& name, const QIcon& icon,
                          bool delay_searches, bool serialised_art) {
  name_ = name;
  icon_ = icon;
  delay_searches_ = delay_searches;
  serialised_art_ = serialised_art;
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

SearchProvider::Result::MatchQuality SearchProvider::MatchQuality(
    const QStringList& tokens, const QString& string) {
  Result::MatchQuality ret = Result::Quality_None;

  foreach (const QString& token, tokens) {
    const int index = string.indexOf(token, 0, Qt::CaseInsensitive);
    if (index == 0) {
      return Result::Quality_AtStart;
    } else if (index != -1) {
      ret = Result::Quality_Middle;
    }
  }

  return ret;
}

BlockingSearchProvider::BlockingSearchProvider(QObject* parent)
  : SearchProvider(parent) {
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
