/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#include "clementinerunner.h"
#include "globalsearchinterface.h"

#include <QApplication>
#include <QDBusConnection>
#include <QMutexLocker>
#include <QtDebug>

#include <unistd.h>

const char* ClementineRunner::kDbusService   = "org.mpris.clementine";
const char* ClementineRunner::kDbusPath      = "/GlobalSearch";


ClementineRunner::ClementineRunner(QObject* parent, const QVariantList& args)
  : Plasma::AbstractRunner(parent, args),
    interface_(NULL)
{
  qDBusRegisterMetaType<GlobalSearchServiceResult>();
  qDBusRegisterMetaType<GlobalSearchServiceResultList>();

  Q_INIT_RESOURCE(clementineplasmarunner);

  nocover_ = QIcon(":/clementineplasmarunner/nocover.png");

  setIgnoredTypes(Plasma::RunnerContext::NetworkLocation |
                  Plasma::RunnerContext::FileSystem);
  setSpeed(SlowSpeed);
  setPriority(LowPriority);
  setHasRunOptions(true);

  Plasma::RunnerSyntax syntax = Plasma::RunnerSyntax("<query>", tr("Search music"));
  syntax.addExampleQuery("foo fighters");
  addSyntax(syntax);

  QDBusConnection bus = QDBusConnection::sessionBus();
  bus.connect(
      kDbusService, kDbusPath,
      OrgClementineplayerGlobalSearchInterface::staticInterfaceName(),
      "SearchFinished", "i", this, SLOT(SearchFinished(int)));
  bus.connect(
      kDbusService, kDbusPath,
      OrgClementineplayerGlobalSearchInterface::staticInterfaceName(),
      "ResultsAvailable", "ia(ibsiiissssbi)",
      this, SLOT(ResultsAvailable(int,GlobalSearchServiceResultList)));
  bus.connect(
      kDbusService, kDbusPath,
      OrgClementineplayerGlobalSearchInterface::staticInterfaceName(),
      "ArtLoaded", "iay", this, SLOT(ArtLoaded(int,QByteArray)));
}

void ClementineRunner::match(Plasma::RunnerContext& context) {

  if (context.query().length() < 3)
    return;

  if (!interface_) {
    interface_ = new OrgClementineplayerGlobalSearchInterface(
          kDbusService, kDbusPath, QDBusConnection::sessionBus());
  }

  int id = 0;
  PendingQuery query;

  // Start the search
  {
    QMutexLocker l(&query.results_mutex_);
    QMutexLocker l2(&pending_mutex_);

    QDBusPendingReply<int> reply = interface_->StartSearch(context.query(), true);
    reply.waitForFinished();

    if (!reply.isValid()) {
      qDebug() << reply.error();
      return;
    }
    id = reply.value();

    pending_queries_[id] = &query;
  }

  // Wait for results to arrive
  forever {
    query.results_semaphore_.acquire();

    // If the user closed the query then stop early.
    if (!context.isValid())
      break;

    // Take the next result on the list
    GlobalSearchServiceResult result;
    {
      QMutexLocker l(&query.results_mutex_);
      if (query.results_.isEmpty() && query.finished_)
        break;

      result = query.results_.takeFirst();
    }

    // Create a match for the result.
    Plasma::QueryMatch match(this);
    FillMatch(result, &match);

    // Emit the match
    context.addMatch(QString(), match);
  }

  // Remove the pending search
  {
    QMutexLocker l(&pending_mutex_);
    pending_queries_.remove(id);
  }
}

void ClementineRunner::FillMatch(const GlobalSearchServiceResult& result,
                                 Plasma::QueryMatch* match) const {
  QString text;
  QString subtext;

  switch (result.type_) {
  case globalsearch::Type_Track:
  case globalsearch::Type_Stream: {
    text = result.title_;

    if (!result.artist_.isEmpty()) {
      subtext += result.artist_;
    } else if (!result.album_artist_.isEmpty()) {
      subtext += result.album_artist_;
    }

    if (!result.album_.isEmpty()) {
      subtext += "  -  " + result.album_;
    }

    if (result.track_ > 0) {
      subtext += "  -  " + tr("track %1").arg(result.track_);
    }

    break;
  }

  case globalsearch::Type_Album: {
    if (!result.album_artist_.isEmpty())
      text += result.album_artist_;
    else if (result.is_compilation_)
      text += tr("Various Artists");
    else if (!result.artist_.isEmpty())
      text += result.artist_;
    else
      text += tr("Unknown");

    // Dash
    text += "  -  ";

    // Album
    if (result.album_.isEmpty())
      text += tr("Unknown");
    else
      text += result.album_;

    if (result.album_size_ > 1)
      subtext = tr("Album with %1 tracks").arg(result.album_size_);

    break;
  }
  }

  match->setType(Plasma::QueryMatch::CompletionMatch);
  match->setText(text);
  match->setSubtext(subtext);
  match->setRelevance(ResultRelevance(result));

  if (!result.image_.isNull()) {
    match->setIcon(result.image_);
  } else {
    match->setIcon(nocover_);
  }
}

