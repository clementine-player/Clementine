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
#include "internet/core/internetsongmimedata.h"
#include "playlist/songmimedata.h"

#include <QPainter>
#include <QUrl>
#include <QtConcurrentRun>

const int SearchProvider::kArtHeight = 32;

SearchProvider::SearchProvider(Application* app, QObject* parent)
    : QObject(parent), app_(app), hints_(0) {}

void SearchProvider::Init(const QString& name, const QString& id,
                          const QIcon& icon, Hints hints) {
  name_ = name;
  id_ = id;
  icon_ = icon;
  hints_ = hints;
  icon_as_image_ = QImage(icon.pixmap(48, 48).toImage());
}

void SearchProvider::SetHint(Hint hint, bool set) {
  if (set) {
    hints_ |= hint;
  } else {
    hints_ &= ~hint;
  }
}

QStringList SearchProvider::TokenizeQuery(const QString& query) {
  QStringList tokens(query.split(QRegExp("\\s+")));

  for (QStringList::iterator it = tokens.begin(); it != tokens.end(); ++it) {
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

bool SearchProvider::Matches(const QStringList& tokens, const QString& string) {
  for (const QString& token : tokens) {
    if (!string.contains(token, Qt::CaseInsensitive)) {
      return false;
    }
  }

  return true;
}

BlockingSearchProvider::BlockingSearchProvider(Application* app,
                                               QObject* parent)
    : SearchProvider(app, parent) {}

void BlockingSearchProvider::SearchAsync(int id, const QString& query) {
  QFuture<ResultList> future =
      QtConcurrent::run(this, &BlockingSearchProvider::Search, id, query);

  BoundFutureWatcher<ResultList, int>* watcher =
      new BoundFutureWatcher<ResultList, int>(id);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(BlockingSearchFinished()));
}

void BlockingSearchProvider::BlockingSearchFinished() {
  BoundFutureWatcher<ResultList, int>* watcher =
      static_cast<BoundFutureWatcher<ResultList, int>*>(sender());

  const int id = watcher->data();
  emit ResultsAvailable(id, watcher->result());
  emit SearchFinished(id);

  watcher->deleteLater();
}

QImage SearchProvider::ScaleAndPad(const QImage& image) {
  if (image.isNull()) return QImage();

  const QSize target_size = QSize(kArtHeight, kArtHeight);

  if (image.size() == target_size) return image;

  // Scale the image down
  QImage copy;
  copy =
      image.scaled(target_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  // Pad the image to kHeight x kHeight
  if (copy.size() == target_size) return copy;

  QImage padded_image(kArtHeight, kArtHeight, QImage::Format_ARGB32);
  padded_image.fill(0);

  QPainter p(&padded_image);
  p.drawImage((kArtHeight - copy.width()) / 2, (kArtHeight - copy.height()) / 2,
              copy);
  p.end();

  return padded_image;
}

void SearchProvider::LoadArtAsync(int id, const Result& result) {
  emit ArtLoaded(id, QImage());
}

MimeData* SearchProvider::LoadTracks(const ResultList& results) {
  MimeData* mime_data = nullptr;

  if (mime_data_contains_urls_only()) {
    mime_data = new MimeData;
  } else {
    SongList songs;
    for (const Result& result : results) {
      songs << result.metadata_;
    }

    if (internet_service()) {
      InternetSongMimeData* internet_song_mime_data =
          new InternetSongMimeData(internet_service());
      internet_song_mime_data->songs = songs;
      mime_data = internet_song_mime_data;
    } else {
      SongMimeData* song_mime_data = new SongMimeData;
      song_mime_data->songs = songs;
      mime_data = song_mime_data;
    }
  }

  QList<QUrl> urls;
  for (const Result& result : results) {
    urls << result.metadata_.url();
  }
  mime_data->setUrls(urls);

  return mime_data;
}
