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

#include "songinfofetcher.h"
#include "songinfoprovider.h"

SongInfoFetcher::SongInfoFetcher(QObject* parent)
  : QObject(parent),
    next_id_(1)
{
}

void SongInfoFetcher::AddProvider(SongInfoProvider* provider) {
  providers_ << provider;
  connect(provider, SIGNAL(ImageReady(int,QUrl)), SLOT(ImageReady(int,QUrl)));
  connect(provider, SIGNAL(InfoReady(int,CollapsibleInfoPane::Data)), SLOT(InfoReady(int,CollapsibleInfoPane::Data)));
  connect(provider, SIGNAL(Finished(int)), SLOT(ProviderFinished(int)));
}

int SongInfoFetcher::FetchInfo(const Song& metadata) {
  const int id = next_id_ ++;
  results_[id] = Result();

  foreach (SongInfoProvider* provider, providers_) {
    waiting_for_[id].append(provider);
    provider->FetchInfo(id, metadata);
  }
  return id;
}

void SongInfoFetcher::ImageReady(int id, const QUrl& url) {
  if (!results_.contains(id))
    return;
  results_[id].images_ << url;
}

void SongInfoFetcher::InfoReady(int id, const CollapsibleInfoPane::Data& data) {
  if (!results_.contains(id))
    return;
  results_[id].info_ << data;
}

void SongInfoFetcher::ProviderFinished(int id) {
  if (!results_.contains(id))
    return;
  if (!waiting_for_.contains(id))
    return;

  SongInfoProvider* provider = qobject_cast<SongInfoProvider*>(sender());
  if (!waiting_for_[id].contains(provider))
    return;

  waiting_for_[id].removeAll(provider);
  if (waiting_for_[id].isEmpty()) {
    emit ResultReady(id, results_.take(id));
    waiting_for_.remove(id);
  }
}