void ClementineRunner::run(const Plasma::RunnerContext& context, const Plasma::QueryMatch& match) {
  // TODO
  qDebug() << __PRETTY_FUNCTION__;
}

qreal ClementineRunner::ResultRelevance(const GlobalSearchServiceResult& result) const {
  // Plasma sorts results by a qreal that we provide.  We have to do some dumb
  // maths to create a qreal for the result such that it will produce the same
  // sorting as you'd get in clementine.

  // Clementine sorts by:
  //    provider
  //    match quality
  //    type
  //    title (for tracks and streams)
  //    artist/album (for albums)

  qreal ret =
      10000 * qreal(result.provider_order_) / 10.0 +
      100   * qreal(globalsearch::Quality_None + 1 - result.match_quality_) +
      10    * qreal(globalsearch::Type_Album + 1 - result.type_);

  return ret;
}

void ClementineRunner::ResultsAvailable(int id, GlobalSearchServiceResultList results) {
  Q_ASSERT(QThread::currentThread() == qApp->thread());

  // Lock the mutex and add the results to the list.
  QMutexLocker l(&pending_mutex_);
  PendingMap::iterator it = pending_queries_.find(id);
  if (it == pending_queries_.end())
    return;

  (*it)->provider_order_ ++;

  QMutexLocker result_l(&(*it)->results_mutex_);
  foreach (GlobalSearchServiceResult result, results) {
    result.provider_order_ = (*it)->provider_order_;

    if (result.art_on_the_way_) {
      // Brace yourselves, art is coming.
      (*it)->results_waiting_for_art_ << result;
    } else {
      (*it)->results_.append(results);
      (*it)->results_semaphore_.release();
    }
  }
}

void ClementineRunner::SearchFinished(int id) {
  Q_ASSERT(QThread::currentThread() == qApp->thread());

  QMutexLocker l(&pending_mutex_);
  PendingMap::iterator it = pending_queries_.find(id);
  if (it == pending_queries_.end())
    return;

  (*it)->finished_signal_emitted_ = true;

  if ((*it)->results_waiting_for_art_.isEmpty()) {
    QMutexLocker result_l(&(*it)->results_mutex_);
    (*it)->finished_ = true;
    (*it)->results_semaphore_.release();
  }
}

void ClementineRunner::ArtLoaded(int result_id, const QByteArray& image_data) {
  Q_ASSERT(QThread::currentThread() == qApp->thread());

  QMutexLocker l(&pending_mutex_);

  // Find a query that has a result with this ID waiting for art
  foreach (PendingQuery* query, pending_queries_.values()) {
    GlobalSearchServiceResultList::iterator it;
    for (it  = query->results_waiting_for_art_.begin() ;
         it != query->results_waiting_for_art_.end() ; ++it) {
      if (it->result_id_ == result_id) {
        break;
      }
    }

    if (it == query->results_waiting_for_art_.end())
      continue;

    // The API is stupid so we have to create the QIcon here on the GUI thread.
    if (!image_data.isEmpty()) {
      QImage image;
      if (image.loadFromData(image_data)) {
        it->image_ = QIcon(QPixmap::fromImage(image));
      }
    }

    // Add the art to this result and remove it from the waiting list
    QMutexLocker result_l(&query->results_mutex_);
    query->results_ << *it;
    query->results_waiting_for_art_.erase(it);
    query->finished_ =
        query->finished_signal_emitted_ &&
        query->results_waiting_for_art_.isEmpty();
    query->results_semaphore_.release(query->finished_ ? 2 : 1);
    break;
  }
}

K_EXPORT_PLASMA_RUNNER(clementine, ClementineRunner)
