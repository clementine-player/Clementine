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

#include "artistinfofetcher.h"
#include "echonestartistinfo.h"

ArtistInfoFetcher::ArtistInfoFetcher(QObject* parent)
  : QObject(parent),
    next_id_(1)
{
  AddProvider(new EchoNestArtistInfo(this));
}

void ArtistInfoFetcher::AddProvider(ArtistInfoProvider* provider) {
  providers_ << provider;
  connect(provider, SIGNAL(ImageReady(int,QUrl)), SLOT(ImageReady(int,QUrl)));
  connect(provider, SIGNAL(InfoReady(int,CollapsibleInfoPane::Data)), SLOT(InfoReady(int,CollapsibleInfoPane::Data)));
  connect(provider, SIGNAL(Finished(int)), SLOT(ProviderFinished(int)));
}

int ArtistInfoFetcher::FetchInfo(const QString& artist) {
  const int id = next_id_ ++;
  results_[id] = Result();

  foreach (ArtistInfoProvider* provider, providers_) {
    waiting_for_[id].append(provider);
    provider->FetchInfo(id, artist);
  }
  return id;
}

void ArtistInfoFetcher::ImageReady(int id, const QUrl& url) {
  if (!results_.contains(id))
    return;
  results_[id].images_ << url;
}

void ArtistInfoFetcher::InfoReady(int id, const CollapsibleInfoPane::Data& data) {
  if (!results_.contains(id))
    return;
  results_[id].info_ << data;
}

void ArtistInfoFetcher::ProviderFinished(int id) {
  if (!results_.contains(id))
    return;
  if (!waiting_for_.contains(id))
    return;

  ArtistInfoProvider* provider = qobject_cast<ArtistInfoProvider*>(sender());
  if (!waiting_for_[id].contains(provider))
    return;

  waiting_for_[id].removeAll(provider);
  if (waiting_for_[id].isEmpty()) {
    emit ResultReady(id, results_.take(id));
    waiting_for_.remove(id);
  }
}
