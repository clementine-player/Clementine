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

#include "songinfoprovider.h"
#include "songinfoview.h"
#include "ultimatelyricsprovider.h"
#include "ultimatelyricsreader.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

typedef QList<SongInfoProvider*> ProviderList;

SongInfoView::SongInfoView(NetworkAccessManager* network, QWidget* parent)
  : SongInfoBase(network, parent),
    ultimate_reader_(new UltimateLyricsReader(network))
{
  // Parse the ultimate lyrics xml file in the background
  QFuture<ProviderList> future = QtConcurrent::run(
      ultimate_reader_.get(), &UltimateLyricsReader::Parse,
      QString(":lyrics/ultimate_providers.xml"));
  QFutureWatcher<ProviderList>* watcher = new QFutureWatcher<ProviderList>(this);
  watcher->setFuture(future);
  connect(watcher, SIGNAL(finished()), SLOT(UltimateLyricsParsed()));
}

SongInfoView::~SongInfoView() {
}

void SongInfoView::UltimateLyricsParsed() {
  QFutureWatcher<ProviderList>* watcher =
      static_cast<QFutureWatcher<ProviderList>*>(sender());

  foreach (SongInfoProvider* provider, watcher->result()) {
    fetcher_->AddProvider(provider);
  }

  watcher->deleteLater();
  ultimate_reader_.reset();
}

void SongInfoView::ResultReady(int id, const SongInfoFetcher::Result& result) {
  if (id != current_request_id_)
    return;

  Clear();

  foreach (const CollapsibleInfoPane::Data& data, result.info_) {
    AddSection(new CollapsibleInfoPane(data, this));
  }
}
